/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: bfs_bitmap.c
 *
 * Description: A set of helper functions when working with the
 * Basic File System (BFS).
 **************************************************************/

#include "bfs_helpers.h"

int bytes_to_blocks(int bytes, int block_size)
{
	return (bytes + block_size - 1) / block_size;
}

void print_dir_entry(struct bfs_dir_entry *bde)
{
	printf("name: %s  size: %ld  location: %ld  type: %d\n", bde->name, bde->size,
				 bde->location, bde->file_type);
}
