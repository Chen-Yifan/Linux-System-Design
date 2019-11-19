/*
*	keyboard.c - handles interrupts received by keyboard and allows
*				 interfacing between keyboard and processor
*/

#include "keyboard.h"
#include "terminal.h"
#include "lib.h"
#include "types.h"
#include "i8259.h"
#include "x86_desc.h"
#include "syscall_handler.h"
#include "global.h"
#include "pit.h"


// the counter for the next empty location in video memory
int current_location = 0;
// for the video memory pointer
uint8_t* video_pointer = (uint8_t*) VIDEO;

// buffer for the keyboard
char keyboard_buffer[KEYBOARD_BUFFER_SIZE] = {0};
// the current length of the keyboard buffer
volatile unsigned int length_key = 0;

// mode specification
// 0: neither shift or caps
// 1: shift enabled
// 2: caps enabled
// 3: both are enabled
static uint8_t key_mode = 0;

// the status of shift and ctrl and cap
static int cap_on = UNPRESSED;
static int ctrl_pressed = UNPRESSED;
static int alt_pressed = UNPRESSED;

/* KEYBOARD SCANCODE */
static uint8_t scancode_map[KEY_MODES][KEY_COUNT] = {
	// no caps / no shift
	{'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
	 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0', 'a', 's',
	 'd', 'f', 'g', 'h', 'j', 'k', 'l' , ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
	 'b', 'n', 'm',',', '.', '/', '\0', '*', '\0', ' ', '\0'},
	// no caps / shift
	{'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
	 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0', '\0', 'A', 'S',
	 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ':', '"', '~', '\0', '|', 'Z', 'X', 'C', 'V',
	 'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0'},
	// caps / no shift
	{'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
	 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0', '\0', 'A', 'S',
	 'D', 'F', 'G', 'H', 'J', 'K', 'L' , ';', '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V',
	 'B', 'N', 'M', ',', '.', '/', '\0', '*', '\0', ' ', '\0'},
	// caps / shift
	{'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
	 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0', '\0', 'a', 's',
	 'd', 'f', 'g', 'h', 'j', 'k', 'l' , ':', '"', '~', '\0', '\\', 'z', 'x', 'c', 'v',
	 'b', 'n', 'm', '<', '>', '?', '\0', '*', '\0', ' ', '\0'}
};

/*
 *    Function: keyboard_interrupt_hander()
 *    Description: this function check for the input character and output the correct
 *    char to the console.
 *    inputs:        nothing
 *    outputs:    nothing
 *    effects:   increase the current location of in the video memory.
 */
void keyboard_interrupt_handler(){
	// clear interrupt
	cli();
	int scancode = 0;
	int i;
	int c = inb(KB_DATA);
	send_eoi(KEYBOARD_IRQ);
	// wait for the interrupt
	if( c != 0) {
		scancode = c;
		// check if c is a valid scancode
		if( scancode >= KEY_COUNT || scancode < 0) {
			if (scancode == LSHIFT_UP || scancode == RSHIFT_UP) {
				if(key_mode == 2 || key_mode == 0){
				//	send_eoi(KEYBOARD_IRQ);
					sti();
					return;
				}
				// cap is on
				if(key_mode == 3)
					key_mode = 2;
				// cap is not on
				else
					key_mode = 0;
				//send_eoi(KEYBOARD_IRQ);
				sti();
				return;
			}
			if (scancode == CTRL_UP) {
				ctrl_pressed = UNPRESSED;
				//send_eoi(KEYBOARD_IRQ);
				sti();
				return;
			}

			if (scancode == ALT_UP) {
				alt_pressed = UNPRESSED;
				//send_eoi(KEYBOARD_IRQ);
				sti();
				return;
			}
		//	send_eoi(KEYBOARD_IRQ);
			sti();
			return;
		}
		i = process_char(scancode);
	}
	//send_eoi(KEYBOARD_IRQ);
	sti();
	// halt for ctrl+C
	if (i==4){
		halt(1); //halt current program
	}
	// launch term for alt+F1/2/3
	else if(i!=0) {
		launch_term(i-1);
	}
	return;
}


/*
*	Function: init_keyboard()
*	Description: This function initializes the keyboard to the appropriate
*				 IRQ Line on the PIC (this being Line#1 according to IDT table)
*	inputs:		nothing
*	outputs:	nothing
*	effects:	enables line 1 on the master PIC
*/
void init_keyboard(){
	enable_irq(KEYBOARD_IRQ);
	length_key = 0;
}

