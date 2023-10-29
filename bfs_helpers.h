/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: bfs_helpers.h
 *
 * Description: Function definitions for bfs_helpers.
 **************************************************************/
#ifndef BFS_HELPERS
#define BFS_HELPERS

#include "mfs.h"

int bytes_to_blocks(int bytes, int block_size);

void print_dir_entry(struct bfs_dir_entry *bde);

#endif
