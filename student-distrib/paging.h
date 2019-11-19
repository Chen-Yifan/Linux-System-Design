/* paging.h - Defines for paging.c
 *			  used to initialize paging
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

#define NUMBER_ENTRIES 1024
#define four_KB 4096
#define four_MB 0x400000
#define NUMBER_PROCESS 4
#define shift 12
#define VIDEO_ADDR 0xB8


/* align pages (page directory and page tables) on 4 kB boundaries */
/* page directory */
/* Page directory¡ªAn array of 32-bit page-directory entries (PDEs) contained in a 4-KByte page. Up to 1024 page-directory entries can be held in a page directory */

/* new struct for page_directory_entry ==> 4-MB page */
typedef union PDE_4MB{
	uint32_t pointer;					/* the value store in this pointer */
	struct{								/* define bit by bit */
		uint32_t p : 1;
		uint32_t rw : 1;
		uint32_t us : 1;
		uint32_t pwt : 1;
		uint32_t pcd : 1;
		uint32_t a : 1;
		uint32_t d : 1;
		uint32_t ps : 1;
		uint32_t g : 1;
		uint32_t avail : 3;
		uint32_t pat : 1;
		uint32_t reserved : 9;
		uint32_t page_base_addr : 10; //leftmost 10 bits
		}__attribute__((packed));		/* aligning bits in order */
} PDE_4MB_t;


/* new struct for page_directory_entry ==> 4-KB page */
typedef union PDE_4KB{
	uint32_t pointer;					/* the value store in this pointer */
	struct{								/* define bit by bit */
		uint32_t p : 1;
		uint32_t rw : 1;
		uint32_t us : 1;
		uint32_t pwt : 1;
		uint32_t pcd : 1;
		uint32_t a : 1;
		uint32_t reserved : 1;
		uint32_t ps : 1;
		uint32_t g : 1;
		uint32_t avail : 3;
		uint32_t page_table_base_addr : 20; 
		}__attribute__((packed));		/* aligning bits in order */
}PDE_4KB_t;


/* new struct for one 4kb/4mb page_directory_entry */
typedef union PDE{
	PDE_4KB_t kb;
	PDE_4MB_t mb;
} PDE_t;

/* new struct for every 1024 entries directory */
typedef struct PD{
	PDE_t page_directory[NUMBER_ENTRIES];
} PD_t;

/* an array to store page_directory for every processor */
PD_t page_directory_array[NUMBER_PROCESS] __attribute__((aligned (four_KB)));


/* new struct for page_table_entry ==> 4-KB page */
typedef union PTE{
	uint32_t pointer;					/* the value store in this pointer */
	struct{								/* define bit by bit */
		uint32_t p : 1;
		uint32_t rw : 1;
		uint32_t us : 1;
		uint32_t pwt : 1;
		uint32_t pcd : 1;
		uint32_t a : 1;
		uint32_t d : 1;
		uint32_t pat : 1;
		uint32_t g : 1;
		uint32_t avail : 3;
		uint32_t page_base_addr : 20;
		}__attribute__((packed));		/* aligning bits in order */
} PTE_t;

typedef struct PT{
	PTE_t page_table[NUMBER_ENTRIES];
} PT_t;

/* Page table¡ªAn array of 32-bit page-table entries (PTEs) contained in a 4-KByte page. Up to 1024 page-table entries can be held in a page table. */
/* page table */
PT_t page_table_array[NUMBER_PROCESS] __attribute__((aligned (four_KB)));

/* functions */
void init_paging();

void remap(int32_t virtual_addr, int32_t physical_addr);

void remap_vid(int32_t virtual_addr, int32_t physical_addr);

void flush_TLB();

void set_up_PD_PT();

void enable_paging();

#endif /* _PAGING_H */