/*
*	Function: clear_keyboard_buffer()
*	Description: This function clears the contents in the keyboard buffer
*	(for full buffer or other reset)
*	inputs:		nothing
*	outputs:	nothing
*	effects:	clears the content in buffer
*/
void clear_keyboard_buffer() {
	int i;
	cli();
	for(i = 0; i <= length_key; i++) {
		keyboard_buffer[i] = NULL_KEY;
	}
	length_key = 0;
	sti();
	return;
}

/*
*	Function: clear_keyboard_backup()
*	Description: This function clears the contents in the terminal keyboard buffer backup
*	(for full buffer or other reset)
*	inputs:		term_id - the target terminal
*	outputs:	nothing
*	effects:	clears the content in buffer
*/
void clear_keyboard_backup(uint32_t term_id) {
	int i;
	cli();
	for(i = 0; i <= term[term_id].len_key_buf; i++) {
		term[term_id].term_key_buf[i] = NULL_KEY;


	}
	term[term_id].len_key_buf = 0;
	sti();
	return;
}

/*
*	Function: process_char()
*	Description: this function changes the scode into the correct
*	char to be printed to the console or do the special change for
*	the special cases
*	inputs:		scancode of a character
*	outputs:	return 0 for normal situation
*				return 1/2/3 for terminal switch
*				return 4 for halting
*	effects:	may printing the character to the screen or change the
*	states of special characters and key modes; besides, scroll the screen
*	if it is full.
*/
int process_char(int scode) {
	uint8_t c;
	// check for the input char for special cases
	switch(scode) {
		case BACKSPACE:
			handle_backspace();
			return 0;
		case TAB:
			// else, return if the buffer is full
			if(length_key == KEYBOARD_BUFFER_SIZE)
				return 0;
			handle_tab();
			// update the keyboard buffer
			keyboard_buffer[length_key++] = TAB_CHAR;
			return 0;
		case CAP:
			if(cap_on) {
				cap_on = UNPRESSED;
				// shift is not pressed
				if(key_mode == 2)
					key_mode = 0;
				// shift is pressed
				else
					key_mode = 1;
			}
			else {
				cap_on = PRESSED;
				// shift is not pressed
				if(key_mode == 0)
					key_mode = 2;
				// shift is pressed
				else
					key_mode = 3;
			}
			return 0;
		case LSHIFT_DOWN:
		case RSHIFT_DOWN:
			if(key_mode == 3 || key_mode == 1)
				return 0;
			// cap is on
			if(key_mode == 2)
				key_mode = 3;
			// cap is not on
			else
				key_mode = 1;
			return 0;
		case ALT_DOWN:
			alt_pressed = PRESSED;
			return 0;
		case F1:
			if (alt_pressed == PRESSED) {
				return 1;		// Switch to terminal 1
			}
			return 0;
		case F2:
			if (alt_pressed == PRESSED) {
				return 2;		// Switch to terminal 2
			}
			return 0;
		case F3:
			if (alt_pressed == PRESSED) {
				return 3;		// Switch to terminal 3
			}
			return 0;
		case CTRL_DOWN:
			ctrl_pressed = PRESSED;
			return 0;
		case ENTER:
			// else, return if the buffer is full
			if(length_key == KEYBOARD_BUFFER_SIZE)
				return 0;
			handle_enter();
			// update the keyboard buffer
			keyboard_buffer[length_key++] = NEWLINE;
			return 0;
		case LETTER_L:
			// check if we need to handle ctrl+l case
			if(ctrl_pressed) {
				// clear the screen
				clear();
				current_location = 0;
				// put the cursor to the top
				ctrl_pressed = UNPRESSED;
				update_cursor(current_location/2);
				clear_keyboard_buffer();
				return 0;
			}
		case LETTER_C:
			// check if we need to handle ctrl+l case
			if(ctrl_pressed) {
				ctrl_pressed = UNPRESSED;
				return 4;
			}
		//case PAGE_UP:
			//show_last_command();
			//return 0;
		default:
			c = scancode_map[key_mode][scode];
			// else, return if the buffer is full
			if(length_key == KEYBOARD_BUFFER_SIZE - 1)
				return 0;
			// update the keyboard buffer
			keyboard_buffer[length_key++] = c;
			break;
	}
	if(current_location < TERMINAL_WIDTH * TERMINAL_HEIGHT*2) {
		putc(c);
		if(current_location == TERMINAL_WIDTH * TERMINAL_HEIGHT*2)
			handle_scroll();
		update_cursor(current_location/2);
	}
	// we need to scroll the screen otherwise
	else{
		handle_scroll();
		putc(c);
		update_cursor(current_location/2);
	}
	return 0;
}


