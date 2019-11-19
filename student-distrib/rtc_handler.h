#ifndef _RTC_H
#define _RTC_H

#include "types.h"

#define RTC_PORT	0x70
#define CMOS_PORT 0x71
#define RTC_IRQ_NUM 8
#define RTC_REG_A 0x0a
#define RTC_REG_B 0x0b
//Status Register C will contain a bitmask telling which interrupt happened
#define RTC_REG_C 0x0c
#define RTC_REG_D 0x0d
#define NMI_BIT 0x80
#define RTC_PIE 0x40

#define MAX_RTC_FREQ 1024
#define MIN_RTC_FREQ 2

#define MAX_FREQ 0x6
#define MIN_FREQ 0xF


void rtc_init();
void rtc_interrupt_handler();
int rtc_open(const uint8_t* filename);
int rtc_read(int32_t fd, void* buf, int32_t nbytes);
int rtc_write(int32_t fd, const void* buf, int32_t nbytes);
int rtc_close(int32_t fd);
int rtc_set_freq(int32_t freq);

#endif

