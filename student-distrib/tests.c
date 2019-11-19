#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "interrupt_handler.h"
#include "paging.h"
#include "terminal.h"
#include "keyboard.h"
#include "rtc_handler.h"
#include "file_system.h"
#define PASS 1
#define FAIL 0



/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Check the value stored in IDT
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;
	int i;
	int result = PASS;
	// check the exception IDT values (00 - 19 without 15)
	for (i = 0; i < 19; ++i){
        if(i == 15)
            continue;
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
		// print out our changing parts
		printf("IDT %d: offset_15_00- %u, offset_31_16 - %u\n",i,idt[i].offset_15_00, idt[i].offset_31_16);
	}
	// check keyboard interrupt IDT value (0x21)
	i = 0x21;
	if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
	}
	printf("IDT %d: offset_15_00 - %u, offset_31_16- %u\n",i,idt[i].offset_15_00, idt[i].offset_31_16);

	// check RTC interrupt IDT value (0x28)
	i = 0x28;
	if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
	}
	printf("IDT %d: offset_15_00 - %u, offset_31_16 - %u\n",i,idt[i].offset_15_00, idt[i].offset_31_16);
	// check the system call IDT values (0x80)
	i = 0x80;
	if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
	}
	printf("1");
	printf("IDT %d: offset_15_00 - %u, offset_31_16 - %u\n",i,idt[i].offset_15_00, idt[i].offset_31_16);
	return result;
}

/* Test Values contained in our paging structures 
 * Dereferencing different address ranges with paging turned on.
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: trigger an RTC interrupt
 * Files: x86_desc.h/S, lib.c/h
 */
int paging_valid_test(){
	TEST_HEADER;
	int result = PASS;
    // video memory
    if(page_table_array[0].page_table[184].p != 1) {
        assertion_failure();
        result = FAIL;
    }
    // first directory
    else if(page_directory_array[0].page_directory[0].kb.p != 1) {
        assertion_failure();
        result = FAIL;
    }
    // check base address of the page table, should not be empty
    else if(page_directory_array[0].page_directory[0].kb.page_table_base_addr == NULL) {
        assertion_failure();
        result = FAIL;
    }
    // second directory - kernel
    else if(page_directory_array[0].page_directory[1].mb.p != 1) {
        assertion_failure();
        result = FAIL;
    }
    // check for start point of kernel should be 0x400000
    else if(page_directory_array[0].page_directory[1].mb.page_base_addr != 1) {
        assertion_failure();
        result = FAIL;
    }
	return result;
}

/*
 * Dereferencing different address ranges with paging turned on.
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: trigger an RTC interrupt
 * Files: x86_desc.h/S, lib.c/h
 */
int paging_dereference_test(){
    TEST_HEADER;
    int result = PASS;
    // dereference test
    // test for video memory
	printf("test for video memory 0xB8000\n"); 
    uint32_t* a = (uint32_t*)0xB8000;
    uint32_t b = *a;
    // test for kenerel memory
    // start of the kernel addr
	printf("test for the start of kernel addr 0x400000\n"); 
    a = (uint32_t*)0x400000;
    b = *a;
	// a random middle location in the kernel
	printf("test for the start of kernel addr 0x400BFB\n"); 
    a = (uint32_t*)0x400BFB;
    b = *a;
	
    // end of the kernel addr
	printf("test for end of kernel addr 0x7FFFFC\n"); 
    a = (uint32_t*)0x7FFFFC;
    b = *a;
	
    // not present, should threw page fault exception
	//printf("test for unvalid page location 0x800000\n"); 
    a = (uint32_t*)0x800000;
    b = *a;
    return result;
}




/* Checkpoint 2 tests */
/* test for the terminal open, read, write
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal/ keyboard
 * Files: x86_desc.h/S, lib.c/h, keyboard.h/c, terminal.h/c
 */
int terminal_test(){
	TEST_HEADER;
	int result = PASS;
	terminal_open(0);
	int i;
	for(i=0 ; i < 3; i++) {
		char buf[128] = {0}; // set up a new buffer
		printf("test for written in different length of strings:\n");
		//while(keyboard_buffer[length_key-1] != '\n') {}
		//printf("write in the string for length %d:\n", length_key);
		//printf("print out the contents in keyboard buffer:\n");
		// read_buffer();
		printf("calling terminal read and write:\n");
		printf("read bytes: %d\n", terminal_read(0,buf,128));
		printf("print out the contents in terminal buffer\n");
		printf("write bytes: %d\n",terminal_write(0,buf,128));
	}
	return result;	
}


