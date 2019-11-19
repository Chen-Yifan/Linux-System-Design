#include "syscall_handler.h"
#include "file_system.h"
#include "global.h"
#include "terminal.h"
#include "pit.h"

//initialize the global variables
uint8_t pid_array [MAX_PROCESSES] = {0,0,0,0,0,0};
op_table_t rtc_table = {rtc_read, rtc_write, rtc_open, rtc_close};
op_table_t dir_table = {dir_read, dir_write, dir_open, dir_close};
op_table_t file_table = {file_read, file_write, file_open, file_close};
op_table_t stdin_table = {terminal_read, no_write, no_open, no_close};
op_table_t stdout_table = {no_read, terminal_write, no_open, no_close};

uint8_t cur_pid = 0;
/* 
*	Function execute()
*	Description: This function executes the specified program, in the sequence as follows:
*		1. Parse
*		2. Executable check
*		3. Paging
*		4. User-level Program Loader
*		5. Create PCB
*		6. Context switch		
*	input: pointer to command to execute (for example: "shell")
*	output: an integer returning the status of the function:
			-1  : cannot be executed
			256 : program dies by an exception
			0 - 255 : the program executes a halt system call
*	effect:
*/
int32_t execute_func(const uint8_t* command){
	uint32_t entry_point;
	int length_cmd;
	int8_t parsed_command[MAX_PARSED];
	int8_t argument[MAX_ARG];
	uint8_t buf[4]; // check for the executable
	dentry_t execute_dentry;  //executable files
	
	/* 1. parse the commands */
	int i = 0;

	cli();
	while(command[i] == ' ') {
		i++;
	}
	int start_point = i;
	length_cmd = strlen((int8_t*)command);
	for(;i < length_cmd; i++) {
		if(command[i] == ' ' || command[i] == '\0'){
			break;
		}
		if(i - start_point > MAX_PARSED-1){
			sti();
			return -1;
		}
		parsed_command[i-start_point] = command[i];
	}
	/* set the start_point points to the second part of command */
	start_point = i+1;
	while (command[start_point]==' ') {
		start_point++;
	}
	
	for(;i < MAX_PARSED; i++) {
		parsed_command [i] = '\0';
	}
	
	/* check the length of the second part of command */
	int end_point = start_point;
	while (command[end_point]!=' ' && command[end_point]!='\0') {
		end_point++;
	}
	
	for (i=start_point;i<end_point;i++) {
		argument[i-start_point] = (int8_t)command[i];
	}
	argument[end_point-start_point] = '\0';
	

	/* 2. executable check */
	if(0 != read_dentry_by_name((uint8_t*)parsed_command,&execute_dentry)){
		sti();
		return -1;
	}

	// try to read the first 4 bytes
	if(-1 == read_data(execute_dentry.inode,0, buf,4)){
		sti();
		return -1;
	}
	// check for the first four magic numbers
	if(buf[0]!=firstB_in_file || buf[1]!=secondB_in_file || buf[2]!=thirdB_in_file || buf[3]!=fourthB_in_file){ //the first four numbers
		sti();
		return -1; 
	}
	// read in the 24-27 bytes in the executable file to the entry_point
	read_data(execute_dentry.inode,(uint32_t)ENTRY_POINT_START, buf,4); //start from 24 in file
	entry_point = *((uint32_t*)buf);
	
	/* 3. set up paging, input virtual mem 128MB */
	int8_t new_pid = get_available_pid();
	if(new_pid < 0){
		sti();
	    return -2;
	}
	cur_pid = new_pid;
	remap(_128MB , _8MB + new_pid * _4MB); //create new page to map virtual mem to physical mem (paging), and enable paging
	/* 4. user-level program loader, read program image to virtual memory, as well as in physical memory */
	i = read_data(execute_dentry.inode, 0, (uint8_t*)LOAD_START,f_size); //0x08048000, virtual memory

	/*5. create PCB */
	pcb_t * new_pcb = get_specific_pcb(new_pid);
	strcpy((int8_t*)new_pcb->arg,argument);
	
	asm volatile(
		"movl %%ebp, %%eax;"
		"movl %%esp, %%ebx;"
		:"=a"(new_pcb->ebp),"=b"(new_pcb->esp)
	);
	//Initialize value of the new PCB
	init_pcb(new_pcb, new_pid);

	/* 6. context switch */
	//most of the info in tss is unchanged, so we 
	tss.ss0 = KERNEL_DS;
    tss.esp0 = _8MB - _8KB * new_pid - 4; //the current process' stack base

	term[curr_term].ss0 = tss.ss0;
	term[curr_term].esp0 = tss.esp0;
	new_pcb->ss0 = tss.ss0;
	new_pcb->esp0 = tss.esp0;

    /*Increment the running process number for current terminal */
	running_term = curr_term;
	term[curr_term].running_pid = new_pid;

	sti();
    /* Artificial iret */
    asm volatile(
    	"cli;"
    	// User DS
    	"mov $0x2B, %%ax;"
    	"mov %%ax, %%ds;"
    	"pushl $0x2B;"
    	// ESP
    	"movl $0x83FFFFC, %%eax;" // 0x83FFFFC = 128MB + 4MB - 4, everytime we remap, the virtual memory location keeps the same
    	"pushl %%eax;"
    	// EFLAG
    	"pushfl;"
		"popl %%edx;"
		"orl $0x200,%%edx;"
		"pushl %%edx;"
    	// CS
    	"pushl $0x23;"
    	// EIP
    	"pushl %0;" //eip
    	"iret;"
    	"RET_FROM_IRET:;"
    	"leave;"
    	"ret;"
    	: // no outputs
    	:"r"(entry_point) // input
    	:"%edx","%eax" 
    );
    return 0;
}


