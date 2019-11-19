/* Implement terminal driver */
#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include "keyboard.h"
#include "global.h"

#define NUM_TERM	3
#define TERMINAL_BUFFER_SIZE 128
#define TERMINAL_WIDTH 		80
#define TERMINAL_HEIGHT 	25
#define VIDEO 0xB8000
#define SCREEN_SIZE 2 * TERMINAL_HEIGHT * TERMINAL_WIDTH
#define VIDEOA 0xB9000
#define VIDEOB 0xBA000
#define VIDEOC 0xBB000

#define NULL_KEY    '\0'
#define NEWLINE '\n'
#define UNDERSCORE '_'
#define TAB_CHAR '\t'

// the counter for the next empty location in video memory
extern int current_location;
// for the video memory pointer
extern uint8_t* video_pointer;
extern uint8_t cur_pid ;
// One buffer for the keyboard for each terminal
extern char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
// the current length of the keyboard buffer
extern volatile unsigned int length_key;

typedef struct {
	uint32_t term_id;
	int32_t running_pid;
	uint32_t cursor_x;
	uint32_t cursor_y;
	int cursor_location;
	uint8_t* vid_backup;
	unsigned int len_key_buf;
	int has_enter;		// 0 - keyboard backup not empty but enter not pressed; 1 - keyboard backup empty
	char term_key_buf[KEYBOARD_BUFFER_SIZE];
	char term_last_buf[KEYBOARD_BUFFER_SIZE];
	uint16_t ss0;
	uint32_t esp0;
	uint32_t rtc_freq;
} term_info;

term_info term[NUM_TERM];


int32_t terminal_open(const uint8_t* filename);

int32_t terminal_close(int32_t fd);

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

void prep_term();

int32_t init_term();

int32_t launch_term(uint32_t term_id);

int32_t switch_terminal(uint32_t old_term, uint32_t new_term);

int32_t save_term_info(uint32_t old_term);

int32_t restore_term_info(uint32_t new_term);
#endif





