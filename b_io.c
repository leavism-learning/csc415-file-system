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
#include "bfs.h"

b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

int next_block(b_fcb* fcb)
{
	fcb->current_block = fcb->block_arr[fcb->block_idx++];
	if (fcb->current_block == 0) {
		fprintf(stderr, "Error: Advancing to index %d (block %ld) overflows\n",
		  fcb->block_idx - 1, fcb->block_arr[fcb->block_idx - 1]);
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

	char* parent_dir;
	char* fname;
	if (get_parent_directory_and_filename(filename, &parent_dir, &fname)) {
		fprintf(stderr, "Unable to parse %s\n", filename);
		free(parent_dir);
		free(fname);
		return (-1);
	}

	struct bfs_dir_entry* parent_entry = malloc(sizeof(struct bfs_dir_entry));
	if (get_file_from_path(parent_entry, parent_dir)) {
		fprintf(stderr, "Unable to find directory %s\n", parent_dir);
		free(parent_dir);
		free(fname);
		free(parent_entry);
		return (-1);
	}

	struct bfs_dir_entry* target_file = malloc(sizeof(struct bfs_dir_entry));
	if (target_file == NULL) {
		fprintf(stderr, "Failed to allocate memory for bfs_dir_entry.\n");
		free(parent_dir);
		free(fname);
		free(target_file);
		free(parent_entry);
		return (-1);
	}

	// Handle when file doesn't exist
	if (get_file_from_path(target_file, fname)) {
		// When file doesn't exist and O_CREAT is set, create the file
		if (!(flags & O_CREAT)) {
			fprintf(stderr, "Cannot b_open %s. File does not exist and create flag has not been set.\n", fname);
			free(parent_dir);
			free(fname);
			free(target_file);
			free(parent_entry);
			return (-1);
		}

		// Allocate space for file
		void* extent_b = malloc(bfs_vcb->block_size);
		if (bfs_create_extent(extent_b, INIT_FILE_LEN * bfs_vcb->block_size)) {
			fprintf(stderr, "Unable to create extents for new file %s\n", fname);
			free(parent_dir);
			free(fname);
			free(target_file);
			free(parent_entry);
			return -1;
		}
		bfs_block_t extent_loc = bfs_get_free_blocks(1);
		if (LBAwrite(extent_b, 1, extent_loc) != 1) {
			fprintf(stderr, "error writing new extent block\n");
			free(parent_dir);
			free(fname);
			free(target_file);
			free(parent_entry);
			return -1;
		}
		free(extent_b);
		extent_b = NULL;

		bfs_create_dir_entry(
			target_file, 
			fname, 
			INIT_FILE_LEN * bfs_vcb->block_size, 
			extent_loc, 
			1
		);
		
		struct bfs_dir_entry* parent_dir = malloc(parent_entry->size);
		LBAread(parent_dir, parent_entry->len, parent_entry->location);
		// add the target_file dir entry to the parent dir array
		int i = 2;
		struct bfs_dir_entry d = parent_dir[i];
		while (d.name[0] != '\0') {
			d = parent_dir[++i];
		}
		parent_dir[i] = *target_file;
		parent_dir[++i].name[0] = '\0';
		LBAwrite(parent_dir, parent_entry->len, parent_entry->location);
	}
	free(parent_dir);
	free(fname);

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

	fcbArray[returnFd].parent_dir_entry = malloc(sizeof(struct bfs_dir_entry));

	memcpy(fcbArray[returnFd].file, target_file, sizeof(struct bfs_dir_entry));
	
	memcpy(fcbArray[returnFd].parent_dir_entry, parent_entry, sizeof(struct bfs_dir_entry));

	// Initialize the b_fcb struct
	char* buffer = malloc(bfs_vcb->block_size);
	if (buffer == NULL) {
		fprintf(stderr, "Failed to malloc for buffer.\n");
		free(target_file);
		return (-1);
	}

	bfs_block_t* block_array = bfs_extent_array(target_file->location);
	if (block_array == NULL) {
		// case for files with size 0
		printf("reading from empty file\n");
		block_array = malloc(sizeof(bfs_block_t));
		*block_array = 0;
	} 
	
	if (block_array[0] != 0) {
		if (LBAread(buffer, 1, block_array[0]) != 1) {
			fprintf(stderr, "Unable to read block %ld\n", block_array[0]);
			return 1;
		}
	}

	fcbArray[returnFd].buf = buffer;
	fcbArray[returnFd].buf_index = 0;
	fcbArray[returnFd].buf_size = bfs_vcb->block_size;
	fcbArray[returnFd].access_mode = flags;
	fcbArray[returnFd].block_arr = block_array;
	fcbArray[returnFd].block_idx = 0;
	fcbArray[returnFd].current_block = fcbArray[returnFd].block_arr[0];

	free(target_file);
	free(parent_entry);
	return returnFd; // all set
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
	if(offset < 0 && fcbArray[fd].buf_index + offset < 0) {
		fcbArray[fd].buf_index = 0;
		return 0;
	}

	int numOfBlocks = offset / bfs_vcb->block_size;
	off_t actualOffset = offset - (bfs_vcb->block_size * numOfBlocks);

	//TODO take care of case of offset going outside file size

	if (whence & SEEK_SET) {
		fcbArray[fd].current_block = fcbArray[fd].block_arr[numOfBlocks];
		fcbArray[fd].block_idx = numOfBlocks;
		fcbArray[fd].buf_index = actualOffset;
	} else if (whence & SEEK_CUR) {
		//maybe not needed
		// fcbArray[fd].current_block += fcbArray[fd].block_arr[numOfBlocks];
		// fcbArray[fd].block_idx += numOfBlocks;
		fcbArray[fd].buf_index += actualOffset;
	} else if (whence & SEEK_END) {
		// TODO handle case of size being multiple of chunk size

		int last_indx_block_arr = fcbArray[fd].file->size / bfs_vcb->block_size;
		fcbArray[fd].current_block = fcbArray[fd].block_arr[last_indx_block_arr];

		fcbArray[fd].buf_index = fcbArray[fd].file->size - fcbArray[fd].file->size*last_indx_block_arr;
		fcbArray[fd].buf_index += offset;
	}

	fcbArray[fd].file->date_accessed = time(NULL);

	// returns the new start point
	return fcbArray[fd].buf_index;
}

// Interface to write function	
int b_write (b_io_fd fd, char* buffer, int count)
{
	if (startup == 0) 
		b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)) {
		fprintf(stderr, "Invalid file descriptor\n");
		return -1; 					//invalid file descriptor
	}

	// Check for write flag
	if (fcbArray[fd].access_mode & O_RDONLY) {
		fprintf(stderr, "Failed to b_write. Write flag is not set.");
		return -1;
	}

	// Check that the file exists
	if (fcbArray[fd].file == NULL) {
		fprintf(stderr, "File does not exist.");
		return -1;
	}

	// Before writing, ensure that there are enough blocks to write to
	int extra_blocks = bytes_to_blocks(count);
	// create a new extent leaf for new blocks
	if (extra_blocks > 0) {
		fcbArray[fd].file->len += extra_blocks;

		// get existing extent block
		struct bfs_extent* extent_block = malloc(bfs_vcb->block_size);
		LBAread(extent_block, 1, fcbArray[fd].file->location);
		struct bfs_extent_header* header = ((struct bfs_extent_header*) extent_block); 

		// if entries >= max, need to adjust extents
		if (header[0].eh_entries >= header[0].eh_max) {
			printf("Reallocating space, max: %d\n", header[0].eh_max);
			// read the extent data into one big buffer
			
			printf("reading file data\n");
			char* file_data = malloc(bytes_to_blocks(fcbArray[fd].file->size + count) * bfs_vcb->block_size);
			if (bfs_read_extent(file_data, fcbArray[fd].file->location)) {
				fprintf(stderr, "Error reading file data\n");
				free(file_data);
				return -1;
			}
			printf("data is %s\n", file_data);

			printf("creating new extent\n");
			// make a new extent for all that data
			struct bfs_extent_header* new_extent_b = malloc(bfs_vcb->block_size);
			if (bfs_create_extent(new_extent_b, fcbArray[fd].file->size + count)) {
				fprintf(stderr, "Error creating new extents\n");
				free(file_data);
				return -1;
			}
			
			printf("writing new extent\n");
			// write new extent block
			bfs_block_t new_extent_loc = bfs_get_free_blocks(1);
			LBAwrite(new_extent_b, 1, new_extent_loc);

			printf("writing new file data\n");
			// copy file data from old to new buffer
			if (bfs_write_extent_data(file_data, new_extent_loc)) {
				fprintf(stderr, "Unable to write file data to new extent\n");
				free(file_data);
				return -1;
			}
			printf("freeing old extents\n");
			// free old extents
			if (bfs_clear_extents(fcbArray[fd].file)) {
				fprintf(stderr, "Unable to clear old extents \n");
				free(file_data);
				return -1;
			}

			printf("assigning new extents");
			// modify inode to point to new extent block
			fcbArray[fd].file->location = new_extent_loc;
			
			// write modified directory to disk
			struct bfs_dir_entry* dir = malloc(
				fcbArray[fd].parent_dir_entry->len * bfs_vcb->block_size);

			LBAread(
				dir, 
				fcbArray[fd].parent_dir_entry->len, 
				fcbArray[fd].parent_dir_entry->location
			);
			dir[find_file(fcbArray[fd].file->name, dir)] = *fcbArray[fd].file;
			LBAwrite(
				dir,
				fcbArray[fd].parent_dir_entry->len,
				fcbArray[fd].parent_dir_entry->location
			);

			fprintf(stderr, "All entries in header used\n");
			free(file_data);
		} else {
			struct bfs_extent new_ext_leaf;
			new_ext_leaf.ext_len = extra_blocks;
			new_ext_leaf.ext_block = bfs_get_free_blocks(extra_blocks);

			extent_block[++(header[0].eh_entries)] = new_ext_leaf;

			LBAwrite(extent_block, 1, fcbArray[fd].file->location);
		}

		free(extent_block);

		// add new block numbers to array
		free(fcbArray[fd].block_arr);
		fcbArray[fd].block_arr = bfs_extent_array(fcbArray[fd].file->location);

		// if necessary, move block pointer to inital block 
		// only if writing to empty file 
		if (fcbArray[fd].current_block == 0) {
			if (next_block(&fcbArray[fd])) {
				fprintf(stderr, "Wrote extra blocks but current block is still 0\n");
				return 1;
			}
			fcbArray[fd].block_idx = 0;
		}
	}

	int bytes_available = 0;
	int bytes_delivered = 0;
	if  (fcbArray[fd].file->size < 1) {
		fcbArray[fd].file->size = 0;
		bytes_available = bfs_vcb->block_size;
	} else {
		// available bytes in buffer
		bytes_available = bfs_vcb->block_size - fcbArray[fd].buf_index;
	}

	int part1 = count;
	int part2 = 0;
	int part3 = 0;
	int blocks_written = 0;
	int num_blocks = 0;

	if (bytes_available < count) {
		int num_blocks = (count - bytes_available) / bfs_vcb->block_size;
		part1 = bytes_available;
		part2 = num_blocks * bfs_vcb->block_size;
		part3 = (count - bytes_available) - part2;
	}

	printf("part1: %d part2: %d part3: %d num_blocks: %d, bytes_available: %d, count: %d, extra_blocks: %d\n", 
		part1, part2, part3, num_blocks, bytes_available, count, extra_blocks);

	if (part1 > 0) {
		memcpy(fcbArray[fd].buf + fcbArray[fd].buf_index, buffer, part1);
		
		blocks_written = LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].current_block);
		fcbArray[fd].buf_index += part1;

		if (fcbArray[fd].buf_index >= bfs_vcb->block_size) {
			if (next_block(&fcbArray[fd])) {
				printf("current block: %d, block_index: %d\n", fcbArray[fd].current_block, fcbArray[fd].block_idx);
				fprintf(stderr, "next_block in part1 failed\n");
				return 1;
			}
			fcbArray[fd].buf_index = 0;
		}
	}

	if (part2 > 0) {
		int block_written = 0;
		for(int i = 0; i < num_blocks; i++) {
			blocks_written += LBAwrite(buffer + part1 + (i * bfs_vcb->block_size), 1, fcbArray[fd].current_block);
			if (next_block(&fcbArray[fd])) {
				fprintf(stderr, "next_block in part2 failed\n");
				return 1;
			}
		}
		part2 = blocks_written * bfs_vcb->block_size;
	}

	if (part3 > 0) {
		// reading past end of buffer
		// buffer + part1 > 512? 
		memcpy(fcbArray[fd].buf + fcbArray[fd].buf_index, buffer + part1 + part2, part3);
		blocks_written = LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].current_block);
		if (next_block(&fcbArray[fd])) {
			fprintf(stderr, "next_block in part3 failed\n");
			return 1;
		}

		fcbArray[fd].buf_index += part3;
	}

	if (fcbArray[fd].buf_index >= bfs_vcb->block_size) {
		if(next_block(&fcbArray[fd])) {
			return 1;
		}
		fcbArray[fd].buf_index = 0;
	}

	time_t t = time(NULL);
	fcbArray[fd].file->date_accessed = t;
	fcbArray[fd].file->date_modified = t;
	bytes_delivered = part1 + part2 + part3;
	fcbArray[fd].file->size += bytes_delivered;

	struct bfs_dir_entry* directory = malloc(fcbArray[fd].parent_dir_entry->len * bfs_vcb->block_size);
	LBAread(directory, fcbArray[fd].parent_dir_entry->len, fcbArray[fd].parent_dir_entry->location);
	int idx = find_file(fcbArray[fd].file->name, directory);
	directory[idx] = *fcbArray[fd].file;
	LBAwrite(directory, fcbArray[fd].parent_dir_entry->len, fcbArray[fd].parent_dir_entry->location);

	free(directory);
	return bytes_delivered;
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

	// check if file is size 0
	if (fcbArray[fd].file->size == 0) {
		buffer[0] = '\0';
		return 0;
	}

	int bytes_read = fcbArray[fd].block_idx * bfs_vcb->block_size + 
		fcbArray[fd].buf_index;

	printf("bytes_read: %d block_idx: %d buf_index: %d\n", bytes_read, fcbArray[fd].block_idx, fcbArray[fd].buf_index);
	if (bytes_read >= fcbArray[fd].file->size) {
		fprintf(stderr, "Requested more bytes (%d) than file size (%ld)\n", 
		bytes_read, fcbArray[fd].file->size);
		return -1;
	}

	int bytes_available = fcbArray[fd].buf_size - fcbArray[fd].buf_index;
	int bytes_written = (fcbArray[fd].block_idx * bfs_vcb->block_size) -
		(bfs_vcb->block_size - bytes_available);

	if ((count + bytes_written) > fcbArray[fd].file->size) {
		count = fcbArray[fd].file->size - bytes_written;
		if (count < 0) {
			fprintf(stderr, "Negative count with %d at block %d", 
		   		bytes_written, fcbArray[fd].current_block);
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
	printf("part1: %d part2: %d part3: %d num_blocks: %d, bytes_available: %d, count: %d\n", 
		part1, part2, part3, num_blocks, bytes_available, count);

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

		memcpy(buffer + part1 + part2, fcbArray[fd].buf + fcbArray[fd].buf_index, part3);
		fcbArray[fd].buf_index += part3;
	}

	fcbArray[fd].file->date_accessed = time(NULL);
	return part1 + part2 + part3;
}

