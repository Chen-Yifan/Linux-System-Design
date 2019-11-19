/* file_system.h - Defines for paging.c
 *				   used to build file system
 */

#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEN_H

#include "types.h"
#include "syscall_handler.h"
#define MAX_NAME_LENGTH 32
#define FOUR_KB 4096

/* new struct to store every directory entry===>64B in total */
typedef struct dir_entry{
	char file_name[MAX_NAME_LENGTH];
	uint32_t file_type;
	uint32_t inode;
	uint32_t reserved[6];			// 24B reserved
} dentry_t;

/* new struct to store the boot block ===>4kB in total */
typedef struct boot_block{
	uint32_t num_dir_entries;
	uint32_t num_inodes;
	uint32_t num_data_blocks;
	uint32_t reserved[13];			// 52/4 = 13
	dentry_t dir_entries[63];		// 4096/64 - 1 = 63
} boot_block_t;

/* new struct to store every index nodes ===>4kB in total */
typedef struct index_node{
	uint32_t length;
	uint32_t data_block[1023];		// (4KB/4B) - 1 = 1023
} index_node_t;

extern int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
extern int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
extern int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
extern void init_file_system(uint32_t bootBlock_addr);

extern int32_t file_open (const uint8_t* filename);
extern int32_t file_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t file_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t file_close (int32_t fd);

extern int32_t dir_open (const uint8_t* filename);
extern int32_t dir_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t dir_close (int32_t fd);


boot_block_t* bootBlock;
uint32_t dir_number;
uint32_t inode_number;
uint32_t f_type;
uint32_t f_size;

#endif /* _FILE_SYSTEM_H */
