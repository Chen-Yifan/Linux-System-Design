#ifndef IDT_INIT_H
#define IDT_INIT_H

#include "x86_desc.h"


#define KB_IDT_ENTRY 0x21
#define RTC_IDT_ENTRY 0x28
/* Helper functions to initialize IDT */
extern void initialize_IDT();
extern void set_exceptions();
extern void set_interrupts();

#endif /* IDT_H */
