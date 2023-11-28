/**************************************************************
* Class:  CSC-415-01 Fall 2023
* Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
* Student IDs: 922498210, 922432027, 917386319, 922722304
* GitHub Name: griffinevans
* Group Name: Team CDeez
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "bfs.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512
#define INIT_FILE_LEN 16

typedef struct b_fcb {
	/** TODO add all the information you need in the file control block **/
	char* buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	int access_mode;	// The current access mode
	int currBlockNum;   // tracks the block num
	struct bfs_dir_entry * file; // Holds the file info
} b_fcb;

b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++) {
		fcbArray[i].buf = NULL; //indicates a free fcbArray
	}

	startup = 1;
}

//Method to get a free FCB element
b_io_fd b_getFCB ()
{
	for (int i = 0; i < MAXFCBS; i++) {
		if (fcbArray[i].buf == NULL) {
			return i;		//Not thread safe (But do not worry about it for this assignment)
		}
	}
	return (-1);  //all in use
}

// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open(char* filename, int flags)
{
	// Initialize our system
	if (startup == 0)
		b_init();

	if (fs_isDir(filename)) {
		fprintf(stderr, "Cannot b_open %s because it is a directory.", filename);
		return (-1);
	}

	struct bfs_dir_entry* target_file = malloc(sizeof(struct bfs_dir_entry));
	if (target_file == NULL) {
		fprintf(stderr, "Failed to allocate memory for bfs_dir_entry.\n");
		return (-1);
	}
	// Handle when file doesn't exist
	if (get_file_from_path(target_file, filename)) {
		// When file doesn't exist and O_CREAT is set
		if (flags & O_CREAT) {
			char* trimmed_name = get_filename_from_path(filename);
			if (*trimmed_name == '\0') {
				fprintf(stderr, "Filename from path is empty.\n");
				free(trimmed_name);
				free(target_file);
				return (-1);
			}

			bfs_block_t pos = bfs_get_free_blocks(INIT_FILE_LEN);
			bfs_create_dir_entry(target_file, trimmed_name, 0, pos, 1);

			if (LBAwrite(target_file, INIT_FILE_LEN, pos) != INIT_FILE_LEN) {
				fprintf(stderr, "Unable to LBAwrite pos %llu in b_open.\n", pos);
				free(trimmed_name);
				free(target_file);
				return (-1);
			}
		} else {
			fprintf(stderr,
							"Cannot b_open %s. File does not exist and create flag has not been set.\n",
							filename);
			free(target_file);
			return (-1);
		}
	}

	// The rest of this code is handling when the file does exist
	// and the flags are set correctly

	b_io_fd returnFd = b_getFCB();
	// Handle b_getFCB errors
	if (returnFd == -1) {
		fprintf(stderr, "No available FCB.\n");
		free(target_file);
		return returnFd;
	}

	// Load file into fcbArray
	fcbArray[returnFd].file = malloc(sizeof(struct bfs_dir_entry));
	if (fcbArray[returnFd].file == NULL) {
		fprintf(stderr, "Failed to malloc for buffer.\n");
		free(target_file);
		return (-1);
	}

	memcpy(fcbArray[returnFd].file, target_file, sizeof(struct bfs_dir_entry));

	// Initialize the b_fcb struct
	char* buffer = malloc(bfs_vcb->block_size);
	if (buffer == NULL) {
		fprintf(stderr, "Failed to malloc for buffer.\n");
		free(target_file);
		return (-1);
	}

	fcbArray[returnFd].buf = buffer;
	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].buflen = 0;
	fcbArray[returnFd].access_mode = flags;

	return (returnFd); // all set
}


// Interface to seek function
int b_seek (b_io_fd fd, off_t offset, int whence)
{
	if (startup == 0)
		b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)) {
		return (-1); 					//invalid file descriptor
	}

    // to checks if the file exists
	if (fcbArray[fd].file == NULL) {
		return (-1); 			//empty fd
	}

    // for case of going beyond the start of file
    // sets to 0 [start of file]
    if(offset < 0 && fcbArray[fd].index + offset < 0)
    {
        fcbArray[fd].index = 0;
        return 0;
    }

    switch(whence)
    {
        case 0 :
            fcbArray[fd].index = offset;     // For SEEK_SET
            break;
        case 1 :
            fcbArray[fd].index += offset;   // For SEEK_CUR
            break;
        case 2 :
            fcbArray[fd].index = fcbArray[fd].file->size + offset;    // For SEEK_END
            break;
        default:
            break;
    }

    // returns the new start point
    return fcbArray[fd].index;
}

// Interface to write function
int b_write (b_io_fd fd, char * buffer, int count)
{
	if (startup == 0)
		b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)) {
		return (-1); 					//invalid file descriptor
	}

	return (0); //Change this
}

// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
{

	if (startup == 0)
		b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)) {
		return (-1); 					//invalid file descriptor
	}

	return (0);	//Change this
}

// Interface to Close the file	
int b_close (b_io_fd fd)
{
	// Write remaining content from fd's buffer onto disk
	if (fcbArray[fd].index > 0) {
		if (fcbArray[fd].access_mode & (O_RDWR | O_WRONLY)) {
			LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].currBlockNum);
		} else {
			// There shouldn't be in anything in the buffer anyways, but checking access mode
			// just in case.
			fprintf(stderr, "Couldn't write remaining buffer on b_close.\n");
		}
	}

	}
	// TODO Calculate the actual amount of blocks the file used. It might've not used all 16 blocks.

	// TODO memcpy changes from the fcb dir entry into the directory array itself

	// TODO Write dir array to disk

	// TODO free the fd from memory
	return 0;
}
