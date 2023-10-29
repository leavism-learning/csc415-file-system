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

/**************************************************************
 * bfs_bitmap.c
 * Function definition for working with BFS bitmaps.
 **************************************************************/

/**
 * @brief Check the status of the bit at `position` in `byte`.
 *
 * @param byte: The byte in which the bit needs to be checked.
 * @param position: The position (0-7) of the bit to be checked within the
 * byte.
 * @return The value at `position`, which is either clear (0) or set (1).
 *
 */
int bit_check(uint8_t byte, uint8_t position);

/**
 * @brief Set the value of a bit (to 1) in `byte` at `position`.
 *
 * @param byte: The byte in which the bit needs to be set.
 * @param position: Position (0-7) of the bit to be set within the byte.
 * @return The new byte after setting the bit at `position`.
 */
uint8_t bit_set(uint8_t byte, uint8_t position);

/**
 * @brief Clear the value of a bit (to 0) in `byte` at `position`.
 *
 * @param byte: The byte in which the bit needs to be cleared.
 * @param position: Position (0-7) of the bit to be cleared within the byte.
 * @return The new byte after clearing the bit at `position`.
 */
uint8_t bit_clear(uint8_t byte, uint8_t position);

/**
 * @brief Toggle the value of a bit at the given position.
 *
 * @param byte: The byte in which the bit needs to be toggled.
 * @param position: Position (0-7) of the bit to be toggled within the byte.
 * @return The new byte after toggling the bit at `position`.
 */
uint8_t bit_toggle(uint8_t byte, uint8_t position);

/**
 * @brief Gets the first empty block in a bitmap.
 *
 * @param bitmap: Pointer to the block bitmap.
 * @param size: Size of the bitmap in bytes.
 * @return The position of the first empty block in the bitmap or -1 if no
 * empty block is found.
 */
int get_empty_block(uint8_t *bitmap, int size);

/**
 * @brief Fetch the block number of the first available block, mark it as
 * used, or return -1 if none found.
 *
 * The function traverses through block groups, checks the associated
 * bitmaps, and identifies the first block that is free. Once found, it
 * marks that block as used and returns its block number.
 *
 * @return Block number of the first available block, or -1 if no block is
 * available.
 * @note Assumes `bfs_vcb` and `bfs_gdt` are valid and initialized.
 */
int bfs_get_free_block();

/**
 * @brief Set the value of a bit at the given position in a block.
 *
 * @param block: Pointer to the block where the bit needs to be set.
 * @param position: Overall position of the bit to be set within the block.
 * @return 0 on success, -1 on failure (e.g., position out of bounds).
 */
int block_bit_set(uint8_t *block, uint8_t position);

/**************************************************************
 * bfs_directory.c
 * Function definitions for working with BFS directory entries.
 **************************************************************/

/**
 * @brief Creates the root directory for the BFS file system.
 *
 * @param buffer: Pointer to a buffer where the directory entries will be stored.
 * @param lba_position: Logical Block Address position where the root directory resides.
 *
 * @return Returns 0 on successful creation of the root directory.
 */
int bfs_create_root(struct bfs_dir_entry *buffer, int lba_position);
/**
 * @brief Creates a directory for the BFS file system.
 *
 * @return Returns 0 on successful creation of the directory.
 */
int bfs_init_directory();

/**
 * @brief Creates the `.` current directory for the BFS file system.
 *
 * @param here: Pointer to the `.` current directory that is being created
 * @param lba_position: Logical Block Address position of the `.` current directory.
 *
 * @return Returns 0 on successful creation of the current directory.
 */
void bfs_create_here(struct bfs_dir_entry *here, int lba_position);

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
 * Initialize a directory entry. Returns 0 on success, non-zero on failure
 */
int create_dir_entry(struct bfs_dir_entry *dentry, char *name, int size,
                     int type);

/*
 * Generate a random UUID. Returns 0 on success, non-zero on failure
 */
void bfs_generate_uuid(uint8_t *uuid);

#endif
