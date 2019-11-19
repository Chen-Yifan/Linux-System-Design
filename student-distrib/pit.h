#ifndef _PIT_H
#define _PIT_H

#include "lib.h"
#include "i8259.h"
#include "global.h"

extern uint32_t running_term;
extern uint32_t next_term;
extern uint32_t next_process;

#define PIT_IRQ_NUM 0
#define PIT_IDT_ENTRY 0x20

#define CMD_PORT 0x43
#define DATA_PORT0 0x40
#define CMD_REG_VAL 0x36  //command register is set to 0x36

#define PIT_MAX_FREQ 1193180
#define PIT_FREQ 11932
#define FREQ_MASK 0xFF

void pit_init();
void pit_interrupt_handler();  //interrupt handler
void schedule(uint32_t process);   //do the schedule
uint32_t get_next_process();	   // search the next term for scheduling

#endif