/* 
*	Function halt_func()
*	Description: terminates a process, returning the specified value to its parent process
*	input: 	status -- the value to return to its parent process
*	output: returns status
*	effect: terminates the current process
*/
int32_t halt_func(uint8_t status) { //halt term[curr_term].running_pid
	int i;
	cli();
	pcb_t* cur_pcb;
	if(status == 1) //halt from ctrl+C
		/* find the current and parent pcb address */
		cur_pcb = get_specific_pcb(term[curr_term].running_pid);
	else //normal halt in scheduling
		cur_pcb = get_specific_pcb(cur_pid); //from running term

	uint32_t halt_term; //the term in which process is gonna halt
	if(status == 1) {
		halt_term = curr_term;
		if (running_term != halt_term){
			pcb_t* running_pcb = get_specific_pcb((uint8_t)term[running_term].running_pid);
			asm volatile(
			"movl %%ebp, %%eax;"
			"movl %%esp, %%ebx;"
			:"=a"(running_pcb->curr_ebp),"=b"(running_pcb->curr_esp)
			);
			term[running_term].esp0 = tss.esp0;
			term[running_term].ss0 = tss.ss0;
			running_term = halt_term;
		}
		clear_keyboard_buffer();
	}
	else {
		halt_term = get_specific_pcb(cur_pid)->term_id;
		if (halt_term != curr_term){
			clear_keyboard_backup(halt_term);
		} else {
			clear_keyboard_buffer();
		}
	}
		
	pcb_t* parent_pcb = cur_pcb -> parent;
	// unable the current pcb and close the open files
	pid_array[cur_pcb -> pid] = 0;
	for(i=0; i < MAX_FILES; i++) {
		if(cur_pcb -> fd_table[i].flags == 1)
			close(i);
	}
	if(cur_pcb-> parent == NULL) { //this terminal(curr_term in display)
		term[halt_term].running_pid = -1;
		execute((uint8_t*)"shell");
	} 

	// Change the pid of running process in current terminal 
	term[halt_term].running_pid = parent_pcb->pid;

	// restore paging
	remap(_128MB , _8MB + parent_pcb->pid * _4MB);
	tss.esp0 = (uint32_t)cur_pcb->esp;
	cur_pid = parent_pcb->pid;

	sti();
    /* Return from iret */
	asm volatile(

		"mov %0, %%eax;"
		"mov %1, %%esp;"
		"mov %2, %%ebp;"
		"jmp RET_FROM_IRET;"
		: // no output
		:"r"((uint32_t)status), "r"(cur_pcb->esp),"r"(cur_pcb->ebp) 
		:"%eax"
		);
	return 0;
}



/* 
*	Function get_available_pid()
*	Description: gets the next available process number using the process id array
*	input: none
*	output: returns the next available process number upon success, -1 upon failure
*	effect: none
*/
int8_t get_available_pid(){
	/* Determine the next available process number */
    int32_t i;
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (pid_array[i] == 0) {
        	pid_array[i] = 1;
	    	return i;
        }
    }
    /* return -1 if no more processes available */
    printf("Too many processes running.\n");
    return -1;
}