/* Test the initialization of file system 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: filesystem
 * Files: x86_desc.h/S, lib.c/h
 */
int file_system_initial_test(){
	TEST_HEADER;
	int result = PASS;
	
	//check the input value
	printf("test for bootBlock\n");
	if(bootBlock == NULL) {
        assertion_failure();
        result = FAIL;
    }
	else if(dir_number!=0){
		assertion_failure();
        result = FAIL;
	}
	return result;
}

/* Test the dir_read of file system 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: filesystem
 * Files: x86_desc.h/S, lib.c/h
 */
int file_system_dir_read_test(){
	TEST_HEADER;
	int result = PASS;
	
	//check the input value
	printf("test for dir_read()\n");
	
	int32_t fd, cnt;
    uint8_t buf[33];

    if (-1 == (fd = dir_open ((uint8_t*)"."))) {
        printf("directory open failed\n");
        assertion_failure();
        result = FAIL;
    }

    while (0 != (cnt = dir_read (fd, buf, 32))) {
        if (-1 == cnt) {
	        printf("directory entry read failed\n");
	        assertion_failure();
        	result = FAIL;
	    }
		printf("file name: %s  file type: %u  file size: %u\n", buf, f_type, f_size);
		
    }
	
	return result;
}


/* Test the file_read of file system 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: filesystem
 * Files: x86_desc.h/S, lib.c/h
 */
int file_system_file_read_test1(){
	TEST_HEADER;
	int result = PASS;
	int32_t fd, cnt;
	int i;
	
	//check the input value
	printf("test for file_read() for txt file\n");

    if (-1 == (fd = file_open ((uint8_t*)"frame0.txt"))) {
        printf("file open failed\n");
        assertion_failure();
        result = FAIL;
    }
	uint8_t buf[f_size];
    cnt = file_read (fd, buf, f_size);
    if (-1 == cnt) {
	    printf("file entry read failed\n");
	    assertion_failure();
     	result = FAIL;
	}

	for (i=0;i<cnt;i++){
		putc(buf[i]);
	}
		
	printf("\nfile name: frame0.txt\n");
	
	return result;
}

/* Test the file_read of file system 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: filesystem
 * Files: x86_desc.h/S, lib.c/h
 */
int file_system_file_read_test2(){
	TEST_HEADER;
	int result = PASS;
	int i;
	
	//check the input value
	printf("test for file_read() for non-txt file\n");
	
	int32_t fd, cnt;

    if (-1 == (fd = file_open ((uint8_t*)"ls"))) {
        printf("file open failed\n");
        assertion_failure();
        result = FAIL;
    }
	uint8_t buf[f_size];
    cnt = file_read (fd, buf, f_size);
    if (-1 == cnt) {
	    printf("file entry read failed\n");
	    assertion_failure();
     	result = FAIL;
	}

	for (i=0;i<cnt;i++){
		putc(buf[i]);
	}
		
	printf("\nfile name: ls\n");
	return result;
}

/* Test the file_read of file system 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: filesystem
 * Files: x86_desc.h/S, lib.c/h
 */
int file_system_file_read_test3(){
	TEST_HEADER;
	int result = PASS;
	int i;
	
	//check the input value
	printf("test for file_read() for large file\n");
	
	int32_t fd, cnt;

    if (-1 == (fd = file_open ((uint8_t*)"verylargetextwithverylongname.tx"))) {
        printf("file open failed\n");
        assertion_failure();
        result = FAIL;
    }
	uint8_t buf[f_size];
    cnt = file_read (fd, buf, f_size);
    if (-1 == cnt) {
	    printf("file entry read failed\n");
	    assertion_failure();
     	result = FAIL;
	}

	for (i=0;i<cnt;i++){
		putc(buf[i]);
	}
		
	printf("\nfile name: verylargetextwithverylongname.tx\n");
	return result;
}

/* Test the file_read of file system 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: filesystem
 * Files: x86_desc.h/S, lib.c/h
 */
