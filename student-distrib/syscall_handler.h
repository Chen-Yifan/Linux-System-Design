#ifndef SYSCALL_HANDLER_H
#define SYSCALL_HANDLER_H
#include "x86_desc.h"
#include "rtc_handler.h"
#include "paging.h"
#include "syscall.h"
#include "lib.h"

#define MAX_FILES 8
#define MAX_PROCESSES 6
#define MAX_PARSED 10

#define LAST_TWO_B_USER_CS 0x23
#define LAST_TWO_B_USER_DS 0x2B
#define _8MB 0x800000
#define _4MB 0x400000
#define _8KB 0x8000
#define _128MB 0x8000000 
#define _132MB 0x8400000
#define KERNEL_CS 0x0010
#define KERNEL_DS 0x0018
#define ENTRY_POINT_START 24
#define LOAD_START 0x08048000
#define firstB_in_file 0x7f
#define secondB_in_file 0x45
#define thirdB_in_file   0x4c
#define fourthB_in_file 0x46
#define MAX_ARG 1024

/* new struct to store the operation table for fd */
typedef struct op_table{
	 int32_t (*read) (int32_t fd, void* buf, int32_t nbytes);
	 int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);
	 int32_t (*open) (const uint8_t* filename);
	 int32_t (*close) (int32_t fd); 
} op_table_t;

/* new struct to store every file descriptor */
typedef struct file_desc{
	op_table_t op_table_ptr;
	uint32_t inode;
	uint32_t file_position;
	uint32_t flags;		// 1->in use,0->not in use
} file_desc_t;

/* new struct to store every pcb */
typedef struct pcb{
	file_desc_t fd_table[MAX_FILES];	// file descriptor array
	uint8_t pid;				// unique identifier for the process: One bit of per-task state that needs to be saved is the file array
	//parent_pid = pid - 1
	struct pcb * parent;
	uint32_t term_id;
	uint32_t esp;
	uint32_t ebp;
	uint32_t curr_esp;
	uint32_t curr_ebp;
	int8_t arg[MAX_ARG];
	uint16_t ss0;
	uint32_t esp0;

} pcb_t;


extern uint8_t cur_pid;

//helper functions of pcb
void init_pcb(pcb_t * pcb, uint8_t pid);
int8_t get_available_pid(); //by cyf
pcb_t* get_parent_pcb(uint8_t pid);
pcb_t* get_specific_pcb(uint8_t pid);


//global variables
extern uint8_t pid_array [MAX_PROCESSES]; //0 means not process yet, 1 means there is a process
//extern pcb_t * curr_pcb; //the pointer to the current program's pcb

extern int32_t halt_func(uint8_t status);
extern int32_t execute_func(const uint8_t * command);
extern int32_t read_func(int32_t fd, void * buf, int32_t nbytes);
extern int32_t write_func(int32_t fd, const void * buf, int32_t nbytes);
extern int32_t open_func(const uint8_t * filename);
extern int32_t close_func(int32_t fd);
extern int32_t getargs_func(uint8_t * buf, int32_t nbytes);
extern int32_t vidmap_func(uint8_t ** screen_start);
extern int32_t set_handler_func(int32_t signum, void * handler_address);
extern int32_t sigreturn_func(void);

int32_t no_read(int32_t fd, void * buf, int32_t nbytes);
int32_t no_write(int32_t fd, const void * buf, int32_t nbytes);
int32_t no_open(const uint8_t * filename);
int32_t no_close(int32_t fd);

#endif