/*
*	Function: handle_enter()
*	Description: this function is called when enter key needs to be handled
*	inputs:		none
*	outputs:	none
*	effects:	begin a new line on the screen; besides, scroll the screen
*	if it is full.
*/
void handle_enter() {
	int cur_y = current_location / (TERMINAL_WIDTH*2);
	// first, check whether the buffer is full
	if(length_key == KEYBOARD_BUFFER_SIZE)
		return;
	// need to scroll the screen if next line is out of place
	if((cur_y+1) == TERMINAL_HEIGHT){
		handle_scroll();
		update_cursor(current_location/2);
	}
	else{
		// update the cursor location
		current_location = (cur_y+1) * TERMINAL_WIDTH * 2;
		update_cursor(current_location/2);
	}
	term[curr_term].has_enter = 1;
}

/*
*	Function: handle_backspace()
*	Description: this function is called when backspace key needs to be handled
*	inputs:		none
*	outputs:	none
*	effects:	decrement the current location and erase one char;
*	deletes one item from keyboard buffer.
*/
void handle_backspace(){
	// check if it is valid to delete
	if(current_location == 0)
		return;
	// the keyboard buffer is empty, do nothing
	if(length_key== 0)
		return;
	// handle backtrace for special case
	if(keyboard_buffer[length_key-1] == '\t') {
		 current_location -= 8;
	}
	else if(keyboard_buffer[length_key-1] == '\n') {
		 if(length_key > 1 && keyboard_buffer[length_key-2] == '\n')
			 current_location -= TERMINAL_WIDTH*2;
		 else {
			 while(video_pointer[current_location] != keyboard_buffer[length_key-2]) {
				 current_location--;
			 }
			 // get to the next empty space
			 current_location +=2;
		 }
	}
	else {
		current_location -= 2;
		//video_pointer[current_location] = NULL_KEY;
		//video_pointer[current_location+1] = BLACK;
		video_pointer[current_location] = NULL_KEY;
		if(curr_term == 0)
			video_pointer[current_location+1] = ATTRIB_TERM1;
		else if(curr_term == 1)
			video_pointer[current_location+1] = ATTRIB_TERM2;
		else
			video_pointer[current_location+1] = ATTRIB_TERM3;
	}
	// clear the specific content in the buffer
	keyboard_buffer[length_key-1] = NULL_KEY;
	length_key--;
	update_cursor(current_location/2);
	return;
}

/*
*	Function: handle_tab()
*	Description: this function is called when tab key needs to be handled
*	inputs:		none
*	outputs:	none
*	effects:	increase the current location by 4; return if the buffer is full
*	check for scrolling if needed
*/
void handle_tab() {
	int tab_len = 4;
	int i;
	// first, check whether the buffer is full
	if(length_key == KEYBOARD_BUFFER_SIZE)
		return;
	for(i = 0; i < tab_len; i++) {
		if(current_location == TERMINAL_WIDTH * TERMINAL_HEIGHT)
			handle_scroll();
		// [0][57] is the space char ' '
		video_pointer[current_location++] = scancode_map[0][57];
		video_pointer[current_location++] = BLACK;
		if(current_location == TERMINAL_WIDTH * TERMINAL_HEIGHT*2)
			handle_scroll();
		// update the cursor
		update_cursor(current_location/2);
	}
	return;
}

/*
*	Function: handle_scroll()
*	Description: this function handles the situation that we need
*	to scroll the screen.
*	inputs:		none
*	outputs:	none
*	effects: move the contents in the video memory one line up and update
*	the current location to the first location in the new line.
*/
void handle_scroll() {
	int i;
	// change the contents by moving one line up
	for(i = TERMINAL_WIDTH*2; i < TERMINAL_HEIGHT*TERMINAL_WIDTH*2; i = i+2) {
		video_pointer[i - TERMINAL_WIDTH*2] = video_pointer[i];
	}
	// clear the last line
	for(i = (TERMINAL_HEIGHT-1)*TERMINAL_WIDTH*2; i < TERMINAL_HEIGHT*TERMINAL_WIDTH*2; i = i + 2) {
		video_pointer[i] = NULL_KEY;
	}
	current_location = (TERMINAL_HEIGHT-1)*TERMINAL_WIDTH*2;
	return;
}

/*
*	Function: read_buffer()
*	Description: this function read the contents in the current keyboard
*	buffer when the "enter" is pressed.
*	inputs:		none
*	outputs:	none
*	effects:  print the contents in the keyboard buffer to the screen
*/
void read_buffer() {
	while(term[running_term].has_enter == 0) {}
	term[running_term].has_enter = 0;
	return;
}
