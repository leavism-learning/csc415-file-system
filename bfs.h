/*
 * Definitions for BFS system
 * Author: Griffin Evans
 */
#ifndef BFS_H
#define BFS_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/random.h>

#define MAX_FILENAME_LEN 64
#define BLOCK_SIZE 512
#define BLOCK_COUNT 8
#define NUM_FILES 2048
#define ROOT_INODE 1;

/*
 * Volume Control Block
 * The block and bitmap records which entries in the block table are in use. One bit 
 * represents the usage status of one data block
 */
struct vcb_s {
	uint32_t       	block_size;             // block size
	uint32_t       	block_count;            // number of blocks
	uint8_t*       	fs_bitmap;            	// bitmap of unused blocks
	uint64_t       	block_head;             // location of first block
	char           	volume_name[64];        // volume name
	uint8_t        	uuid[16];               // volume signature 
	uint32_t	magic;			// magic signature
};

/*
 * Directory Entry stores first-class information about a file
 * 18 bytes size, 
 */
struct direntry_s {
	uint64_t  size;               		// file size LIMIT ~18k petabytes
	uint32_t  location;                   	// file location in memory
	char      name[MAX_FILENAME_LEN];       // file name
	uint8_t   file_type;                    // 0 if directory, otherwise file 
	uint8_t   num_blocks;                   // number of blocks used by the file
	time_t    date_created;                 // file creation time       
	time_t    date_modified;                // last time file was modified
	time_t    date_accessed;                // last time file was read
};

/*
 * Creates a volume with the given name. Returns 0 on success, non-zero on failure
 */
int vcb_init(struct vcb_s* vcb, char* name);

/*
 * Initialize a directory entry. Returns 0 on success, non-zero on failure
 */
int create_dentry(struct vcb_s* vcb, struct direntry_s* dentry, char* name, int size, int type);

/*
 * Generate a random UUID. Returns 0 on success, non-zero on failure
 */
void bfs_generate_uuid(uint8_t* uuid);

#endif