/* 
*	Function init_pid()
*	Description: initialize pcb, including parent, pid and fd_table
*	input: new pcb added to kernel
*	output: none
*	effect: First process has no parent
*/
void init_pcb(pcb_t * pcb, uint8_t pid){ //new_pcb, new_pid
	if (get_parent_pcb(pid) == NULL){ //the first process in that terminal, there is no parent
		pcb->parent = NULL;
		pcb->pid = pid;
	}
	else{
		pcb->parent = get_parent_pcb(pid);
		pcb->pid = pid;
	}
	pcb->term_id = curr_term;
	pcb->fd_table[0].op_table_ptr = stdin_table;
	pcb->fd_table[1].op_table_ptr = stdout_table;
	pcb->fd_table[0].flags = 1;
	pcb->fd_table[1].flags = 1;
}

/* 
*	Function get_specific_pcb (uint8_t pid)
*	Description: get the pointer to pcb of the input process
*   Input:  process---the index of the process
*   Output: return the pointer to the pcb
 */
pcb_t* get_specific_pcb(uint8_t pid){
	uint32_t addr;
	addr = _8MB - _8KB * (1+pid);
	return (pcb_t*)addr;
}

/* 
*	Function get_parent_pcb (uint8_t pid)
*	Description: get the pointer to the parent pcb of the input process
*   Input:  process---the index of the process
*   Output: return the pointer to the parent pcb
 */
pcb_t* get_parent_pcb(uint8_t pid){
	if (term[curr_term].running_pid != -1) {
		return get_specific_pcb(term[curr_term].running_pid);
	}
	return NULL;
		
}

/* int32_t read_func(): read function in the system call
 * Input:  fd-----the index of the file_descriptor
 *		   buf----the buffer needed to fill with the data
 *		   nbytes-the number of bytes need to read
 * Output: return the number of bytes finally read
 */
int32_t read_func(int32_t fd, void* buf, int32_t nbytes){
	/* check the non-valid inputs */
	if (fd<0 || fd>MAX_FILES-1 || buf==NULL){
		return -1;
	}
	
	/* get the current pcb */
	pcb_t* pcb = get_specific_pcb(cur_pid);
	/* check the state of the file */
	if (pcb->fd_table[fd].flags == 0){
		return -1;		/* the file is not in use */
	}
	
	/* return the read function */
	return pcb->fd_table[fd].op_table_ptr.read(fd,buf,nbytes);
}

/* int32_t write_func(): write function in the system call
 * Input:  fd-----the index of the file_descriptor
 *		   buf----the buffer needed to fill with the data
 *		   nbytes-the number of bytes need to write
 * Output: return the number of bytes finally write
 */
int32_t write_func(int32_t fd, const void* buf, int32_t nbytes){
	cli();
	/* check the nonvalid inputs */
	if (fd<0 || fd>MAX_FILES-1 || buf==NULL){
		return -1;
	}
	
	/* get the current pcb */
	pcb_t* pcb = get_specific_pcb(cur_pid);
	/* check the state of the file */
	if (pcb->fd_table[fd].flags == 0){
		return -1;		/* the file is not in use */
	}
	int i;
	i = pcb->fd_table[fd].op_table_ptr.write(fd,buf,nbytes);
	/* return the write function */
	sti();
	return i;
}

/* int32_t open_func(): open function in the system call
 * Input:  filename-----the name of the file to open
 * Output: if success return the index in fd_table, otherwise return -1
 */
