/**************************************************************
 * Class:  CSC-415-01 Fall 2021
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs:
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


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "bfs.h"
#include "bfs_helpers.h"

struct vcb_s* bfs_vcb;
struct block_group_desc* bfs_gdt;
struct bfs_dir_entry* bfs_root;

int is_valid_volname(char* name);
void print_uuid(uint8_t* uuid);

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("size is %ld\n", sizeof(struct bfs_dir_entry));

	printf ("Initializing File System with %ld blocks with a block size of %ld\n", 
			numberOfBlocks, blockSize);

	bfs_vcb = malloc(blockSize);

	// read first block of memory
	LBAread(bfs_vcb, 1, 0);

	// if the file system already exists, load it 
	if (bfs_vcb->magic == BFS_MAGIC) {

		fprintf(stderr, "Existing filesystem found with uuid ");
		print_uuid(bfs_vcb->uuid);


	/*
	 * if the file system does not exist, create it 
	 * broadly there are 3 steps here: 
	 * first, create the vcb, which is always the 0th block
	 * next, create the gdt, which always starts at the 1st block, and is usually one block
	 * but can be larger if necessary.
	* finally, create the directory array, which starts after the gdt, and is arbitrarily 
	* long
	*/ 
	} else { 

		// for now, volume name is hardcoded
		char* volume_name = "NewVolume";

		if(!is_valid_volname(volume_name)) {
			fprintf(stderr, "Invalid volume name \n");
		}

		// populate bfs_vcb with initial values 
		if (bfs_vcb_init(volume_name, numberOfBlocks, blockSize)) {
			fprintf(stderr, "Failed to create volume\n");
			return 1;
		} 

		// write newly created VCB to disk
		if (LBAwrite(bfs_vcb, 1, 0) != 1) {
			fprintf(stderr, "Error: Unable to LBAwrite VCB to disk\n");
			return 1;
		}

		// allocate one empty block for block group descriptor table
		bfs_gdt = calloc(bfs_vcb->block_size, bfs_vcb->gdt_size);
		if (bfs_gdt_init(bfs_gdt)) {
			fprintf(stderr, "Error: Unable to initialize GDT\n");
			return 1;
		}
		printf("Initialized gdt with %d groups of size %d\n", 
				bfs_vcb->block_group_count, bfs_vcb->block_group_size);
		printf("first block info:\n");
		struct block_group_desc first_entry = bfs_gdt[0];
		printf("position: %ld  free blocks count: %d\n", first_entry.bitmap_location, 
				first_entry.free_blocks_count);
		if (LBAwrite(bfs_gdt, bfs_vcb->gdt_size, 1) != 1) {
			fprintf(stderr, "Error: Unable to LBAwrite GDT to disk\n");
			return 1;
		}

		// initialize root directory
		int pos = bfs_get_free_block();
		if (pos == -1) {
			fprintf(stderr, "Error: Unable to get free block for root directory\n");
		}

		// get buffer for root directory
		struct bfs_dir_entry* root_directory = malloc(bfs_vcb->block_size);
		if (LBAread(root_directory, 1, pos) != 1) {
			fprintf(stderr, "Error: Unable to LBAread buffer %d\n", pos);
			free(root_directory);
			exitFileSystem();
		}
		bfs_create_root(root_directory, pos);
		if (LBAwrite(root_directory, 1, pos) != 1) {
			fprintf(stderr, "Error: Unable to LBAwrite buffer %d\n", pos);
			exitFileSystem();
		}
		printf("Wrote root directory to block %d\n", pos);
		print_dir_entry(&root_directory[0]);
		print_dir_entry(&root_directory[1]);
	}
	return 0;
}


void exitFileSystem ()
{
	//wirte current GDT
	if (LBAwrite(bfs_gdt, 1, bfs_vcb->gdt_size) != bfs_vcb->gdt_size) {
		fprintf(stderr, "Error: LBAwrite failed to write GDT\n");
	}
	free(bfs_gdt);
	bfs_gdt = NULL;

	// write current VCB 
	if (LBAwrite(bfs_vcb, 0, 1 != 1)) {
		fprintf(stderr, "LBAwrite failed to write bfs_vcb\n");
	}


	free(bfs_vcb);
	bfs_vcb = NULL;

	printf ("System exiting\n");
}

/*
 * Check if a given string is a valid volume name. Valid strings must:
 * Have length 1-63
 * Contain only english alphabet letters and numbers
 */
int is_valid_volname(char* string) {
	int len = strlen(string);
	if (len < 1 || len > 63) 
		return 0;

	for (int i = 0; i < len; i++) {
		if (!isascii( (int) string[i])) {
			return 0;
		}
	}
	
	return 1;
}

void print_uuid(uint8_t* uuid) 
{
	for (int i = 0; i < 16; i++) {
		if ( i >= 4 && i % 2 == 0 && i < 12)
			printf("-");
		printf("%02X", uuid[i]);
	}
	printf("\n");
}
