#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "mfs.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb {
	/** TODO add all the information you need in the file control block **/
	char* buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	int access_mode;	// The current access mode
	int block_index;	// holds current block position in file
	struct bfs_dir_entry * file; // Holds the file infos
} b_fcb;

int startup = 0;	//Indicates that this has not been initialized

b_fcb fcbArray[MAXFCBS];

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
b_io_fd b_open (char * filename, int flags)
	{
	b_io_fd returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//
	
	if (startup == 0) b_init();  //Initialize our system
	
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	
	return (returnFd);						// all set
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

	return (0); //Change this
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

/*
 //* Directory Entry stores first-class information about a file. 128 byte size
 
struct bfs_dir_entry {
	uint64_t size;				 // file size in bytes (max 16384 PiB)
	uint32_t len;				 // file length in blocks 
	bfs_block_t location;		 // lba position of file extents
	uint8_t file_type;			 // 0 if directory, otherwise file
	time_t date_created;		 // file creation time
	time_t date_modified;		 // last time file was modified
	time_t date_accessed;		 // last time file was read
	char name[MAX_FILENAME_LEN]; // file name
};*/

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

	if (startup == 0) {
		b_init();  //Initialize our system
	}

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	
	if (count < 0)
	{
		return (-1);				//invalid count
	}

	//TODO: check access mode for file?

	// checks if file exists
	if (fcbArray[fd].file == NULL) {
		return (-1); 			//empty fd
	}

	// keeps track of total bytes to be read
	// either count or size of file, depending on which is smaller
	int totalToRead = count;
	if (fcbArray[fd].buflen < count) {
		totalToRead = fcbArray[fd].buflen;
	}

	int totalRead = 0;
	char* fileBuf = fcbArray[fd].buf;
	int size = fcbArray[fd].file->len;

	//find the file
	return (0);	//Change this
	}
	

// Interface to Close the file	
int b_close (b_io_fd fd)
{
	return 0;
}