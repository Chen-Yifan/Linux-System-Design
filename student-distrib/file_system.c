/* file_system.c - build file system */

#include "file_system.h"
#include "lib.h"
#include "syscall_handler.h"


/* int32_t read_dentry_by_name()
 * Task:  fill in the dentry t block passed as their second argument with the file name,
 *																			 file type,
 *																			 inode number for the file
 * Input : fname----a pointer of the file name need to be read
 *		   dentry---a pointer of the new dentry need to be filled
 * Output: success return 0, otherwise return -1
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
	int length = strlen((int8_t*)fname);	/* in lib.c */
	int i,true_length;
	/* check the length which need to be in 0<length<=32 */
	if (length>MAX_NAME_LENGTH || length==0){
		return -1;
	}
	
	if (length==MAX_NAME_LENGTH){
		true_length = length+1;
	}
	else{
		true_length = length;
	}
	
	/* check the dentry with the same name */
	for (i=0;i<bootBlock->num_dir_entries;i++){
		/* if find the same name----in lib.c */
		/* check the length of the file first */
		if (true_length == strlen(bootBlock->dir_entries[i].file_name)){
			if(strncmp((int8_t*)fname,bootBlock->dir_entries[i].file_name,length)==0){
				strcpy(dentry->file_name,bootBlock->dir_entries[i].file_name);		/* pass the file name */
				dentry->file_type = bootBlock->dir_entries[i].file_type;			/* pass the file type */
				dentry->inode = bootBlock->dir_entries[i].inode;					/* pass the number of inode */
				inode_number = bootBlock->dir_entries[i].inode;
				
				index_node_t* inode_block;
				inode_block = (index_node_t*)bootBlock+inode_number+1;		// find the corresponding inode block
	
				f_size = inode_block->length;
				return 0;	/* success */
			}
		}
	}
	/* don't find, fail */
	return -1;
}


/* int32_t read_dentry_by_index()
 * Task:  fill in the dentry t block passed as their second argument with the file name,
 *																			 file type,
 *																			 inode number for the file
 * Input : index----a pointer of the index of inode need to be read
 *		   dentry---a pointer of the new dentry need to be filled
 * Output: success return 0, otherwise return -1
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
	/* check the index which need to be in 0=<index<63===>0--(N-1) */
	if (index>=bootBlock->num_dir_entries || index<0){
		return -1;
	}
	
	/* store the dentry by index */
	strcpy(dentry->file_name,bootBlock->dir_entries[index].file_name);		/* pass the file name */
	dentry->file_type = bootBlock->dir_entries[index].file_type;			/* pass the file type */
	dentry->inode = bootBlock->dir_entries[index].inode;					/* pass the number of inode */
	
	return 0;
}

/* int32_t read_data()
 * Task: copy the file data into buf
 * Input : inode----the inode number need to read (index node)
 *		   offset---the position from which to start reading
 *		   buf------a pointer to a buf which we copy data into
 *		   length---the number of bytes to read
 * Output: success return the number of bytes, otherwise return -1
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	/* check the index first */
	 if (inode>=bootBlock->num_inodes || inode<0){
		 return -1;
	 }
	 
	 index_node_t* inode_block;
	 inode_block = (index_node_t*)bootBlock+inode+1;		/* find the corresponding inode block  */
	 
	 /* find the location of the start data block */
	 uint32_t data_block_index = offset/FOUR_KB;				/* find the corresponding data block */
	 uint32_t data_line_index = offset%FOUR_KB;				/* find the corresponding data in data block */
	 uint8_t* data_ptr = (uint8_t*)(bootBlock + bootBlock->num_inodes + 1 + inode_block->data_block[data_block_index])+data_line_index;
	 int i;
	 
	 for (i=0;i<length;i++){
		  if (i+offset >= inode_block->length){
			  return i;						/* data in current inode has already been copied */
		  }
		  
		  buf[i] = *data_ptr;				/* store byte in buf */
		  data_line_index++;				/* move to next data in the current data block */

		  if (data_line_index%FOUR_KB==0){		/* the current data block has already finished, move to next data block */
			  data_block_index++;
			  if (inode_block->data_block[data_block_index]>=bootBlock->num_data_blocks){
				  return -1;				/* the next data block doesn't exist */
			  }
			  /* reset data pointer to the first of the next data block */
			  data_ptr = (uint8_t*)(bootBlock + bootBlock->num_inodes + 1 + inode_block->data_block[data_block_index]);
			  /* now, the data index is 0 */
		  }
		  else{
			  data_ptr++;	/* move to next data */
		  }
			  
	 }
		 
	 return length;
 }
 
 /* void init_file_system()
 * Task: initialize every global variable used in this driver
 * Input : 
 * Output: None
 */
void init_file_system(uint32_t bootBlock_addr){
	 
	 /* initialize */
	 bootBlock = (boot_block_t*) bootBlock_addr;
	 dir_number = 0;
 }



/* int32_t file_open(): initialize any temporary structures
 * Input:  filename
 * Output: return 0
 */
int32_t file_open (const uint8_t* filename){
	return 0;
}

/* int32_t file_close(): undo what you did in the open function
 * Input:  fd
 * Output: return 0
 */
int32_t file_close (int32_t fd){
	return 0;
}

/* int32_t file_read(): read files filename by filename, including “.”
 * Input : fd		--	file descriptor
 *		   buf		--	passed buffer
 *		   nbytes	--	number of bytes need to be copied
 * Output: return 0 for success, otherwise return -1
 */
int32_t file_read (int32_t fd, void* buf, int32_t nbytes){
	
	// get pcb
	pcb_t* pcb = get_specific_pcb(cur_pid);
	uint32_t offset = pcb->fd_table[fd].file_position;
	uint32_t inode = pcb->fd_table[fd].inode;
	// read data
	uint32_t num_data = read_data(inode,offset,(uint8_t*)buf,nbytes);
	// move to next position of the data
	pcb->fd_table[fd].file_position+=num_data;
	
	return num_data;
}

/* int32_t file_write(): do nothing
 * Output: return -1
 */
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes){
	return -1;
}

/* int32_t dir_open(): initialize any temporary structures
 * Input:  filename
 * Output: return 0
 */
int32_t dir_open (const uint8_t* filename){
	return 0;
}

/* int32_t dir_close(): undo what you did in the open function
 * Input:  fd
 * Output: return 0
 */
int32_t dir_close (int32_t fd){
	return 0;
}

/* int32_t dir_read(): read files filename by filename, including “.”
 * Input : fd		--	file descriptor
 *		   buf		--	passed buffer
 *		   nbytes	--	number of bytes need to be copied
 * Output: return the length of filename, otherwise return -1
 */
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes){
	/* reading finished, reset */
	
	/* check the dentry */
	dentry_t dentry;
	
	if (read_dentry_by_index(dir_number,&dentry) == 0){
		uint32_t length = strlen((int8_t*)dentry.file_name);		// get the length
		if (length>=MAX_NAME_LENGTH){
			length = MAX_NAME_LENGTH;
		}
		strncpy((int8_t*)buf, (int8_t*)dentry.file_name, length);	// copy to buf
		dir_number++;												// move to next dir
		return length;
	}
	else{
		dir_number = 0;		// move to the end
		return 0;
	}
}

/* int32_t dir_write(): do nothing
 * Output: return -1
 */
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes){
	return -1;
}

