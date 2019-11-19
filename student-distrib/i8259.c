/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
static uint8_t master_mask = 0xff; /* IRQs 0-7  */
static uint8_t slave_mask = 0xff;  /* IRQs 8-15 */
#define MASTER_8259_DATA	MASTER_8259_PORT+1
#define SLAVE_8259_DATA		SLAVE_8259_PORT+1


/* i8259_init:
* input: none
* output: none
* description: Initialize the 8259 PIC */
void i8259_init(void) {
	unsigned long flags;    /* save flags */
	cli_and_save(flags);
	
	outb(master_mask, MASTER_8259_DATA);	/* mask all of 8259A-1 */
	outb(slave_mask, SLAVE_8259_DATA);	/* mask all of 8259A-2 */
	/*
		 * outb_p - this has to work on a wide range of PC hardware.
	*/
	//master
	outb(ICW1, MASTER_8259_PORT);	/* ICW1: select 8259A-1 init */
	/* ICW2: 8259A-1 IR0-7 mapped to ISA_IRQ_VECTOR(0) */
	outb(ICW2_MASTER, MASTER_8259_DATA);
	/* 8259A-1 (the master) has a slave on IR2 */
	outb(ICW3_MASTER, MASTER_8259_DATA);
	/* master does EOI */
	outb(ICW4, MASTER_8259_DATA);
	//slave
	outb(ICW1, SLAVE_8259_PORT);	/* ICW1: select 8259A-2 init */
	/* ICW2: 8259A-2 IR0-7 mapped to ISA_IRQ_VECTOR(8) */
	outb(ICW2_SLAVE, SLAVE_8259_DATA);
	/* 8259A-2 is a slave on master's IR2 */
	outb(ICW3_SLAVE, SLAVE_8259_DATA);
	/* (slave's support for EOI in flat mode is to be investigated) */
	outb(ICW4, SLAVE_8259_DATA);	
	//after initialization, enabling the slave IRQ line to the master: #2
	
	enable_irq(ICW3_SLAVE); //actually unmask master IR2 
	
    sti();
    restore_flags(flags);   /* restore flags */

}


/* enable_irq:
* input: none
* output: none
* description: Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
	//if master
	if (irq_num >= 0 && irq_num <= 7){
		master_mask &= ~(1 << irq_num); //update the mask,so that the ir is enabled
		outb(master_mask, MASTER_8259_DATA);
	}
	//else if enable slave pic
	else if (irq_num >= 8 && irq_num <= 15){
		slave_mask &= ~(1 << (irq_num - 8)); //update the mask for slave,so that the ir is enabled
		outb(slave_mask, SLAVE_8259_DATA);  //mask slave
	}
}

/* disable_irq:
* input:none
* output: none
* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
	//if master
	if(irq_num >= 0 && irq_num <=7){
		master_mask |= (1 << irq_num);//mask the input bit
		outb(master_mask, MASTER_8259_DATA);   
	}
	//else if mask salve
    else if(irq_num >= 8 && irq_num <= 15){
        slave_mask |= (1 << (irq_num-8)); // mask the salve bit
        outb(slave_mask, SLAVE_8259_DATA);
    }
}

/* send_eoi:
* input: none
* output:none 
* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
	//if from master, sufficient to issue eoi to master pic
	if(irq_num >= 0 && irq_num <= 7){
		outb(EOI | irq_num, MASTER_8259_PORT);
	}
	//if from the slave, 
	else{
		outb(EOI | (irq_num-8), SLAVE_8259_PORT);
		outb(EOI | 2, MASTER_8259_PORT);	//OR'ed with the irq number in master pic
	}
}
