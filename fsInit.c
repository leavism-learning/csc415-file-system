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


struct vcb_s* vcb;

int is_valid_volname(char* name);
void print_uuid(uint8_t* uuid);

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", 
			numberOfBlocks, blockSize);

	vcb = malloc(BLOCK_SIZE);

	// read first block of memory
	LBAread(vcb, 1, 0);

	if (vcb->magic == 0x434465657A) {

		fprintf(stderr, "Existing filesystem found with uuid ");
		print_uuid(vcb->uuid);

		//TODO: initialize free space
		vcb->fs_bitmap = malloc(BLOCK_SIZE);


	} else { 

		// for now, volume name is hardcoded
		char* volume_name = "NewVolume";

		if(!is_valid_volname(volume_name)) {
			fprintf(stderr, "Invalid volume name \n");
		}

		// populate vcb with initial values 
		if (vcb_init(vcb, volume_name)) {
			fprintf(stderr, "Failed to create volume\n");
			return 1;
		} 

		// allocate one empty block for block group descriptor table
		bfs_group_desc = calloc(BLOCK_SIZE, 1);


		// write newly created VCB to disk
		if (LBAwrite(vcb, 1, 0) != 1) {
			fprintf(stderr, "Error: Unable to LBAwrite VCB to disk\n");
			return 1;
		}
	}


	return 0;
}


void exitFileSystem ()
{
	// write current VCB and free space bitmap before exiting
	if (LBAwrite(vcb->fs_bitmap, 1, 1 != 1)) {
		fprintf(stderr, "LBAwrite failed to write fs_bitmap\n");
	}

	if (LBAwrite(vcb, 0, 1 != 1)) {
		fprintf(stderr, "LBAwrite failed to write vcb\n");
	}

	free(vcb->fs_bitmap);
	vcb->fs_bitmap = NULL;

	free(vcb);
	vcb = NULL;

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


/*
 * create the group descriptor table
 * group descriptor table contains block_size / 24 block groups
 *
 * uint8_t* gdt: buffer to store gdt data  
 * uint8_t size: number of blocks allocated for the group descriptor table (at least one) 
 */
void init_gdt(uint8_t* gdt, uint8_t num_blocks) 
{
	// 0th block is VCB, block after the VCB is the GDT
	int lba_pos = 1;

	int block_start = num_blocks + 1;

	for (int i = 0; i < num_blocks * vcb->block_size; i += sizeof(struct block_group_desc)) {

		struct block_group_desc descriptor;
		descriptor.bitmap_location = block_start;
		descriptor.free_blocks_count = 0;
		descriptor.dirs_count = 0;

		gdt[i] = block_group_desc descriptor;
		lba_pos++;
	}
}