int file_system_file_read_file_by_file_test(){
	TEST_HEADER;
	int result = PASS;
	int i,c;
	
	//check the input value
	printf("test for file_read() read file by file\n");

	int32_t fd1,fd2, cnt1,cnt2;
    uint8_t buf1[33];

    if (-1 == (fd1 = dir_open ((uint8_t*)"."))) {
        printf("directory open failed\n");
        assertion_failure();
        result = FAIL;
    }

	c = 0;
    while (0 != (cnt1 = dir_read (fd1, buf1, 32))) {
        if (-1 == cnt1) {
	        printf("directory entry read failed\n");
	        assertion_failure();
        	result = FAIL;
	    }
		
		if (c == 0){
			c = 1;
			continue;
		}
		
		if (-1 == (fd2 = file_open ((uint8_t*)buf1))) {
        	printf("file open failed\n");
        	assertion_failure();
       		result = FAIL;
    	}
		uint8_t buf2[f_size];
	    cnt2 = file_read (fd2, buf2, f_size);
	    if (-1 == cnt2) {
		    printf("file entry read failed\n");
		    assertion_failure();
	     	result = FAIL;
		}
		
		for (i=0;i<cnt2;i++){
			
			putc(buf2[i]);
		}
		printf("\n");
		printf("file name: %s\n", buf1);
		
    }
		
	return result;
}

/* Test the file_write and dir_write of file system 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: filesystem
 * Files: x86_desc.h/S, lib.c/h
 */
int file_system_write_test(){
	TEST_HEADER;
	int result = PASS;
	
	//check the input value
	printf("test for file_write() and dir_write()\n");

	int32_t fd, cnt;
    uint8_t buf[33];

	
	if (-1 != (cnt = dir_write(fd, buf, 32))){
		printf("directory entry write test failed\n");
	    assertion_failure();
        result = FAIL;
	}
	
	cnt = file_write (fd, buf, 32);
	if (-1 != cnt) {
		printf("file entry write test failed\n");
		assertion_failure();
	    result = FAIL;
	}
	
		
	return result;
}

/* Test the nonvalid input and NULL pointer of file system 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: filesystem
 * Files: x86_desc.h/S, lib.c/h
 */
int file_system_file_read_test4(){
	TEST_HEADER;
	int result = PASS;
	
	//check the input value
	printf("test for nonvalid input and NULL pointer\n");
	
	int32_t fd, cnt;

	printf("Try to open file: ccc\n");
    if (-1 != (fd = file_open ((uint8_t*)"ccc"))) {
        printf("test failed\n");
        assertion_failure();
        result = FAIL;
    }
	else{
		printf("file open failed, can't find this file\n");
	}
	
	printf("Try to pass a NULL pointer\n");
    cnt = file_read (fd, NULL, f_size);
    if (-1 != cnt) {
	    printf("test failed\n");
	    assertion_failure();
     	result = FAIL;
	}
	else{
		printf("file read failed since buffer is a NULL pointer\n");
	}
	
		
	return result;
}

/* Test the nonvalid input and NULL pointer of file system 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: filesystem
 * Files: x86_desc.h/S, lib.c/h
 */
int rtc_test(){
	TEST_HEADER;
	int result = PASS;
	int i, k;
	int32_t* freq;
	//check the input value
	//clear(); //clear the screen for testing
	rtc_open(NULL); //Call rtc open. set rtc to 2hz
	//freq from low to high
	int32_t freq_set[4]= {2,4,8,16};

	for(k=0; k<4; k++){
		freq = freq_set + k;
		printf("Frequency is %d\n",*freq);
		rtc_write(0,freq,0); //write rtc freq
		for(i = 0; i<20; i++)	//wait until rtc_read respond for having rtc interrupt
		{
			rtc_read(0,freq,0);
			printf("1"); //RTC read successful. Print hi for showing freq rate
		}
		printf("\n");
	}

	rtc_close(0);
	return result;
}

/* Checkpoint 3 tests */

/*  Running the shell and testprint programs
 * Halting the shell and testprint programs
 * Inputs: run the user programs and may display something on the screen, printing out
 * some snetences in the middle to keep track of the execution system call
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: system call
 * Files: system_calls.h, system_call_handler.S/h, lib.c/h
 */
int shell_test(){
	TEST_HEADER;
	int result = PASS;
	printf("executing shell program:\n");
	execute((uint8_t *)"shell");
	return result;
}

