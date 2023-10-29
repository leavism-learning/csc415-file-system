/*
 * BFS helper functions
 * Author: Griffin Evans
 */
#ifndef BFS_HELPERS
#define BFS_HELPERS

#include "mfs.h"

int bytes_to_blocks(int bytes, int block_size);

void print_dir_entry(struct bfs_dir_entry* bde);

#endif
