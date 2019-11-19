/* paging.c - initialize paging */

#include "paging.h"

uint32_t page_dir_addr; /* Global variable to refer to the new page directory address */

/* void init_paging()
 * Inputs: None
 * Return Value: None
 * Function: should be called in kernel.c to initialize paging
 *			 initialize the page directory and the page table with valid page directory entries (PDEs) and page table entries (PTEs)
 *			 map virtual memory to physical memory for kernel (4MB chunk), and video memory (4KB page)
 *			 set other memory as "not present" 
 */
void init_paging() {
	set_up_PD_PT();
	enable_paging();
	return;
}

/* void remap() 4MB
 * Inputs: virtual_addr - the virtual address of the new task
 *			physical_addr - the physical address to map to
 * Return Value: None
 * Function: map the new program's virtual address to a physical address.
 *			According to the document, the first program will use physical address 8MB and the second will use 12MB
 */
void remap(int32_t virtual_addr, int32_t physical_addr) {
	int32_t pde = virtual_addr / four_MB;
	
	/* Set up the 4MB page directory entry for program */
	page_directory_array[0].page_directory[pde].mb.p = 1;			/* set present */
	page_directory_array[0].page_directory[pde].mb.rw = 1;		/* read or write */
	page_directory_array[0].page_directory[pde].mb.us = 1;		/* assign the user privilege level */
	page_directory_array[0].page_directory[pde].mb.pwt = 0;		/* write-back caching is enabled for the associated page or page table */
	page_directory_array[0].page_directory[pde].mb.pcd = 0;		/* the page or page table can be cached */
	page_directory_array[0].page_directory[pde].mb.a = 0;			/* a page or page table is initially loaded into physical memory */
	page_directory_array[0].page_directory[pde].mb.d = 0;			/* when a page is initially loaded into physical memory */
	page_directory_array[0].page_directory[pde].mb.ps = 1;		/* 1 indicates 4MB */
	page_directory_array[0].page_directory[pde].mb.g = 0;			/* not global */
	page_directory_array[0].page_directory[pde].mb.avail = 0;		/* initialize */
	page_directory_array[0].page_directory[pde].mb.pat = 0;		/* no processor now, so reset to 0 */
	page_directory_array[0].page_directory[pde].mb.reserved = 0;	/* For a page-directory entry for a 4-MByte page, bits 12 through 21 are reserved and must be set to 0. */
	page_directory_array[0].page_directory[pde].mb.page_base_addr = physical_addr >> 22;	/* get the address for index===>0x400000 * (0 + 1)  32-22bit equals to (0 + 1) */
	
	flush_TLB();
	return;
}

/* void remap_vid() 
 * Inputs: virtual_addr - the virtual address of the new task
 *			physical_addr - the physical address to map to
 * Return Value: None
 * Function: Map the given virtual address to video memory (4KB page)
 * video memory maps to kernel also 132MB
 */
void remap_vid(int32_t virtual_addr, int32_t physical_addr) {
	int32_t pde = virtual_addr / four_MB; //for video memory
	
	page_directory_array[0].page_directory[pde].kb.p = 1;			/* set present */
	page_directory_array[0].page_directory[pde].kb.rw = 1;		/* read or write */
	page_directory_array[0].page_directory[pde].kb.us = 1;		/* assign the user privilege level */
	page_directory_array[0].page_directory[pde].kb.pwt = 0;		/* write-back caching is enabled for the associated page or page table */
	page_directory_array[0].page_directory[pde].kb.pcd = 0;		/* the page or page table can be cached */
	page_directory_array[0].page_directory[pde].kb.a = 0;			/* a page or page table is initially loaded into physical memory */
	page_directory_array[0].page_directory[pde].kb.reserved = 0;	/* set to 0 */
	page_directory_array[0].page_directory[pde].kb.ps = 0;		/* 0 indicates 4KB */
	page_directory_array[0].page_directory[pde].kb.g = 0;			/* not global now */
	page_directory_array[0].page_directory[pde].kb.avail = 0;		/* initialize */
	page_directory_array[0].page_directory[pde].kb.page_table_base_addr = ((uint32_t)page_table_array[0].page_table >> shift);	/* get the first page_base_address in table 12*/
	//map video memory
	page_table_array[0].page_table[0].rw = 1;				/* read or write */
	page_table_array[0].page_table[0].us = 1;				/* assign the supervisor privilege level */
	page_table_array[0].page_table[0].p = 1;
	page_table_array[0].page_table[0].page_base_addr = physical_addr>>shift; // the first page_table, 0xB8 entry --table
	flush_TLB();
	return;
}

