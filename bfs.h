/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: bfs.h
 *
 * Description: Function definition for bfs.
 **************************************************************/
#ifndef BFS_H_
#define BFS_H_

#include "mfs.h"

#define BFS_MAGIC 0x4465657A

/********************************************************************
 * bfs_bitmap.c
 * Functions for working with BFS bitmaps.
 */

int bit_check(uint8_t byte, uint8_t pos);

int bit_set(uint8_t byte, uint8_t pos);

int bit_clear(uint8_t byte, uint8_t pos);

int bit_toggle(uint8_t byte, uint8_t pos);

int get_empty_block(uint8_t *bitmap, int size);

int bfs_get_free_block();

// set vaule of a bit at given position in block
void block_bit_set(uint8_t *block, uint8_t pos);

/********************************************************************
 * bfs_directory.c
 * Function definitions for bfs_directory.
 */

int bfs_create_root(struct bfs_dir_entry *buffer, int lba_pos);

int bfs_init_directory();

void bfs_create_here(struct bfs_dir_entry *here, int lba_pos);

/********************************************************************
 * bfs_file.c
 * Functions for creating & modifying BFS files
 */

/********************************************************************
 * bfs_helpers.c
 * Helper funcions for Basic File System
 */

int bytes_to_blocks(int bytes, int block_size);

void print_dir_entry(struct bfs_dir_entry *bde);

/*******************************************************************
 * bfs_init.c
 * Functions for initializing the bfs system
 */

// Creates a volume with the given name. Returns 0 on success, non-zero on
// failure
int bfs_vcb_init(char *name, uint64_t num_blocks, uint64_t block_size);

/*
 * Create the group descriptor table
 */
int bfs_gdt_init(struct block_group_desc *gdt);

/*
 * Create a UUID
 */
void bfs_generate_uuid(uint8_t *uuid);

#endif
