/**************************************************************
* Class:  CSC-415-0#  Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: b_io.h
*
* Description: Interface of basic I/O functions
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include "bfs.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512
#define INIT_FILE_LEN 16

typedef struct b_fcb {
	/** TODO add all the information you need in the file control block **/
	char* buf;		//holds the open file buffer
	int buf_index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	int access_mode;	// The current access mode
	int current_block;   // tracks the block num
	struct bfs_dir_entry * file; // Holds the file info
} b_fcb;

typedef int b_io_fd;

b_io_fd b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_write (b_io_fd fd, char * buffer, int count);
int b_seek (b_io_fd fd, off_t offset, int whence);
int b_close (b_io_fd fd);

#endif

