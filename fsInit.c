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

		fprintf(stderr, "Existing filesystem found with uuid \n");
		print_uuid(vcb->uuid);

	} else { 

		char* volume_name = "MyVolume";

		if(!is_valid_volname(volume_name)) {
			fprintf(stderr, "Invalid volume name \n");
		}

		if (vcb_init(vcb, volume_name)) {
			fprintf(stderr, "Failed to create volume\n");
			return 1;
		} 

		printf("Sucessfully created volume %s with uuid ", vcb->volume_name);
		print_uuid(vcb->uuid);

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
	for(int i = 0; i < 16; i++) {
		if (i > 0 && i % 4 == 0) 
			printf("-");
		printf("%x", uuid[i]);
	}
	printf("\n");
}
