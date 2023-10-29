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
 * Description: Function definition for bfs.
 **************************************************************/
#ifndef BFS_H
#define BFS_H

#include "mfs.h"
#include "fsLow.h"
#include "bfs_helpers.h"
#include "bfs_bitmap.h"
#include "bfs_directory.h"

#define BFS_MAGIC 0x4465657A

/*
 * Creates a volume with the given name. Returns 0 on success, non-zero on failure
 */
int bfs_vcb_init(char *name, uint64_t num_blocks, uint64_t block_size);

/*
 * Create the group descriptor table
 */
int bfs_gdt_init(struct block_group_desc *gdt);

/*
 * Initialize a directory entry. Returns 0 on success, non-zero on failure
 */
int create_dir_entry(struct bfs_dir_entry *dentry, char *name, int size, int type);

/*
 * Generate a random UUID. Returns 0 on success, non-zero on failure
 */
void bfs_generate_uuid(uint8_t *uuid);

/*
 * Get value of bit at given position
 */
uint8_t get_bit_value(uint8_t byte, uint8_t pos);

int bfs_get_free_block();

int is_valid_volname(char *name);

void print_uuid(uint8_t *uuid);

#endif
