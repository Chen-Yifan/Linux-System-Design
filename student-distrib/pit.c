#include "pit.h"
#include "terminal.h"
#include "paging.h"
#include "syscall_handler.h"
#include "lib.h"

//uint32_t pit_counter;
uint32_t running_term = 0;
uint32_t next_term = 0;
uint32_t next_process = 0;

/*
 * pit_init
 *   DESCRIPTION: initialize pit device to avoid getting an undefined state.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enables pit_interrupt_handler to use pit
 *                 sets the frequency of pit to 100 Hz
 */
void pit_init(){

	outb(CMD_REG_VAL, CMD_PORT);
    outb(PIT_FREQ & FREQ_MASK, DATA_PORT0);   /* Set low byte of divisor */
    outb(PIT_FREQ >> 8, DATA_PORT0);     /* Set high byte of divisor, first 8 bits */
	
  	enable_irq(PIT_IRQ_NUM); 
}

/*
 * pit_interrupt_handler
 *   DESCRIPTION: handle pit interrupt.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle pit interrupt
 */
void pit_interrupt_handler(){
	send_eoi(PIT_IRQ_NUM); //irq 0,send eoi
	cli();
	if (term[1].running_pid != -1 || term[2].running_pid != -1) {
		next_process = get_next_process();
		schedule(next_process); //current kernal that need to be scheduled to CPU
	}
	sti();
	return;
}


/*
 * schedule
 *   DESCRIPTION: do the context switch.
 *   INPUTS: next process number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: context switch
 */
void schedule(uint32_t process){
	
	// remap video memory of this program (text-mode print)
	uint8_t* screen_start;
	vidmap(&screen_start); //132MB
	
	// get the new terminal
	term_info new_terminal = term[next_term];
	
	// check if the task belongs to the active terminal
	if (new_terminal.term_id != curr_term){ //not the active terminal
		remap_vid((int32_t)screen_start, (int32_t)new_terminal.vid_backup);	// include flush TLB
	}

		//save tss ss0	
	term[running_term].esp0 = tss.esp0;
	term[running_term].ss0 = tss.ss0;


	// get current pcb
	pcb_t* curr_pcb = get_specific_pcb((uint8_t)term[running_term].running_pid);

	pcb_t* new_pcb = get_specific_pcb((uint8_t)process);

	asm volatile(
	"movl %%esp, %%eax;"
	"movl %%ebp, %%ebx;"
	:"=a"(curr_pcb->curr_esp), "=b"(curr_pcb->curr_ebp)		/* outputs */
	:												/* no input */
	);

	running_term = next_term;
	// first map a page from virtual to physical
	remap(_128MB , _8MB + process*_4MB);
	// restore tss
	tss.ss0 = new_terminal.ss0; // KERNEL_DS;
	tss.esp0 = new_terminal.esp0; //the current process' stack base

	cur_pid = process; //used in read and write and so on

	asm volatile(
	"movl %%eax, %%esp;"
	"movl %%ebx, %%ebp;"
	"leave;"
	"ret;"
	:												/* no outputs */
	:"a"(new_pcb->curr_esp), "b"(new_pcb->curr_ebp)			/* input */
	);

}


/*
 * get_next_process
 *   DESCRIPTION: search for the next process number.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: search the next process
 */
uint32_t get_next_process(){
	int i;
	next_term = running_term;
	for (i=0;i<NUM_TERMS;i++){						// at most 3 searches,from the current term
		next_term = (next_term+1) % NUM_TERMS;
		if (term[next_term].running_pid != -1){	// search next process
			break;
		}
	}
	return term[next_term].running_pid;
}
	
