#define ASM 1


#include "interrupt_handler.h"

.data
    exc0 = 0
    exc1 = 1
    exc2 = 2
    exc3 = 3
    exc4 = 4
    exc5 = 5
    exc6 = 6
    exc7 = 7
    exc8 = 8
    exc9 = 9
    exc10 = 10
    exc11 = 11
    exc12 = 12
    exc13 = 13
    exc14 = 14
    exc16 = 16
    exc17 = 17
    exc18 = 18
    exc19 = 19
    RTC = -41
    KB = -34
    SYSCALL = 0x80
    PIT = -33


.text
.globl  EXCEPTION_0, EXCEPTION_1, EXCEPTION_2, EXCEPTION_3, EXCEPTION_4 
.globl  EXCEPTION_5, EXCEPTION_6, EXCEPTION_7, EXCEPTION_8, EXCEPTION_9
.globl  EXCEPTION_10, EXCEPTION_11, EXCEPTION_12, EXCEPTION_13, EXCEPTION_14
.globl  EXCEPTION_16, EXCEPTION_17, EXCEPTION_18, EXCEPTION_19
.globl  RTC_handler, KB_handler, PIT_handler, syscall
ret_val:		.long 0x0

EXCEPTION_0:
    pushal
    pushfl
    pushl    $exc0
    jmp     interrupt_handler

EXCEPTION_1:
    pushal
    pushfl
    pushl    $exc1
    jmp     interrupt_handler

EXCEPTION_2:
    pushal
    pushfl
    pushl    $exc2
    jmp     interrupt_handler

EXCEPTION_3:
    pushal
    pushfl
    pushl    $exc3
    jmp     interrupt_handler

EXCEPTION_4:
    pushal
    pushfl
    pushl    $exc4
    jmp     interrupt_handler

EXCEPTION_5:
    pushal
    pushfl
    pushl    $exc5
    jmp     interrupt_handler

EXCEPTION_6:
    pushal
    pushfl
    pushl    $exc6
    jmp     interrupt_handler

EXCEPTION_7:
    pushal
    pushfl
    pushl    $exc7
    jmp     interrupt_handler

EXCEPTION_8:
    pushal
    pushfl
    pushl    $exc8
    jmp     interrupt_handler

EXCEPTION_9:
    pushal
    pushfl
    pushl    $exc9
    jmp     interrupt_handler

EXCEPTION_10:
    pushal
    pushfl
    pushl    $exc10
    jmp     interrupt_handler

EXCEPTION_11:
    pushal
    pushfl
    pushl    $exc11
    jmp     interrupt_handler

EXCEPTION_12:
    pushal
    pushfl
    pushl    $exc12
    jmp     interrupt_handler

EXCEPTION_13:
    pushal
    pushfl
    pushl    $exc13
    jmp     interrupt_handler

EXCEPTION_14:
    pushal
    pushfl
    pushl    $exc14
    jmp     interrupt_handler

EXCEPTION_16:
    pushal
    pushfl
    pushl    $exc16
    jmp     interrupt_handler

EXCEPTION_17:
    pushal
    pushfl
    pushl    $exc17
    jmp     interrupt_handler

EXCEPTION_18:
    pushal
    pushfl
    pushl    $exc18
    jmp     interrupt_handler

EXCEPTION_19:
    pushal
    pushfl
    pushl    $exc19
    jmp     interrupt_handler


RTC_handler:
    pushal
    pushfl
    pushl   $RTC
    jmp     interrupt_handler

KB_handler:
    pushal
    pushfl
    pushl   $KB
    jmp     interrupt_handler

PIT_handler:
    pushal
    pushfl
    pushl   $PIT
    jmp     interrupt_handler

syscall:
    pushal
    pushfl
    pushl    $SYSCALL
    jmp     interrupt_handler


interrupt_handler:
    # get pushed number from stack
    popl    %edi
    cmpl    $0,   %edi
    jl  interrupt
    cmpl    $SYSCALL, %edi
    je      syscall_jump_sub
    cli
    jmp     handle
    
interrupt:
    notl    %edi
    
handle:
    call    *int_jumptable(,%edi,4)
    popfl
    popal
    iret        # interrupt return

syscall_jump_sub:
	cmpl	$1, %eax		# check if the syscall number is valid
	jl	invalid_sysnum
	cmpl	$10, %eax
	jg	invalid_sysnum
    pushl  %edx
    pushl  %ecx
    pushl  %ebx
    call    *syscall_jumptable(,%eax,4)
    movl	%eax, ret_val
    addl    $12, %esp
	jmp	finish_syscall
invalid_sysnum:
	movl	$-1, ret_val	# invalid syscall number should return -1
finish_syscall:
    popfl
    popal
    movl	ret_val, %eax
    iret



syscall_jumptable:
    .long   0
    .long   halt_func
    .long   execute_func
    .long   read_func
    .long   write_func
    .long   open_func
    .long   close_func
    .long   getargs_func
    .long   vidmap_func
    .long   set_handler_func 
    .long   sigreturn_func

    
int_jumptable: # functions written in C files
    .long   exception_0
    .long   exception_1
    .long   exception_2
    .long   exception_3
    .long   exception_4
    .long   exception_5
    .long   exception_6
    .long   exception_7
    .long   exception_8
    .long   exception_9
    .long   exception_10
    .long   exception_11
    .long   exception_12
    .long   exception_13
    .long   exception_14
    .long   exception_15
    .long   exception_16
    .long   exception_17
    .long   exception_18
    .long   exception_19
    .rept 12
    .long     exception_1
    .endr
    .long     pit_interrupt_handler
    .long     keyboard_interrupt_handler

    .rept 6
    .long 0
    .endr

    .long     rtc_interrupt_handler #40 --- 8*16= 128