/* Using the read/write system calls to read/write to the terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: output something on the terminal using stdin & stdout
 * Coverage: system call, terminal
 * Files: system_calls.h, system_call_handler.S/h, lib.c/h, terminal.h
 */
int terminal_read_write_test(){
	TEST_HEADER;
	int result = PASS;
	int32_t a;
	uint8_t buf[128];
	printf("executing reading to terminal:\n");
	//execute((uint8_t *)"shell");
	open((uint8_t*)"stdin");
	a = read(0,buf,128);
	printf("reading # of bytes: %d\n", a);
	printf("checking for the file descriptor flags after open:\n");
	pcb_t* cur_pcb = get_specific_pcb(cur_pid);
	for(a=0; a < 8; a++) {
		printf("%d\t", cur_pcb->fd_table[a].flags);
	}
	printf("\n");	
	close(0);
	printf("checking for the file descriptor flags after close:\n");
	for(a=0; a < 8; a++) {
		printf("%d\t", cur_pcb->fd_table[a].flags);
	}
	printf("\n");
	printf("executing writing to terminal:\n");
	open((uint8_t*)"stdout");
	a = write(1,buf,128);
	// invalid writing
	if(a== -1) {
		printf("test fails\n");
	    assertion_failure();
		result = FAIL;
	}
	printf("writing # of bytes: %d\n", a);
	printf("checking for the file descriptor flags after open:\n");
	for(a=0; a < 8; a++) {
		printf("%d\t", cur_pcb->fd_table[a].flags);
	}
	printf("\n");
	close(1);
	printf("checking for the file descriptor flags after close:\n");
	for(a=0; a < 8; a++) {
		printf("%d\t", cur_pcb->fd_table[a].flags);
	}
	printf("\n");	
	//halt();
	return result;
}


/* Test the system call open() and close()
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: none
 * Coverage: system call
 * Files: system_calls.h, system_call_handler.S/h, lib.c/h,
 */
int open_close_test(){
	TEST_HEADER;
	int result = PASS;
	
	open((uint8_t*)"stdin");
	open((uint8_t*)"stdout");
	open((uint8_t*)"hello");
	open((uint8_t*)"frame0.txt");
	pcb_t* cur_pcb = get_specific_pcb(cur_pid);
	if(cur_pcb->fd_table[0].flags != 1) {
		printf("open stdin fails\n");
	    assertion_failure();
		result = FAIL;
	}
	else{
		printf("open stdin successfully\n");
	}
	
	if(cur_pcb->fd_table[1].flags != 1) {
		printf("open stdout fails\n");
	    assertion_failure();
		result = FAIL;
	}
	else{
		printf("open stdout successfully\n");
	}
	
	if(cur_pcb->fd_table[2].flags != 1) {
		printf("open hello fails\n");
	    assertion_failure();
		result = FAIL;
	}
	else{
		printf("open hello successfully\n");
	}
	
	if(cur_pcb->fd_table[3].flags != 1) {
		printf("open frame0.txt fails\n");
	    assertion_failure();
		result = FAIL;
	}
	else{
		printf("open frame0.txt successfully\n");
	}
	
	close(0);
	close(1);
	close(2);
	close(3);
	cur_pcb = get_specific_pcb(cur_pid);
	if(cur_pcb->fd_table[0].flags != 1) {
		printf("test fails\n");
	    assertion_failure();
		result = FAIL;
	}
	else{
		printf("close stdin fails since stdin shouldn't be close\n");
	}
	
	if(cur_pcb->fd_table[1].flags != 1) {
		printf("test fails\n");
	    assertion_failure();
		result = FAIL;
	}
	else{
		printf("close stdin fails since stdout shouldn't be close\n");
	}
	
	if(cur_pcb->fd_table[2].flags == 1) {
		printf("close hello fails\n");
	    assertion_failure();
		result = FAIL;
	}
	else{
		printf("close hello successfully\n");
	}
	
	if(cur_pcb->fd_table[3].flags == 1) {
		printf("close frame0.txt fails\n");
	    assertion_failure();
		result = FAIL;
	}
	else{
		printf("close frame0.txt successfully\n");
	}
	
	
	
	return result;
}

/* Test the system call read()
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: none
 * Coverage: system call
 * Files: system_calls.h, system_call_handler.S/h, lib.c/h,
 */
