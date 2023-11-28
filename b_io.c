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

#include "b_io.h"

b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

int next_block(b_fcb* fcb)
{
	fcb->current_block = fcb->block_arr[fcb->block_idx++];
	if (fcb->current_block == 0) {
		return 1;
	}
	return 0;
}

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
		// When file doesn't exist and O_CREAT is set, create the file
		if (!(flags & O_CREAT)) {
			fprintf(stderr, "Cannot b_open %s. File does not exist and create flag has not been set.\n", filename);
			free(target_file);
			return (-1);
		}

		char* parent_path = expand_pathname(filename);
		char* last_slash = strrchr(parent_path, '/');
		char* filename = NULL;
		if (last_slash != NULL) {
			filename = strdup(last_slash + 1);
			*last_slash = '\0';
		}
		if (strlen(parent_path) < 1) {
			parent_path = strdup("/");
		}

		// Allocate space for file
		void* extent_b = malloc(bfs_vcb->block_size);
		if (bfs_create_extent(extent_b, INIT_FILE_LEN)) {
			fprintf(stderr, "Unable to create extents for new file %s\n", filename);
			return -1;
		}
		bfs_block_t extent_loc = bfs_get_free_blocks(1);
		if (LBAwrite(extent_b, 1, extent_loc) != 1) {
			fprintf(stderr, "error writing new extent block\n");
		}
		free(extent_b);

		bfs_create_dir_entry(target_file, filename, 0, extent_loc, 1);
		struct bfs_dir_entry parent_entry;
		if (get_file_from_path(&parent_entry, parent_path)) {
			fprintf(stderr, "Unable to get parent dir for %s\n", target_file->name);
			return -1;
		}
		struct bfs_dir_entry* parent_dir = malloc(parent_entry.size);
		LBAread(parent_dir, parent_entry.len, parent_entry.location);
		// add the target_file dir entry to the parent dir array
		int i = 2;
		struct bfs_dir_entry d = parent_dir[i];
		while (d.name[0] != '\0') {
			d = parent_dir[++i];
		}
		parent_dir[i] = *target_file;
		parent_dir[++i].name[0] = '\0';
		LBAwrite(parent_dir, parent_entry.len, parent_entry.location);
		free(parent_dir);
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

	bfs_block_t* block_array = bfs_extent_array(target_file->location);
	if (block_array == NULL) {
		fprintf(stderr, "Unable to get block array for file %s\n", target_file->name);
		return 1;
	}

	fcbArray[returnFd].buf = buffer;
	fcbArray[returnFd].buf_index = 0;
	fcbArray[returnFd].buf_size = 0;
	fcbArray[returnFd].access_mode = flags;
	fcbArray[returnFd].block_arr = block_array;
	next_block(&fcbArray[returnFd]);

	free(buffer);
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
    if(offset < 0 && fcbArray[fd].buf_index + offset < 0)
    {
        fcbArray[fd].buf_index = 0;
        return 0;
    }

    switch(whence)
    {
        case 0 : 
            fcbArray[fd].buf_index = offset;     // For SEEK_SET   
            break;
        case 1 : 
            fcbArray[fd].buf_index += offset;   // For SEEK_CUR
            break;
        case 2 : 
            fcbArray[fd].buf_index = fcbArray[fd].file->size + offset;    // For SEEK_END
            break;
        default:
            break;
    }

    // returns the new start point
    return fcbArray[fd].buf_index;

    //to return the updated start position in the file
	//return (0); //Change this
}

