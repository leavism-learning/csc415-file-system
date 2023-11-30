/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
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

typedef struct b_fcb {
	/** TODO add all the information you need in the file control block **/
	char* buf;		//holds the open file buffer
	int buf_index;		//holds the current position in the buffer
	int buf_size;		//holds how many valid bytes are in the buffer
	int access_mode;	// The current access mode
	bfs_block_t* block_arr;		// array of block numbers
	int block_idx;		// index in block_arr
	int current_block;   // current block number  
	struct bfs_dir_entry* file; // Holds the file info
	struct bfs_dir_entry* parent_dir_entry;
} b_fcb;

typedef int b_io_fd;

b_io_fd b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_write (b_io_fd fd, char * buffer, int count);
int b_seek (b_io_fd fd, off_t offset, int whence);
int b_close (b_io_fd fd);
int b_move(char* dest, char* src);

#endif

