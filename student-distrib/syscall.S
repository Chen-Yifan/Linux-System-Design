#define ASM 1
#include "syscall.h"

#before it: have done these in interrupt_handler.S
# save all regs 
# call correct sys call
# restore reg
# IRET

# now we need the linkage to our real function of syscall

// Define SYS_CALL number
#define SYS_HALT    1
#define SYS_EXECUTE 2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETARGS 7
#define SYS_VIDMAP  8
#define SYS_SET_HANDLER  9
#define SYS_SIGRETURN  10

# handle each case for the same
/* 
 * Rather than create a case for each number of arguments, we simplify
 * and use one macro for up to three arguments; the system calls should
 * ignore the other registers, and they're caller-saved anyway.
 */
 
#define DO_CALL(name,number)   \
.GLOBL name                   ;\
name:   PUSHL	%EBX          ;\
	MOVL	$number,%EAX  ;\
	MOVL	8(%ESP),%EBX  ;\
	MOVL	12(%ESP),%ECX ;\
	MOVL	16(%ESP),%EDX ;\
	INT	$0x80         ;\
	POPL	%EBX          ;\
	RET

# the system call library wrappers 
DO_CALL(halt,SYS_HALT)
DO_CALL(execute,SYS_EXECUTE)
DO_CALL(read,SYS_READ)
DO_CALL(write,SYS_WRITE)
DO_CALL(open,SYS_OPEN)
DO_CALL(close,SYS_CLOSE)
DO_CALL(getargs,SYS_GETARGS)
DO_CALL(vidmap,SYS_VIDMAP)
DO_CALL(set_handler,SYS_SET_HANDLER)
DO_CALL(sigreturn,SYS_SIGRETURN)