// Interface to write function	
int b_write (b_io_fd fd, char* buffer, int count)
{
	if (startup == 0) 
		b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)) {
		return (-1); 					//invalid file descriptor
	}

	// Check for write flag
	if (!(fcbArray[fd].access_mode & O_RDWR) && !(fcbArray[fd].access_mode & O_WRONLY)) {
		fprintf(stderr, "Failed to b_write. Write flag was not set.");
		return (-1);
	}

	// Check that the file exists
	if (fcbArray[fd].file == NULL) {
		fprintf(stderr, "File does not exist to write to.");
		return (-1);
	}

	// Before writing, ensure that there are enough blocks to write to
	int new_size = fcbArray[fd].file->size + count; // in bytes
	int allocated_size = fcbArray[fd].file->len * bfs_vcb->block_size; // in bytes
	if (new_size - allocated_size < 0) {
		fprintf(stderr, "Failed to b_write: new size of file is less than currently allocated size.\n");
		return (-1);
	}

	int extra_blocks = bytes_to_blocks(new_size - allocated_size);
	if (extra_blocks > 0) {
		// Double the blocks allocated to the file
		bfs_block_t pos = bfs_get_free_blocks(fcbArray[fd].file->len + INIT_FILE_LEN);
		bfs_create_dir_entry(fcbArray[fd].file, fcbArray[fd].file->name, new_size, pos, 1);

	}

	int bytesRemainInBuffer = B_CHUNK_SIZE - fcbArray[fd].buf_index;
	int check1, check2, check3, bytesWrote = 0;
	int numBlocks, blocksWrote;


	if (bytesRemainInBuffer >= count) {
		check1 = count;
	} else {
		check1 = bytesRemainInBuffer;
		check3 = count - bytesRemainInBuffer;
		numBlocks = check3 / B_CHUNK_SIZE;
		check2 = numBlocks * B_CHUNK_SIZE;
		check3 -= check2;
	}

	if (check1 > 0) {
		memcpy(fcbArray[fd].buf + fcbArray[fd].buf_index, buffer, check1);

		blocksWrote = LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].current_block);

		fcbArray[fd].buf_index += check1;
		bytesWrote += check1;

		if(fcbArray[fd].buf_index == B_CHUNK_SIZE && check2 == 0) {
				fcbArray[fd].buf_index = 0;
				fcbArray[fd].current_block = bfs_get_free_block();
		}

		// TODO: Ask griffin if we need to keep track of which blocks we were on

	}

	if(check2 > 0) {
		fcbArray[fd].current_block = bfs_get_free_blocks(numBlocks + 1);

		for(int i = 0; i < numBlocks; i++)
		{
			blocksWrote += LBAwrite(buffer + bytesWrote, 1, fcbArray[fd].current_block);
			// Go to the next block because consecutive blocks
			fcbArray[fd].current_block++;
			bytesWrote += B_CHUNK_SIZE;
		}

		// For the last block
		fcbArray[fd].buf_index = 0;
	}

	if (check3 > 0) {
		memcpy(fcbArray[fd].buf+fcbArray[fd].buf_index, buffer + bytesWrote, check3);
		fcbArray[fd].buf_index += check3;

		blocksWrote += LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].current_block);

		bytesWrote += check3;

		if (fcbArray[fd].buf_index == B_CHUNK_SIZE) {
			fcbArray[fd].buf_index = 0;
			fcbArray[fd].current_block = bfs_get_free_block();
		}
	}

	time_t curr_time = time(NULL);
	fcbArray[fd].file->date_accessed = curr_time;
	fcbArray[fd].file->date_modified = curr_time;
	fcbArray[fd].file->size += bytesWrote;

	return bytesWrote; //Change this
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

	if (startup == 0) {
		b_init();  //Initialize our system
	}

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)) {
		return -1; 					//invalid file descriptor
	}

	if (count < 0) {
		return -1;				//invalid count
	}

	if (fcbArray[fd].access_mode & O_WRONLY) {
    	fprintf(stderr,"b_read: file does not have read access\n");
    	return -1;
    }

	// checks if file exists
	if (fcbArray[fd].file == NULL) {
		fprintf(stderr, "File does not exist\n");
		return -1; 			//empty fd
	}

	struct bfs_dir_entry* file = fcbArray[fd].file;

	int bytes_read = fcbArray[fd].current_block * bfs_vcb->block_size + fcbArray[fd].buf_index + 1;

	if (bytes_read >= file->size) {
		fprintf(stderr, "Requested more bytes than file size");
		return -1;
	}

	int bytes_available = fcbArray[fd].buf_size - fcbArray[fd].buf_index;
	int bytes_written = (fcbArray[fd].current_block * bfs_vcb->block_size) - bytes_available;

	if ((count + bytes_written) > file->size) {
		count = file->size - bytes_written;
		if (count < 0) {
			fprintf(stderr, "Negative count with %d written at block %d", bytes_written, fcbArray[fd].current_block);
		}
	}

	int part1 = count;
	int part2 = 0;
	int part3 = 0;
	int num_blocks = 0;

	if (bytes_available < count) {
		num_blocks  = (count - bytes_available) / bfs_vcb->block_size;
		part1 = bytes_available;
		part2 = num_blocks  * bfs_vcb->block_size;
		part3 = count - bytes_available - part2;
	}

	if (part1 > 0) {
		memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].buf_index, part1);
		fcbArray[fd].buf_index += part1;
	}

	if (part2 > 0) {
		int blocks_read = 0;
		for (int i = 0; i < num_blocks; i++) {
			blocks_read += LBAread(buffer + part1 + (i * bfs_vcb->block_size), 1, fcbArray[fd].current_block);
			if (next_block(&fcbArray[fd])) {
				fprintf(stderr, "Invalid next block\n");
				return 1;
			}
		}
		part2 = blocks_read * bfs_vcb->block_size;
	}

	if (part3 > 0) {
		int blocks_read = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].current_block);
		fcbArray[fd].buf_size = bfs_vcb->block_size;
		if (next_block(&fcbArray[fd])) {
			fprintf(stderr, "Invalid next block\n");
			return 1;
		}
		fcbArray[fd].buf_index = 0;
	}

	if (part3 > 0) {
		memcpy(buffer + part1 + part2, fcbArray[fd].buf + fcbArray[fd].buf_index, part3);
		fcbArray[fd].buf_index += part3;
	}

	fcbArray[fd].file->date_accessed = time(NULL);
	return part1 + part2 + part3;
}

