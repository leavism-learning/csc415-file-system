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

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", 
			numberOfBlocks, blockSize);

	vcb = malloc(BLOCK_SIZE);

	// read first block of memory
	LBAread(vcb, 1, 0);

	if (vcb->magic == 0x434465657A) {

		printf("Existing filesystem found!\n");

	} else { 

		char* volume_name = "MyVolume";

		if (vcb_init(vcb, volume_name)) {
			printf("Failed to create volume\n");
			return 1;
		} 

		printf("Sucessfully created volume %s\n", vcb->volume_name);

		if (LBAwrite(vcb, 1, 0) != 1) {
			fprintf(stderr, "Error: Unable to LBAwrite VCB to disk\n");
			return 1;
		}
	}


	return 0;
}


void exitFileSystem ()
{
	free(vcb);
	vcb = NULL;

	printf ("System exiting\n");
}