/* void set_up_PD_PT()
 * Inputs: None
 * Return Value: None
 * Function: initialize the page directory and the page table with valid page directory entries (PDEs) and page table entries (PTEs)
 *			 map virtual memory to physical memory for kernel (4MB chunk), and video memory (4KB page)
 *			 set other memory as "not present" 
 */
void set_up_PD_PT() {
	int i;
	for(i = 0; i < NUMBER_ENTRIES; i++){
		/* initialize page_table_array */
		if (i >= VIDEO_ADDR && i <= VIDEO_ADDR + 3){			/* We need four 4KB pages for Video Memory and the video memory backups for 3 terminals */
			page_table_array[0].page_table[i].p = 1;			/* set page_table_entry to be not present except video memory */
		}
		else{
			page_table_array[0].page_table[i].p = 0;
		}
		page_table_array[0].page_table[i].rw = 1;				/* read or write */
		page_table_array[0].page_table[i].us = 0;				/* assign the supervisor privilege level */
		page_table_array[0].page_table[i].pwt = 0;				/*  write-back caching is enabled for the associated page or page table */
		page_table_array[0].page_table[i].pcd = 0;				/*  the page or page table can be cached */
		page_table_array[0].page_table[i].a = 0;				/* a page or page table is initially loaded into physical memory */
		page_table_array[0].page_table[i].d = 0;				/* when a page is initially loaded into physical memory */
		page_table_array[0].page_table[i].pat = 0;				/*  no processor now, so reset to 0 */
		page_table_array[0].page_table[i].g = 0;				/* not global now */
		page_table_array[0].page_table[i].avail = 0;			/* initialize */
		page_table_array[0].page_table[i].page_base_addr = i;	/* store the addr by index */
	}
	
	/* initialize page_directory_array===>the first processor */
	
	/* first initialize the first directory===>4KB */
	page_directory_array[0].page_directory[0].kb.p = 1;			/* set present */
	page_directory_array[0].page_directory[0].kb.rw = 1;		/* read or write */
	page_directory_array[0].page_directory[0].kb.us = 0;		/* assign the supervisor privilege level */
	page_directory_array[0].page_directory[0].kb.pwt = 0;		/* write-back caching is enabled for the associated page or page table */
	page_directory_array[0].page_directory[0].kb.pcd = 0;		/* the page or page table can be cached */
	page_directory_array[0].page_directory[0].kb.a = 0;			/* a page or page table is initially loaded into physical memory */
	page_directory_array[0].page_directory[0].kb.reserved = 0;	/* set to 0 */
	page_directory_array[0].page_directory[0].kb.ps = 0;		/* 0 indicates 4KB */
	page_directory_array[0].page_directory[0].kb.g = 0;			/* not global now */
	page_directory_array[0].page_directory[0].kb.avail = 0;		/* initialize */
	page_directory_array[0].page_directory[0].kb.page_table_base_addr = ((uint32_t)page_table_array[0].page_table >> shift);	/* get the first page_base_address in table */
	
	/* next initialize the second directory===>4MB */
	//page_directory_array[0].page_directory[1].mb.pointer = 0x00400003;		/* 0000 0000 01|00 0000 000|0| 000|0| 0|0|0|0| 0|0|1|1 */
	page_directory_array[0].page_directory[1].mb.p = 1;			/* set present */
	page_directory_array[0].page_directory[1].mb.rw = 1;		/* read or write */
	page_directory_array[0].page_directory[1].mb.us = 0;		/* assign the supervisor privilege level */
	page_directory_array[0].page_directory[1].mb.pwt = 0;		/* write-back caching is enabled for the associated page or page table */
	page_directory_array[0].page_directory[1].mb.pcd = 0;		/* the page or page table can be cached */
	page_directory_array[0].page_directory[1].mb.a = 0;			/* a page or page table is initially loaded into physical memory */
	page_directory_array[0].page_directory[1].mb.d = 0;			/* when a page is initially loaded into physical memory */
	page_directory_array[0].page_directory[1].mb.ps = 1;		/* 1 indicates 4MB */
	page_directory_array[0].page_directory[1].mb.g = 1;			/* global now */
	page_directory_array[0].page_directory[1].mb.avail = 0;		/* initialize */
	page_directory_array[0].page_directory[1].mb.pat = 0;		/* no processor now, so reset to 0 */
	page_directory_array[0].page_directory[1].mb.reserved = 0;	/* For a page-directory entry for a 4-MByte page, bits 12 through 21 are reserved and must be set to 0. */
	page_directory_array[0].page_directory[1].mb.page_base_addr = 1;	/* get the address for index===>0x400000  32-22bit equals to 1 */
	
	/* then initialize the rest not present directory===>4MB */
	for (i=2;i<NUMBER_ENTRIES;i++){
		page_directory_array[0].page_directory[i].mb.p = 0;			/* not present */
		page_directory_array[0].page_directory[i].mb.rw = 1;		/* read or write */
		page_directory_array[0].page_directory[i].mb.us = 0;		/* assign the supervisor privilege level */
		page_directory_array[0].page_directory[i].mb.pwt = 0;		/* write-back caching is enabled for the associated page or page table */
		page_directory_array[0].page_directory[i].mb.pcd = 0;		/* the page or page table can be cached */
		page_directory_array[0].page_directory[i].mb.a = 0;			/* a page or page table is initially loaded into physical memory */
		page_directory_array[0].page_directory[i].mb.d = 0;			/* when a page is initially loaded into physical memory */
		page_directory_array[0].page_directory[i].mb.ps = 0;		/* 0 indicates 4KB */
		page_directory_array[0].page_directory[i].mb.g = 0;			/* not global now */
		page_directory_array[0].page_directory[i].mb.avail = 0;		/* initialize */
		page_directory_array[0].page_directory[i].mb.pat = 0;		/* no processor now, so reset to 0 */
		page_directory_array[0].page_directory[i].mb.reserved = 0;	/* For a page-directory entry for a 4-MByte page, bits 12 through 21 are reserved and must be set to 0. */
		page_directory_array[0].page_directory[i].mb.page_base_addr = i;	/* get the address for index */
	}
	
	return;
}