// Interface to Close the file	
int b_close (b_io_fd fd)
{
	// Write remaining content from fd's buffer onto disk
	if (fcbArray[fd].buf_index > 0) {
		if (fcbArray[fd].access_mode & (O_RDWR | O_WRONLY)) {
			LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].current_block);
		} else {
			// There shouldn't be in anything in the buffer anyways, but checking access mode
			// just in case.
			fprintf(stderr, "Couldn't write remaining buffer on b_close.\n");
		}
	}

	return 0;
	// TODO Calculate the actual amount of blocks the file used. It might've not used all 16 blocks.

	// TODO memcpy changes from the fcb dir entry into the directory array itself

	// TODO Write dir array to disk

	// TODO free the fd from memory
}

int b_move(char *dest, char* src) 
{
	struct bfs_dir_entry source_de;
	if (get_file_from_path(&source_de, src)) {
		fprintf(stderr, "Unable to get file for desrc");
		return 1;
	}

	char* src_path = expand_pathname(src);
	char* parent_path = expand_pathname(dest);
	char* last_slash = strrchr(parent_path, '/');
	char* filename = NULL;
	if (last_slash != NULL) {
		filename = strdup(last_slash + 1);
		*last_slash = '\0';
	}
	if (strlen(parent_path) < 1) {
		parent_path = strdup("/");
	}

	struct bfs_dir_entry parent_entry;
	if (get_file_from_path(&parent_entry, parent_path)) {
		fprintf(stderr, "Unable to get parent file from path %s\n", parent_path);
		free(parent_path);
		free(filename);
		return 1;
	}
	struct bfs_dir_entry* parent_dir = malloc(parent_entry.size);

	int i = 0;
	struct bfs_dir_entry d = parent_dir[2];

	return 0 ;
}
