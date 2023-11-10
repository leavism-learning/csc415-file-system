/**************************************************************
 * Class:  CSC-415-01 Fall 2021
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: bfs_file.c
 *
 * Description: Functions for working with BFS files.
 *
 **************************************************************/

#include "bfs.h"

/*
 * Create a file with given name & size
 */
int bfs_create_file(struct bfs_dir_entry* dir_entry, char* name, uint64_t size, uint64_t pos, uint8_t type)
{
	dir_entry->size = size;
	strcpy(dir_entry->name, name);
	dir_entry->location = pos;
	time_t current_time = time(NULL);
	dir_entry->file_type = type;
	dir_entry->date_created = current_time;
	dir_entry->date_modified = current_time;
	dir_entry->date_accessed = current_time;
	return 0;
}

/*
 * Given a buffer, create extend table for that buffer
 * WIP: currently only allocated one block for each file
 */
int bfs_create_extent(struct bfs_extent_header* extent_block, int size)
{
	// find out how many extents are needed for file
	int blocks_needed = bytes_to_blocks(size);
	// try to get one as few extents as possible for num_blocks
	// write extent information to buffer
	int ext_len = blocks_needed;
	int num_exts = 0;
	int is_odd = 0;
	if (ext_len % 2 == 1) {
		is_odd = 1;
	}

	struct bfs_extent ext_leaves[ext_len];
	int pos = 0;

	while (blocks_needed > 0) {
		int loc = bfs_get_free_blocks(ext_len + is_odd);
		if (loc != -1) {
			struct bfs_extent ext;
			ext.ext_block = loc;
			ext.ext_len = ext_len + is_odd;
			extents[num_exts++] = ext;
			blocks_needed -= ext_len;
			is_odd = 0;
		} else {
			ext_len /= 2;
			if (ext_len < 1) {
				return -1;
			}
		}
	}

	int exts_per_block = (bfs_vcb->block_size / sizeof(struct bfs_extent)) - 1;
	extent_block = malloc(bfs_vcb->block_size);

	// all extents fit in one block
	if (num_exts < exts_per_block) {
		struct bfs_extent_header header;
		header.eh_entries = num_exts;
		header.eh_depth = 1;
		header.max = exts_per_block;

		// one block for the extent 
		extent_block = malloc(bfs_vcb->block_size);
		extent_block[0] = header;

		for (int i = 0; i < num_exts; i++) {
			extent_block[i + 1] = ext_leaves[i];
		}
	}


	if (ext.ext_block == -1) {
		fprintf(stderr, "Error: No free blocks found\n");
		return 1;
	}
	ext.ext_len = 1;

	int index = sizeof(struct bfs_extent_header);
	memcpy(buffer, (void *)&header, index);
	memcpy(buffer index, (void *)&ext, sizeof(bfs_extent));

	return 0;
}