int32_t open_func(const uint8_t* filename){
	pcb_t* pcb = get_specific_pcb(cur_pid); 
	dentry_t file_dentry;
	uint32_t fd,file_type;
	
	/*check filename */ 
	if(filename == NULL){
		return -1;
	}

	/* check for stdin */
	if (strncmp((const int8_t*)filename, (const int8_t*)"stdin",5) == 0){
		pcb->fd_table[0].op_table_ptr = stdin_table;
		pcb->fd_table[0].flags = 1;
		return 0;
	}
	
	/* check for stdout */
	if (strncmp((const int8_t*)filename, (const int8_t*)"stdout",6) == 0){
		pcb->fd_table[1].op_table_ptr = stdout_table;
		pcb->fd_table[1].flags = 1;
		return 0;
	}
	
	/* read by name */
	if (read_dentry_by_name(filename,&file_dentry) == -1){
		return -1;		// read by name fails
	}
	
	/* get a new fd==>valid from 2 to 7 */
	for (fd=2;fd<MAX_FILES;fd++){
		if (pcb->fd_table[fd].flags==0){
			pcb->fd_table[fd].flags = 1;
			pcb->fd_table[fd].file_position = 0;
			break;
		}
	}
	
	/* check fd */
	if (fd==MAX_FILES){
		return -1;
	}
	
	/* check the file type */
	file_type = file_dentry.file_type;
	if (file_type==0){			// rtc
		if (rtc_open(filename) != 0){
			return -1;			// open fails
		}
		pcb->fd_table[fd].inode = 0;					// initialize
		pcb->fd_table[fd].op_table_ptr = rtc_table;
	}
	else if (file_type==1){		// dir
		pcb->fd_table[fd].inode = 0;					// initialize
		pcb->fd_table[fd].op_table_ptr = dir_table;
	}
	else if (file_type==2){		// file
		pcb->fd_table[fd].inode = file_dentry.inode;	// initialize
		pcb->fd_table[fd].op_table_ptr = file_table;
	}
	else{
		return -1;
	}
	
	return fd;
}

/* int32_t close_func(): close function in the system call
 * Input:  fd-----the index of the file_descriptor
 * Output: if success return 0, otherwise return -1
 */
int32_t close_func(int32_t fd){
	/* check for non-valid inputs */
	if (fd<2 || fd>MAX_FILES-1){		// except stdin and stdout
		return -1;
	}
	
	/* get the current pcb */
	pcb_t* pcb = get_specific_pcb(cur_pid);
	
	/* check the state of the file */
	if (pcb->fd_table[fd].flags == 0){
		return -1;		/* the file is not in use */
	}
	
	/* set the file to be not in use */
	pcb->fd_table[fd].flags = 0;
	
	/* call close function */
	if (pcb->fd_table[fd].op_table_ptr.close(fd) != 0){
		return -1;		// close failed
	}
	
	return 0;
}

/* 
*	Function no_read()
*	Description: do nothing and return -1
*	input: 	none
*	output: -1
*	effect: none
*/
int32_t no_read(int32_t fd, void * buf, int32_t nbytes) {
	return -1;
}

/*
*	Function no_write()
*	Description: do nothing and return -1
*	input: 	none
*	output: -1
*	effect: none
*/
int32_t no_write(int32_t fd, const void * buf, int32_t nbytes) {
	return -1;
}

/*
*	Function no_open()
*	Description: do nothing and return -1
*	input: 	none
*	output: -1
*	effect: none
*/
int32_t no_open(const uint8_t * filename) {
	return -1;
}

/*
*	Function no_close()
*	Description: do nothing and return -1
*	input: 	none
*	output: -1
*	effect: none
*/
int32_t no_close(int32_t fd) {
	return -1;
}

/* int32_t getargs(): reads the program’s command line arguments into a user-level buffer
 * Input:  buf-----pointer of a buffer which we put arguments into
 *		   nbytes--number of bytes to copy
 * Output: if success return 0, otherwise return -1
 */
int32_t getargs_func(uint8_t * buf, int32_t nbytes){
	/* check the buf and nbytes */
	if (buf==NULL || nbytes==0){
		return -1;
	}
	
	/* get the current pcb */
	pcb_t* pcb = get_specific_pcb(cur_pid);
	if (strlen(pcb->arg) == 0){
		return -1;
	}
	strncpy((int8_t*)buf,pcb->arg,nbytes);
	if (strlen(pcb->arg) == 0){
		return -1;
	}
	return 0;
}

/* int32_t vidmap(): maps the text-mode video memory into user space at a pre-set virtual address
 * Input:  screen_start---pointer to start the video memory
 * Output: if success return 0, otherwise return -1
 */
int32_t vidmap_func(uint8_t ** screen_start){
	/* invalid address */
	if ((uint32_t)screen_start == NULL || (uint32_t)screen_start ==_4MB) {
		return -1;
	}
	
	/* map the memory */
	*screen_start = (uint8_t*)_132MB;// not sure, decide by ourselves

	remap_vid(_132MB, VIDEO); //0xB8000

	return 0;
}
int32_t set_handler_func(int32_t signum, void * handler_address){
	return 0;
}
int32_t sigreturn_func(void){
	return 0;
}