/* void init_paging()
 * Inputs: None
 * Return Value: None
 * Function: set cr0, cr3, and cr4 to enable paging
 */
void enable_paging() {
	page_dir_addr = (uint32_t)(&page_directory_array[0]);
	/* enable paging===>initialize cr0/3/4 */
	asm volatile (
	/* set cr3 */
	"movl page_dir_addr, %%eax;"	/* address of directory===>eax */
	"andl $0xFFFFFFE7,%%eax;"
	"movl %%eax, %%cr3;"			/* cr3===>address */
	/* set cr4 */
	"movl %%cr4, %%eax;"
	"orl $0x00000010, %%eax;"		/* set physical address extension */
	"movl %%eax, %%cr4;"					
	/* set cr0 */
	"movl %%cr0, %%eax;"
	"orl $0x80000000, %%eax;"		/* If 1, enable paging and use the CR3 register, else disable paging */
	"movl %%eax, %%cr0;"
	:								/* no output */
	:								/* no input */
	:"%eax","cc"					/* modify */
	);
	return;
}

/* void flush_TLB()
 * Inputs: None
 * Return Value: None
 * Function: Flush TLB. We need to reload cr3
 */
void flush_TLB() {
	asm volatile(
                 "mov %%cr3, %%eax;"
                 "mov %%eax, %%cr3;"
                 :::"%eax"
                 );
}
