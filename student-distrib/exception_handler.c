#include "exception_handler.h"

/*
 * squash
 *	DESCRIPTION: squash all user-level programs by infinite loop
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: squash all the user-level programs
 */
void squash() {
	while (1);
	return;
}

/*
 * print_err_addr
 *	DESCRIPTION: print the memory address reference that causes exception
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the memory address reference that causes exception
 */
void print_err_addr() {
	uint32_t eip;
	/* Print the address casuing the exception */
	asm volatile("1: lea 1b, %0;": "=a"(eip));
	printf("Error Address: %h\n", eip);
}
/*
 * exception_0
 *	DESCRIPTION: handle exception 0
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_0() {
	printf("Divide Error Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_1
 *	DESCRIPTION: handle exception 1
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_1() {
	printf("Intel Reserved Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_2
 *	DESCRIPTION: handle exception 2
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_2() {
	printf("NMI Interrupt Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_3
 *	DESCRIPTION: handle exception 3
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_3() {
	printf("Breakpoint Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_4
 *	DESCRIPTION: handle exception 4
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_4() {
	printf("Overflow Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_5
 *	DESCRIPTION: handle exception 5
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_5() {
	printf("BOUND Range Exceeded Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_6
 *	DESCRIPTION: handle exception 6
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_6() {
	printf("Invalid Opcode (Undefined Opcode) Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_7
 *	DESCRIPTION: handle exception 7
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_7() {
	printf("Device Not Available (No Math Coprocessor) Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_8
 *	DESCRIPTION: handle exception 8
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_8() {
	printf("Double Fault Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_9
 *	DESCRIPTION: handle exception 9
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_9() {
	printf("Coprocessor Segment Overrun Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_10
 *	DESCRIPTION: handle exception 10
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_10() {
	printf("Invalid TSS Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_11
 *	DESCRIPTION: handle exception 11
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_11() {
	printf("Segment Not Present Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_12
 *	DESCRIPTION: handle exception 12
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_12() {
	printf("Stack-Segment Fault Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_13
 *	DESCRIPTION: handle exception 13
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_13() {
	printf("General Protection Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_14
 *	DESCRIPTION: handle exception 14
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_14() {
	printf("Page Fault Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_15
 *	DESCRIPTION: handle exception 15
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_15() {
	printf("INVALID_TSS_handler\n");
	print_err_addr();
	squash();
}

/*
 * exception_16
 *	DESCRIPTION: handle exception 16
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_16() {
	printf("x87 FPU Floating-Point Error (Math Fault) Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_17
 *	DESCRIPTION: handle exception 17
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_17() {
	printf("Alignment Check Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_18
 *	DESCRIPTION: handle exception 18
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_18() {
	printf("Machine Check Exception\n");
	print_err_addr();
	squash();
}

/*
 * exception_19
 *	DESCRIPTION: handle exception 19
 *	INPUTs: none
 *	OUTPUTS: none
 *	RETURN VALUES: none
 *	SIDE EFFECT: print the exception message and squash user-level programs
 */
void exception_19() {
	printf("SIMD Floating-Point Exception\n");
	print_err_addr();
	squash();
}