int sys_read_test1(){
	TEST_HEADER;
	int result = PASS;
	int i,cnt;
	
	printf("test for read()/write() txt file\n");
	open((uint8_t*)"frame0.txt");
	uint8_t buf[f_size];
	
	read(2,buf,f_size);
	for (i=0;i<f_size;i++){
		putc(buf[i]);
	}
	
	if(write(2,buf,f_size)!=-1){
		printf("txt file write failed\n");
	    assertion_failure();
        result = FAIL;
	}
	
	printf("test for read()/write() large txt file by offset:\n");
	open((uint8_t*)"verylargetextwithverylongname.tx");
	uint8_t buf1[10];
	read(3,buf1,10);
	for (i=0;i<10;i++){
		putc(buf1[i]);
	}
	uint8_t buf2[10];
	read(3,buf2,10);
	for (i=0;i<10;i++){
		putc(buf2[i]);
	}
	
	if(write(3,buf,f_size)!=-1){
		printf("large txt file write failed\n");
	    assertion_failure();
        result = FAIL;
	}
	
	printf("\ntest for read()/write() dir\n");
	open((uint8_t*)".");
	uint8_t buf3[33];
	for (i=0;i<33;i++){
			buf3[i] = '\0';
	}
	while (0 != (cnt = read (4, buf3, 32))) {
        if (-1 == cnt) {
	        printf("directory entry read failed\n");
	        assertion_failure();
        	result = FAIL;
	    }
		printf("file name: %s\n", buf3);
		for (i=0;i<33;i++){
			buf3[i] = '\0';
		}
		
    }
	
	if(write(4,buf,f_size)!=-1){
		printf("dir write failed\n");
	    assertion_failure();
        result = FAIL;
	}
	
	close(2);
	close(3);
	//close(4);
	
	return result;
}

/* Test the system call read()/write() for rtc
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: none
 * Coverage: system call
 * Files: system_calls.h, system_call_handler.S/h, lib.c/h, rtc_handler.h
 */
int sys_rtc_test(){
	TEST_HEADER;
	int result = PASS;
	int i, k;
	int32_t* freq;
	//check the input value
	open((uint8_t*)"rtc"); //Call rtc open. set rtc to 2hz
	//freq from low to high
	int32_t freq_set[4]= {2,4,8,16};

	for(k=0; k<4; k++){
		freq = freq_set + k;
		printf("Frequency is %d\n",*freq);
		write(2,freq,0); //write rtc freq
		for(i = 0; i<20; i++)	//wait until rtc_read respond for having rtc interrupt
		{
			read(2,freq,0);
			printf("1"); //RTC read successful. Print hi for showing freq rate
		}
		printf("\n");
	}

	close(2);
	return result;
}


/* Checkpoint 4 tests */


/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("exception_test", exception_test());
	//TEST_OUTPUT("idt_test", idt_test());
    //TEST_OUTPUT("paging_valid_test", paging_valid_test());
    //TEST_OUTPUT("paging_dereference_test", paging_dereference_test());
	//TEST_OUTPUT("terminal_test", terminal_test());
	//TEST_OUTPUT("rtc_test", rtc_test());
	//TEST_OUTPUT("file_system_initial_test",file_system_initial_test());
	//TEST_OUTPUT("file_system_dir_read_test",file_system_dir_read_test());
	//TEST_OUTPUT("file_system_file_read_test1",file_system_file_read_test1());
	//TEST_OUTPUT("file_system_file_read_test2",file_system_file_read_test2());
	//TEST_OUTPUT("file_system_file_read_test3",file_system_file_read_test3());
	//TEST_OUTPUT("file_system_file_read_test4",file_system_file_read_test4());
	//TEST_OUTPUT("file_system_file_read_file_by_file_test",file_system_file_read_file_by_file_test());
	//TEST_OUTPUT("file_system_write_test",file_system_write_test());
	//TEST_OUTPUT("terminal_read_write_test",terminal_read_write_test());
	//TEST_OUTPUT("open and close test",open_close_test());
	//TEST_OUTPUT("system call read/write test for file/dir",sys_read_test1());
	//TEST_OUTPUT("system call read/write test for rtc",sys_rtc_test());
	TEST_OUTPUT("shell_test",shell_test());
    

}

