/*
 * Definitions for BFS system Author: Griffin Evans */
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
#define NUM_FILES 2048
#define ROOT_INODE 1;

/*
 * vcb_s: Volume Control Block
 * The VCB is the first block of the file system, which contains basic parameters of the system.
 */
struct vcb_s {
	uint32_t       	block_size;             // block size
	uint32_t       	block_count;            // number of blocks
	char           	volume_name[64];        // volume name
	uint8_t        	uuid[16];               // volume signature 
	uint32_t	magic;			// magic signature
	uint32_t	block_group_size;	// number of blocks per block group
	uint32_t 	block_group_count;	// number of block groups
	uint32_t	gdt_size;		// size (in blocks) for group descriptor table
};

/*
 * block_group_desc: Block Group Descriptor Table
 * The second block of the file system, after the Volume Control Block, is the Block Group
 * 	Descriptor Table, which contains an array of descriptors for each block group.
 *
 * The first block of each block group is that block group's bitmap, where each bit indicates if 
 * 	a block is available (0) or in use (1). Each block group contains BLOCK_SIZE * 8 blocks. 
 *
 * A block group descriptor contains the following information (positions in bytes):
 *   0-3:  The address (lbaPosition) of block usage bitmap for that group
 *   4-8:  The number of unallocated blocks in the group
 *   8-12: The number of directories in the block group
 */
struct block_group_desc {
	uint32_t bitmap_location;
	uint32_t free_blocks_count;
	uint32_t dirs_count;
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
int vcb_init(struct vcb_s* vcb, char* name, uint64_t num_blocks, uint64_t block_size);

/*
 * Create the group descriptor table
 */
void init_gdt(struct vcb_s* vcb, struct block_group_desc* gdt);


/*
 * Initialize a directory entry. Returns 0 on success, non-zero on failure
 */
int create_dentry(struct vcb_s* vcb, struct direntry_s* dentry, char* name, int size, int type);

/*
 * Generate a random UUID. Returns 0 on success, non-zero on failure
 */
void bfs_generate_uuid(uint8_t* uuid);

#endif
