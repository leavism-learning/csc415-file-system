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

// recursive function to free extent memory
int free_extents(uint8_t* buffer) {
	struct bfs_extent_header header = ((struct bfs_extent_header*) buffer)[0];

	// if depth is 0, then the next section is a leaf node which points
	// to blocks that should be cleared
	if (header.eh_depth == 0) {
		for (int i = 0; i < header.eh_entries; i++) {
			// offset by 1 to skip header
			struct bfs_extent leaf = ((struct bfs_extent*) buffer)[1 + i];  
			bfs_clear_blocks(leaf.ext_block, leaf.ext_len);
			return 0;
		}
	}
	else {
		for (int i = 0; i < header.eh_entries; i++) {
			struct bfs_extent_idx idx = ((struct bfs_extent_idx*) buffer)[1];
			uint8_t* newbuf = malloc(bfs_vcb->block_size);

			if (LBAread(newbuf, 1, idx.idx_leaf) != 1) {
				fprintf(stderr, "unable to read extent block %ld\n", idx.idx_leaf);
				free(newbuf);
				return 1;
			}

			free_extents(newbuf);
			free(newbuf);
		}
	}
	return 0;
}

int fs_delete(char* filename) {
	struct bfs_dir_entry file;
	if (get_file_from_path(&file, filename)) {
		fprintf(stderr, "Unable to find file %s\n", filename);
		return 1;
	}

	if (file.file_type == 0) {
		fprintf(stderr, "Error: %s is a directory\n", filename);
		return 1;
	}

	// read extents to find which blocks must be freed
	uint8_t* buffer = malloc(bfs_vcb->block_size);

	if (LBAread(buffer, 1, file.location) != 1) {
		fprintf(stderr, "Unable to LBAread block %ld in fs_delete\n", file.location);
		free(buffer);
		return 1;
	}

	if (free_extents(buffer)) {
		free(buffer);
		return 1;
	}

	free(buffer);
	buffer = NULL;

	// TODO remove directory entry 
	
	return 0;
}

// return 1 if failed to get file from path
int fs_stat(const char* path, struct fs_stat* buf) 
{
	// TODO: test to make sure this works
	struct bfs_dir_entry dir_entry;
	// TODO: ask about const char
	const char* copy_path = strdup(path); 
	if (!get_file_from_path(&dir_entry, (char*)copy_path))
	{
		return 1;
	}
	buf->st_blksize = bfs_vcb->block_size;
	buf->st_accesstime = dir_entry.date_accessed;
	buf->st_size = dir_entry.size;
	buf->st_createtime = dir_entry.date_created;
	buf->st_modtime = dir_entry.date_modified;
	buf->st_blocks = bytes_to_blocks(dir_entry.size);
	return 0;
}