// Interface to Close the file	
int b_close(b_io_fd fd)
{
	// There shouldn't be any content in the buffer, but
	// we'll let the user know just in case.
	if (!(fcbArray[fd].access_mode & O_RDONLY) && fcbArray[fd].buf_index > 0) {
		if (fcbArray[fd].current_block == 0) {
			fprintf(stderr, "Error: Current block is 0\n");
		} else {
			LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].current_block);
		}
	}

	// Free the fd from memory
	free(fcbArray[fd].buf);
	free(fcbArray[fd].file);
	free(fcbArray[fd].block_arr);
	free(fcbArray[fd].parent_dir_entry);
	fcbArray[fd].buf = NULL;
	fcbArray[fd].file = NULL;
	fcbArray[fd].block_arr = NULL;
	fcbArray[fd].parent_dir_entry = NULL;
	return 0;
}

int b_move(char *dest, char* src) 
{
	// Check source exists
	struct bfs_dir_entry source_de;
	if (get_file_from_path(&source_de, src)) {
		fprintf(stderr, "b_move failed: %s does not exist", src);
		return 1;
	}

	// Get destination's directory
	char* dest_parent_path;
	char* dest_filename;
	get_parent_directory_and_filename(dest, &dest_parent_path, &dest_filename);

	struct bfs_dir_entry dest_entry;
	if (get_file_from_path(&dest_entry, dest_parent_path)) {
		fprintf(stderr, "Unable to get parent dir from path %s\n", dest_parent_path);
		free(dest_parent_path);
		free(dest_filename);
		return 1;
	}
	struct bfs_dir_entry* dest_directory = malloc(dest_entry.size);
	LBAread(dest_directory, dest_entry.len, dest_entry.location);

	// Get the src directory
	char* src_parent_path;
	char* src_filename;
	get_parent_directory_and_filename(src, &src_parent_path, &src_filename);

	struct bfs_dir_entry src_entry;
	if (get_file_from_path(&src_entry, src_parent_path)) {
		fprintf(stderr, "Unable to get parent dir from path %s\n", src_parent_path);
		free(src_parent_path);
		free(src_filename);
		return 1;
	}
	struct bfs_dir_entry* src_directory = malloc(src_entry.size);
	LBAread(src_directory, src_entry.len, src_entry.location);

	// Handle when the location of dest and src are the same
	if (dest_directory->location == src_directory->location) {
		// If destination file already exist, delete pre-existing file
		if (find_file(dest_filename, dest_directory) != -1) {
			fs_delete(dest_filename);
		}

		// Rename source file to the destination filename
		strcpy(src_entry.name, dest_filename);
		LBAwrite(dest_directory, dest_entry.len, dest_entry.location);

		// Free everything
		free(dest_directory);
		free(dest_parent_path);
		free(dest_filename);

		free(src_directory);
		free(src_parent_path);
		free(src_filename);
		return 0;
	}

	int i = 2;
	struct bfs_dir_entry d = dest_directory[i];
	while (d.name[0] != '\0') {
		d = dest_directory[++i];
	}

	dest_directory[i] = src_entry;
	dest_directory[++i].name[0] = '\0';
	LBAwrite(dest_directory, dest_entry.len, dest_entry.location);

	// Free everything
	free(dest_directory);
	free(dest_parent_path);
	free(dest_filename);

	free(src_directory);
	free(src_parent_path);
	free(src_filename);

	return 0 ;
}
