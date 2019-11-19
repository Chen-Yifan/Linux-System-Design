#include "terminal.h"
#include "keyboard.h"
#include "lib.h"
#include "types.h"
#include "syscall.h"
#include "syscall_handler.h"
#include "pit.h"
#include "paging.h"
#include "rtc_handler.h"

// the current length of the terminal buffer
//static volatile unsigned int length_term;
/*
 *    Function: terminal_open
 *    Description: initializes terminal stuff.
 *    inputs:        nothing
 *    outputs:    return 0
 *    effects:   clear the screen, set up the cursor, 
 *	  and initialize the variables related to the terminal.
 */
int32_t terminal_open(const uint8_t* filename){
	clear();
	// set up the cursor, [1][12] is the char '_'
	update_cursor(current_location/2);
	//length_term = 0;
	return 0;
}

/*
 *    Function: terminal_close()
 *    Description: clears any terminal specific variables.
 *    inputs:        nothing
 *    outputs:    return 0
 *    effects:   clears any terminal specific variables.
 */
int32_t terminal_close(int32_t fd){
	//length_term = 0;
	clear();
	return 0;
}

/*
 *    Function: terminal_read
 *    Description: this function reads FROM the keyboard buffer into buf.
 *    inputs:        nothing
 *    outputs:  return number of bytes read, -1 for empty pointer
 *    effects: 
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
	if (buf == NULL){
		return -1;
	}
	read_buffer();
	int i;
	int8_t * buffer = (int8_t *) buf;
	for(i = 0; i < nbytes; i++) {
		// check if the buf is full
		if(i < length_key)
			buffer[i] = keyboard_buffer[i];

	}
	if(length_key < nbytes) {
		i = length_key;
	}
	clear_keyboard_buffer();
	return (int32_t)i;
}

/*
 *    Function: terminal_write
 *    Description: writes the character to the screen from buffer.
 *    inputs:        nothing
 *    outputs: return number of bytes written or -1
 *    effects:  clear 
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
	int i;
	if (buf == NULL){
		return -1;
	}
	int8_t * buffer = (int8_t *) buf;
	
	// reset the buffer
	if (running_term == curr_term) {
		for(i=0;i<nbytes;i++){
			putc(buffer[i]);
		}
	} else {
		for(i=0;i<nbytes;i++){
			putc_term(buffer[i], running_term);
		}
	}
	return i+1;
}

/*
 *    Function: prep_term
 *    Description: initialize all the three terminal's running pid
 *    inputs: none
 *    outputs: none
 *	  return value: none
 *    effects: initialize all the three terminal's running pid
 */
void prep_term() {
	int i;
	for (i = 0; i < NUM_TERM; i++) {
		term[i].running_pid = -1;
	}
	return;
}

/*
 *    Function: init_term
 *    Description: initialize all the three terminal structures, and launch the first terminal
 *    inputs: none
 *    outputs: none
 *	  return value: 0 for success and -1 for failure
 *    effects: initialize all the three terminal structures, and launch the first terminal
 */
int32_t init_term() {
	int i;
	
	term[0].term_id = 0;
	term[1].term_id = 1;
	term[2].term_id = 2;
	/* Each video memory backup takes a 4KB page after the video memory location 0xB8000 */
	term[0].vid_backup = (uint8_t*) VIDEOA;
	term[1].vid_backup = (uint8_t*) VIDEOB;
	term[2].vid_backup = (uint8_t*) VIDEOC;


	for (i = 0; i < 3; i++) {
		term[i].cursor_location = 0;
		term[i].cursor_x = 0;
		term[i].cursor_y = 0;
		clear_backup(i);
		term[i].len_key_buf = 0;
		clear_keyboard_backup(i);
		term[i].has_enter = 0;
		term[i].rtc_freq = MAX_RTC_FREQ;
	}

	curr_term = 0;		/* We launch the first terminal in the beginning */
	restore_term_info(0);
	return execute((uint8_t*) "shell");
}

