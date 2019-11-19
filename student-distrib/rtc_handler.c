#include "rtc_handler.h"
#include "i8259.h"
#include "lib.h"
#include "pit.h"
#include "terminal.h"

volatile int rtc_interrupt_occurred[NUM_TERM] = {0, 0, 0};

/*
 * rtc_interrupt_handler
 *   DESCRIPTION: an interrupt handler specialized with dealing rtc interrupts.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Handle rtc interrupt, send end-of-interrupt
 *				   to unmask handled interrupt
 *
 */
void rtc_interrupt_handler(){
    int i;
	cli();
	outb(RTC_REG_C, RTC_PORT); 	//select register C
	inb(CMOS_PORT);				//just throw away contents
    for (i = 0; i < NUM_TERM; i++) {
        rtc_interrupt_occurred[i] = 1;        //change interrupt status
    }
	//test_interrupts();
	//send eoi the RTC line, mask it
	send_eoi(RTC_IRQ_NUM);
	sti();
}

/*
 * rtc_init
 *   DESCRIPTION: initialize rtc device to avoid getting an undefined state.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enables rtc_interrupt_handler to use rtc
 *                 sets the frquency of rtc to 2Hz
 */
void rtc_init(){
	char prevA, prevB;
	//disable interrupt
	disable_irq(RTC_IRQ_NUM);

	/* select register A, which is responsible for handling freq */
 	outb(RTC_REG_A|NMI_BIT, RTC_PORT);
 	prevA = inb(CMOS_PORT);	//read current value of reg A
 	outb(RTC_REG_A, RTC_PORT);
 	//select freq rate: ranging from 2Hz(bit3-0 1111) to 1024 Hz (0110)
 	outb(MAX_FREQ|prevA, CMOS_PORT);	//the bit map of freq should be larger than MAX_FREQ

 	/* select register B, which is for enabling interrupt */
	outb(RTC_REG_B|NMI_BIT, RTC_PORT);
	/* read the current value of Register B */
 	prevB = inb(CMOS_PORT);
	/* Set the index again ( a read will reset the index to register B) */
 	outb(RTC_REG_B|NMI_BIT, RTC_PORT);
	/* Write the previous value ORed with 0x40. Turns on bit 6 of register B */
    outb(prevB|RTC_PIE, CMOS_PORT); //turn on bit six of reg B (0x40)
	rtc_set_freq(MAX_RTC_FREQ);
     //enable interrupt
 	sti();
	/* enable appropriate IRQ port on PIC (Line #8) */
 	enable_irq(RTC_IRQ_NUM);
}

/*
 *   rtc_open
 *   DESCRIPTION: rtc open set the frequency to 2hz, when default opened
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int rtc_open(const uint8_t* filename){
	term[running_term].rtc_freq = MIN_RTC_FREQ;			 // default is 2Hz
	return 0;
}

/*
 *   rtc_read
 *   DESCRIPTION: RTC read waits until the next interrupt occurs. If it does occur, it returns 0
 *   else it wait until RTC interrupt occurs
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: return 0 if next interrupt occurred
 *   SIDE EFFECTS: none
 *
 */
int rtc_read(int32_t fd, void* buf, int32_t nbytes){
	int count, i, num_running;
	num_running = 1;
	count = MAX_RTC_FREQ / term[running_term].rtc_freq;
	if (term[1].running_pid != -1)
		num_running++;
	if (term[2].running_pid != -1)
		num_running++;

	count /= num_running;
	for (i = 0; i < count; i++) {
		while(!rtc_interrupt_occurred[running_term]) {} //wait unter interrupt comes
		//interrupt occurs
		rtc_interrupt_occurred[running_term] = 0; //interrupt ends
	}
	return 0;
}

/*
 * rtc_write
 *   DESCRIPTION: change the rate of RTC.
 *   INPUTS: buf - new frequency, nbytes - always 4
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fail. otherwise 4.
 *   SIDE EFFECTS: none.
 */
int rtc_write(int32_t fd, const void* buf, int32_t nbytes){

 	int32_t interrupt_freq = *((int32_t*)buf);
	// Check the valid input.
    if (interrupt_freq <MIN_RTC_FREQ || interrupt_freq > MAX_RTC_FREQ  ) //must be power of two
        return -1;

	term[running_term].rtc_freq = interrupt_freq;
	return 4;
    //return rtc_set_freq(interrupt_freq); //return 4 if valid, -1 if not
}

/*
 *   rtc_close
 *   DESCRIPTION: RTC close is a function that that returns success (0)
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int rtc_close(int32_t fd)
{
	return 0; //return 0 with success
}

/*
* rtc_set_freq
* DESCRIPTION: set RTC frequency (regA last 4 bits)
* INPUT: frequncy (4bytes from write function)
* OUTPUT: none
* RETURN: if input is a valid frequncy (power of 2 within bound), return 4 indicating 4 bytes written
*       otherwise return -1 indicating failure.
* SIDE_EFFECTS: none
*/

int rtc_set_freq(int32_t freq){
    /* Local variable*/
    char rate;
    int valid = 0;
    /* Save old value of Reg A*/
    outb(RTC_REG_A, RTC_PORT);
    unsigned char prevA = inb(CMOS_PORT); //store the value in reg A

    /* Values defined in RTC Datasheet (pg.19) */
    if (freq == 1024) {
    	rate = 0x06;	//0110
    	valid = 1;
    }
    if (freq == 512) {
    	rate = 0x07;	//0111
    	valid = 1;
    }
    if (freq == 256) {
    	rate = 0x08;	//1000
    	valid = 1;
    }
    if (freq == 128) {
    	rate = 0x09;	//1001
    	valid = 1;
    }
    if (freq == 64) {
    	valid = 1;
    	rate = 0x0A;	//1010
    }
    if (freq == 32) {
    	valid = 1;
    	rate = 0x0B;	//1011
    }
    if (freq == 16) {
    	valid = 1;
    	rate = 0x0C;	//1100
    }
    if (freq == 8) {
    	valid = 1;
    	rate = 0x0D;		//1101
    }
    if (freq == 4) {
    	valid = 1;
    	rate = 0x0E;		//1110
    }
    if (freq == 2) {
    	valid = 1;
    	rate = 0x0F;		//1111
    }

    /* set REG_A[3:0] (periodic interrupt rate) to rate */
    outb(RTC_REG_A, RTC_PORT);
    outb( ((prevA >> 4) << 4) | rate , CMOS_PORT);
    if (valid)
    	return 4;
    return -1;
}
