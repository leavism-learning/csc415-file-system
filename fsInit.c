/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: fsInit.c
 *
 * Description: Main driver for file system assignment.
 *
 * This file is where you will start and initialize your system
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "bfs.h"

struct vcb_s* bfs_vcb;
struct block_group_desc* bfs_gdt;
struct bfs_dir_entry* bfs_cwd;
char* bfs_path = NULL;

int is_valid_volname(char *name);
void print_uuid(uint8_t *uuid);

/*
* Initializes file system with a given number of blocks and block size
* If the file system does not exist, creates it
* else, loads the file system matching the BFS_MAGIC number
*/
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize) 
{

	printf("Initializing File System with %ld blocks with a block size of %ld\n",
		numberOfBlocks, blockSize);

	bfs_vcb = malloc(blockSize);

	// read first block of memory
	if (LBAread(bfs_vcb, 1, 0) != 1) {
		fprintf(stderr, "Unable to LBAread block 0\n");
		return 1;
	}

	// if the file system already exists, load it
	if (bfs_vcb->magic == BFS_MAGIC) {

		fprintf(stderr, "Existing filesystem found with uuid ");
		print_uuid(bfs_vcb->uuid);
		
		bfs_gdt = malloc(bfs_vcb->block_size * bfs_vcb->gdt_len);
		if (LBAread(bfs_gdt, bfs_vcb->gdt_len, 1) != bfs_vcb->gdt_len) {
			fprintf(stderr, "Unable to LBAread block 1\n");
			return 1;
		}

		bfs_cwd = malloc(bfs_vcb->block_size * bfs_vcb->root_len);
		printf("initializing root at %ld\n", bfs_vcb->root_loc);
		if (LBAread(bfs_cwd, bfs_vcb->root_len, bfs_vcb->root_loc) != bfs_vcb->root_len) {
			fprintf(stderr, "Error: Unable to LBAread buffer %ld\n", bfs_vcb->root_loc);
			free(bfs_cwd);
			bfs_cwd = NULL;
			exitFileSystem();
		}

		bfs_path = strdup("/");
	} 

	/*
	 * if the file system does not exist, create it
	 * broadly there are 3 steps here:
	 * first, create the vcb, which is always the 0th block
	 * next, create the gdt, which always starts at the 1st block, and is
	 * usually one block but can be larger if necessary. finally, create the
	 * directory array, which starts after the gdt, and is arbitrarily long
	 */
	else {

		// for now, volume name is hardcoded
		char *volume_name = "NewVolume";

		if (!is_valid_volname(volume_name)) {
			fprintf(stderr, "Invalid volume name \n");
		}

		// populate bfs_vcb with initial values
		if (bfs_vcb_init(volume_name, numberOfBlocks, blockSize)) {
			fprintf(stderr, "Failed to create volume\n");
			return 1;
		}

		if (write_current_vcb()) {
			exitFileSystem();
		}

		// allocate empty blocks for block group descriptor table
		bfs_gdt = calloc(bfs_vcb->block_size, bfs_vcb->gdt_len);
		if (bfs_gdt_init(bfs_gdt)) {
			fprintf(stderr, "Error: Unable to initialize GDT\n");
			return 1;
		}
		printf("Initialized gdt with %d groups of size %d\n",
		 bfs_vcb->block_group_count, bfs_vcb->block_group_size);
		struct block_group_desc first_entry = bfs_gdt[0];
		printf("position: %ld  free blocks count: %d\n",
		 first_entry.bitmap_location, first_entry.free_blocks_count);
		if (LBAwrite(bfs_gdt, bfs_vcb->gdt_len, 1) != 1) {
			fprintf(stderr, "Error: Unable to LBAwrite GDT to disk\n");
			return 1;
		}

		// initialize root directory with size 1
		bfs_vcb->root_loc = bfs_get_free_blocks(INIT_DIR_LEN);
		bfs_vcb->root_len = INIT_DIR_LEN;

		if (write_current_vcb()) {
			exitFileSystem();
		}

		if (bfs_vcb->root_loc == -1) {
			fprintf(stderr, "Error: Unable to get free block for root directory\n");
		}

		// create root dir
		struct bfs_dir_entry* dir_arr = malloc(bfs_vcb->block_size * bfs_vcb->root_len);
		LBAread(dir_arr, bfs_vcb->root_len, bfs_vcb->root_loc);

		struct bfs_dir_entry root_here;
		bfs_create_dir_entry(&dir_arr[0], ".", bfs_vcb->root_len * bfs_vcb->block_size, 
			bfs_vcb->root_loc, 0);
		bfs_create_dir_entry(&dir_arr[1], "..", bfs_vcb->root_len * bfs_vcb->block_size, 
			bfs_vcb->root_loc, 0);
		dir_arr[2].name[0] = '\0';

		LBAwrite(dir_arr, bfs_vcb->root_len, bfs_vcb->root_loc);

		bfs_cwd = dir_arr;

		bfs_path = strdup("/");
	}

	return 0;
}

/* 
* Frees up the allocated space of the current working directory, group descriptor table
* and the volume control block
* Will do one last write of GDT and VCB before freeing
*/
void exitFileSystem() 
{
	// try to write current GDT and VCB before exiting
	write_current_vcb();
	write_current_gdt();

	if (bfs_cwd != NULL) {
		free(bfs_cwd);
		bfs_cwd = NULL;
	}

	if (bfs_gdt != NULL) {
		free(bfs_gdt);
		bfs_gdt = NULL;
	}

	if (bfs_vcb != NULL) {
		free(bfs_vcb);
		bfs_vcb = NULL;
	}

	printf("System exiting\n");
}

/*
 * Check if a given string is a valid volume name. Valid strings must:
 * Have length 1-63
 * Contain only english alphabet letters and numbers
 */
int is_valid_volname(char *string) 
{
	int len = strlen(string);
	if (len < 1 || len > 63)
		return 0;

	for (int i = 0; i < len; i++) {
		if (!isascii((int)string[i])) {
			return 0;
		}
	}

	return 1;
}

/*
* Displays the volume's UUID
*/
void print_uuid(uint8_t *uuid) 
{
	for (int i = 0; i < 16; i++) {
		if (i >= 4 && i % 2 == 0 && i < 12)
			printf("-");
		printf("%02X", uuid[i]);
	}
	printf("\n");
}
