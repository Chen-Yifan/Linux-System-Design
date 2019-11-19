#include "idt_init.h"
#include "exception_handler.h"
#include "interrupt_handler.h"

/*
 * initialize_IDT
 *		DESCRIPTION: Initialize the IDT
 *		INPUTS: none
 *		OUTPUTS: none
 *		RETURN VALUES: none
 *		SIDE EFFECT: Initialize all the exceptions, interrupts, and system calls
 *						in IDT
 */
void initialize_IDT() {
	int i;
	for (i=0; i < NUM_VEC; i++){
		idt[i].seg_selector = KERNEL_CS;
		idt[i].reserved4 = 0;
		idt[i].size = 1;
		idt[i].reserved3 = 0;           /* Interrupt uses interrupt gate */
		idt[i].reserved2 = 1;
		idt[i].reserved1 = 1;
		idt[i].reserved0 = 0;
		idt[i].dpl = 0;
		idt[i].present = 1;
		if (i < 32) {
			idt[i].reserved3 = 1;		/* Exception uses trap gate */
		}
		if (i == 0x80) {
			idt[i].reserved3 = 1;		/* System call uses trap gate */
			idt[i].dpl = 3;				/* System call has DPL = 3 */
		}
	}

	/* Set up exceptions and interrupts with helper functions */
	set_exceptions();
	set_interrupts();

	/* Set up system calls in IDT */
	SET_IDT_ENTRY(idt[0x80], syscall);	/* 0x80 is the required entry for system call */
	
}

/*
 * set_exceptions
 *		DESCRIPTION: Set all the exceptions in IDT
 *		INPUTS: none
 *		OUTPUTS: none
 *		RETURN VALUES: none
 *		SIDE EFFECT: Fill the entries for exceptions in IDT
 */
void set_exceptions() {
	/* From the x86 ISA manual, we can only use exception 0-19 (except 15) within the first 32 exceptions */
	SET_IDT_ENTRY(idt[0], EXCEPTION_0);
	SET_IDT_ENTRY(idt[1], EXCEPTION_1);
	SET_IDT_ENTRY(idt[2], EXCEPTION_2);
	SET_IDT_ENTRY(idt[3], EXCEPTION_3);
	SET_IDT_ENTRY(idt[4], EXCEPTION_4);
	SET_IDT_ENTRY(idt[5], EXCEPTION_5);
	SET_IDT_ENTRY(idt[6], EXCEPTION_6);
	SET_IDT_ENTRY(idt[7], EXCEPTION_7);
	SET_IDT_ENTRY(idt[8], EXCEPTION_8);
	SET_IDT_ENTRY(idt[9], EXCEPTION_9);
	SET_IDT_ENTRY(idt[10], EXCEPTION_10);
	SET_IDT_ENTRY(idt[11], EXCEPTION_11);
	SET_IDT_ENTRY(idt[12], EXCEPTION_12);
	SET_IDT_ENTRY(idt[13], EXCEPTION_13);
	SET_IDT_ENTRY(idt[14], EXCEPTION_14);
	SET_IDT_ENTRY(idt[16], EXCEPTION_16);
	SET_IDT_ENTRY(idt[17], EXCEPTION_17);
	SET_IDT_ENTRY(idt[18], EXCEPTION_18);
	SET_IDT_ENTRY(idt[19], EXCEPTION_19);
	SET_IDT_ENTRY(idt[20], EXCEPTION_1);        //all these unused ENTRY set to reserved
	SET_IDT_ENTRY(idt[21], EXCEPTION_1);
	SET_IDT_ENTRY(idt[22], EXCEPTION_1);
	SET_IDT_ENTRY(idt[23], EXCEPTION_1);
	SET_IDT_ENTRY(idt[24], EXCEPTION_1);
	SET_IDT_ENTRY(idt[25], EXCEPTION_1);
	SET_IDT_ENTRY(idt[26], EXCEPTION_1);
	SET_IDT_ENTRY(idt[27], EXCEPTION_1);
	SET_IDT_ENTRY(idt[28], EXCEPTION_1);
	SET_IDT_ENTRY(idt[29], EXCEPTION_1);
	SET_IDT_ENTRY(idt[30], EXCEPTION_1);
	SET_IDT_ENTRY(idt[31], EXCEPTION_1);

	return;
}

/*
 * set_interrupts
 *		DESCRIPTION: Set all the interrupts in IDT
 *		INPUTS: none
 *		OUTPUTS: none
 *		RETURN VALUES: none
 *		SIDE EFFECT: Fill the entries for interrupts in IDT
 */
void set_interrupts() {
	/* Set the keyboard and RTC interrupts */
	SET_IDT_ENTRY(idt[40], RTC_handler);
	SET_IDT_ENTRY(idt[33], KB_handler);
	SET_IDT_ENTRY(idt[32], PIT_handler);

	return;
}
