/**************************************************************
 * Class:  CSC-415-01 Fall 2021
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: bfs_directory.c
 *
 * Description: Directory functions for Basic File System (BFS).
 *
 **************************************************************/

#include "bfs.h"

/*
* Create a directory at the specified position
*/
int bfs_create_directory(bfs_block_t pos, bfs_block_t parent)
{
	struct bfs_dir_entry* buffer = malloc(bfs_vcb->block_size);
	if (buffer == NULL) {
		perror("malloc in bfs_create_directory");
		return 1;
	}

	if (LBAread(buffer, 1, pos) != 1) {
		fprintf(stderr, "Unable to LBAread pos %ld in bfs_create_dir\n", pos);
		return 1;
	}
	
	// create . and .. directory entries
	struct bfs_dir_entry here;
	bfs_create_file(&here, ".", bfs_vcb->block_size, pos, 0);
	struct bfs_dir_entry parend_dir;
	bfs_create_file(&parend_dir, "..", bfs_vcb->block_size, pos, 0);

	buffer[0] = here;
	buffer[1] = parend_dir;

	if (LBAwrite(buffer, 1, pos) != 1) {
		fprintf(stderr, "Unable to LBAwrite pos %ld in bfs_create_dir\n", pos);
	}

	return 0;
}

// TODO: Possibly refactor with bfs_init_directory() so we don't
// repeat code.
int bfs_create_root(struct bfs_dir_entry *buffer, int lba_position)
{
	// first, create the root directory.
	// then, create the . and .. directory entries

	// create here (.)
	struct bfs_dir_entry here;
	bfs_create_here(&here, lba_position);

	// parent (..) is same as . for root
	struct bfs_dir_entry parent;
	bfs_create_here(&parent, lba_position);
	strcpy(parent.name, "..");

	buffer[0] = here;
	buffer[1] = parent;

	return 0;
}

// TODO: Possibly refactor with bfs_init_directory() so we don't
// repeat code.
int bfs_init_directory() { return 0; }

void bfs_create_here(struct bfs_dir_entry *here, int lba_position)
{
	here->size = bfs_vcb->block_size;
	here->location = lba_position;
	strcpy(here->name, ".");
	here->file_type = 0;
	time_t current_time = time(NULL);
	here->date_created = current_time;
	here->date_modified = current_time;
	here->date_accessed = current_time;
}