/*
 *    Function: launch_term
 *    Description: launch a terminal based on the input terminal id
 *    inputs: term_id - the id number of the terminal to be launched
 *    outputs: none
 *	  return value: 0 for success and -1 for failure
 *    effects: launch the terminal with the input id
 *///do the switch here!
int32_t launch_term(uint32_t term_id) {
	cli();

	if (term_id == curr_term) {
		sti();
		return 0;	/* No need to switch */
	}

	if (term[term_id].running_pid == -1){ //store current esp ebp of curr_process
		switch_terminal(curr_term, term_id);
		pcb_t * old_pcb = get_specific_pcb(term[running_term].running_pid);
		curr_term = term_id;
		asm volatile("			\n\
	            movl %%ebp, %%eax 	\n\
	            movl %%esp, %%ebx 	\n\
	            "
	            :"=a"(old_pcb->curr_ebp), "=b"(old_pcb->curr_esp)
	     ); //if it's the first process of a term, it will store old pcb's info
		sti();
		execute((uint8_t*) "shell");
	} else {
		if (switch_terminal(curr_term, term_id) != 0) { //curr_term changed
			sti();
			return -1;	/* Switch the current terminal to new terminal */
		}
		curr_term = term_id;
		uint8_t* screen_start;
	    vidmap(&screen_start);
	    if (running_term != curr_term) {
	    	remap_vid((int32_t)screen_start, (int32_t)term[running_term].vid_backup);
	    }
		sti();
	}
	return 0;
}


/* only change some information
 *    Function: switch_terminal
 *    Description: switch from the old terminal to the new terminal.
 *    inputs: old_term - the current terminal id
 *			  new_term - the terminal id to switch to
 *    outputs: none
 *	  return value: 0 for success and -1 for failure
 *    effects:  save the current terminal info and restore the new terminal info
 */
int32_t switch_terminal(uint32_t old_term, uint32_t new_term) {
	if (save_term_info(old_term) != 0)
		return -1;
	if (restore_term_info(new_term) != 0)
		return -1;
	return 0;
}

/*
 *    Function: save_term_info
 *    Description: save the terminal information for the current terminal.
 *    inputs: old_term - the current terminal id
 *    outputs: none
 *	  return value: 0 for success
 *    effects:  save the current terminal info
 */
int32_t save_term_info(uint32_t old_term) {
	term[old_term].cursor_x = screen_x;
	term[old_term].cursor_y = screen_y;
	term[old_term].cursor_location = current_location;
	term[old_term].len_key_buf = length_key;
	clear_keyboard_backup(old_term);
	memcpy(term[old_term].vid_backup, (uint8_t*) VIDEO, SCREEN_SIZE);
	memcpy((uint8_t*) term[old_term].term_key_buf, (uint8_t*) keyboard_buffer, KEYBOARD_BUFFER_SIZE);
	term[old_term].len_key_buf = length_key;
	clear_keyboard_buffer();
	return 0;
}


/*
 *    Function: restore_term_info
 *    Description: save the terminal information for the current terminal.
 *    inputs: new_term - the terminal id to switch to
 *    outputs: none
 *	  return value: 0 for success
 *    effects: restore the new terminal info
 */
int32_t restore_term_info(uint32_t new_term){
	screen_x = term[new_term].cursor_x;
	screen_y = term[new_term].cursor_y;
	current_location = term[new_term].cursor_location;
	update_cursor(current_location/2);
	length_key = term[new_term].len_key_buf;
	memcpy((uint8_t*) VIDEO, term[new_term].vid_backup, SCREEN_SIZE);
	memcpy((uint8_t*) keyboard_buffer, (uint8_t*) term[new_term].term_key_buf, KEYBOARD_BUFFER_SIZE);
	length_key = term[new_term].len_key_buf;
	return 0;
}



