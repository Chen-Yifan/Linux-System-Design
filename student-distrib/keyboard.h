#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "types.h"
#include "global.h"

#define VIDEO 0xB8000
/* PIC Interrupt Line and IDT */
#define KEYBOARD_IRQ    1
#define KEY_MODES 4
#define KEY_COUNT 80
#define TERMINAL_WIDTH         80
#define TERMINAL_HEIGHT     25
#define KB_DATA 0x60
#define KB_COMMAND 0x64

// pressed or not press shift/ctrl
#define PRESSED    1
#define    UNPRESSED 0

// special characters
#define BACKSPACE 0x0E
#define TAB    0x0F
#define CAP    0x3A
#define ENTER 0x1C
#define LSHIFT_DOWN 0x2A
#define RSHIFT_DOWN 0x36
#define LSHIFT_UP (0x2A|0x80)
#define RSHIFT_UP (0x36|0x80)
#define CTRL_DOWN 0x1D
#define CTRL_UP    (0x1D|0x80)
#define ALT_DOWN	0x38
#define ALT_UP		0xB8
#define LETTER_L 0x26
#define LETTER_C 0x2E
#define NULL_KEY    '\0'
#define NEWLINE '\n'
#define UNDERSCORE '_'
#define TAB_CHAR '\t'
#define BLACK 0x07
#define F1	0x3B
#define F2	0x3C
#define F3	0x3D
#define PAGE_UP 0x49
#define DOWN 0x50


extern int screen_x;
extern int screen_y;


/* Initialize Keyboard */
void init_keyboard();

/* Keyboard Handler */
void keyboard_interrupt_handler();

/* Called when enter key needs to be handled */
void handle_enter();

/* Called when backspace key needs to be handled*/
void handle_backspace();

/* Called when we reach the end of the video memory and
 * need to rearrange the screen contents */
void handle_scroll();

/* Called when tab key needs to be handled*/
void handle_tab();

int process_char(int scode);

void clear_keyboard_buffer();

void clear_keyboard_backup(uint32_t term_id);

void read_buffer();
void show_keyboard_buffer();
void show_last_command();

#endif

