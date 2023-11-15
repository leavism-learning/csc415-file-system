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
#include "mfs.h"

/*
 * Create a directory entry with given name & size
 */
int bfs_create_direntry(struct bfs_dir_entry* dir_entry, char* name, uint64_t size, uint64_t pos, uint8_t type)
{
	dir_entry->size = size;
	dir_entry->len = bytes_to_blocks(size);
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
 * Given a buffer, create extent table for that buffer
 */
int bfs_create_extent(void* extent_block, int size)
{
	if (extent_block == NULL) {
		extent_block = malloc(bfs_vcb->block_size);
	} else {
		extent_block = realloc(extent_block, bfs_vcb->block_size);
	}

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
		if (loc == -1) {
			ext_len /= 2;
			if (ext_len < 1) {
				return -1;
			}
		}
		else {
			struct bfs_extent ext;
			ext.ext_block = loc;
			ext.ext_len = ext_len + is_odd;
			ext_leaves[num_exts++] = ext;
			blocks_needed -= ext_len;
			is_odd = 0;
		} 	
	}

	// number of leaf or index nodes that can fit in a block
	// if # of leaf nodes > nodes per block, index nodes are needed
	int nodes_per_block = bfs_vcb->block_size / sizeof(struct bfs_extent) - 1;

	if (num_exts > nodes_per_block) {
		fprintf(stderr, "Error: More than %d extents needed\n", nodes_per_block);
		return -1;
	}

	struct bfs_extent_header header;
	header.eh_depth = 0;
	header.eh_entries = num_exts;
	header.eh_max = nodes_per_block;

	((struct bfs_extent_header*) extent_block)[0] = header;

	for (int i = 0; i < num_exts; i++) {
		((struct bfs_extent*) extent_block)[i+1] =  ext_leaves[i];
	}

	return 0;
}

int bfs_read_extent(void* data, bfs_block_t block_num) 
{
	struct bfs_extent_header* extent = malloc(bfs_vcb->block_size);
	if (LBAread(extent, 1, block_num) != 1) {
		fprintf(stderr, "Unable to LBAread extent at block %ld\n", block_num);
		return 1;
	}

	struct bfs_extent_header header = extent[0];

	if (header.eh_depth != 0) {
		fprintf(stderr, "Error: Extent uses unimplemented indexes\n");
		return 1;
	}

	struct bfs_extent* ext_leaves = (struct bfs_extent*) extent;

	// find out how many blocks are needed for extents
	int data_len = 0;
	for (int i = 0; i < header.eh_entries; i++) {
		struct bfs_extent leaf = ext_leaves[i];
		data_len += leaf.ext_len;
	}

	if (data == NULL) {
		data = malloc(data_len * bfs_vcb->block_size);
	} else {
		data = realloc(data, data_len * bfs_vcb->block_size);
	}

	// read data 
	int pos = 0;
	for (int i = 0; i < header.eh_entries; i++) {
		struct bfs_extent leaf = ext_leaves[i];
		if (LBAread(data + pos, leaf.ext_len, leaf.ext_block) != leaf.ext_len) {
			fprintf(stderr, "Unable to read data blocks %ld\n", leaf.ext_block);
		}
		pos += leaf.ext_len;
	}

	return 0;
}
