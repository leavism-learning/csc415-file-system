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
 * Description: Function definition for the Basic File System (BFS).
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
 */
void bit_set(uint8_t* byte, uint8_t position);

/**
 * @brief Clear the value of a bit (to 0) in `byte` at `position`.
 *
 * @param byte: The byte in which the bit needs to be cleared.
 * @param position: Position (0-7) of the bit to be cleared within the byte.
 */
void bit_clear(uint8_t* byte, uint8_t position);

/**
 * @brief Toggle the value of a bit at the given position.
 *
 * @param byte: The byte in which the bit needs to be toggled.
 * @param position: Position (0-7) of the bit to be toggled within the byte.
 */
void bit_toggle(uint8_t* byte, uint8_t position);

/**
 * @brief Return index of first available bit in bitmap
 *
 * @param bitmap: Pointer to the block bitmap.
 * @param size: Size of the bitmap in bytes.
 * @return The position of the first empty block in the bitmap or -1 if no
 * empty block is found.
 */
int bitmap_find_avail_bit(uint8_t* bitmap);

/**
 * @brief finds num_blocks consecutive blocks in the bitmap
 *
 * @param bitmap: Pointer to the block bitmap.
 * @param bitmap_size: size of the bitmap
 * @param num_blocks: number of consecutive blocks
 * 
 * @return The start of the location of consecutive block list
*/
int bitmap_find_avail_bits(uint8_t* bitmap, int bitmap_size, int num_blocks);

/**
 * @brief For changing a particular block as used
 * @param block_num: Block number to set as used
 * @return 1 if error, 0 if success
*/
int bfs_set_block(bfs_block_t block_num);

/**
 * @brief Return the block number of the first available block & mark it as
 * used, or return -1 if none found.
 *
 * The function traverses through block groups, checks the associated
 * bitmaps, and identifies the first block that is free. Once found, it
 * marks that block as used and returns its block number.
 *
 * @return Block number of the first available block, or -1 on error  
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

/**
 * @brief Return absolute block number from index in block group bitmap
 *
 * @param index: location of block in block group
 * @param block_group: number of block group in GDT
 * @return logical block array number 
 */
int idx_to_bnum(int index, int block_group);

/**
 * @brief Free n consecutive blocks starting from block_num.
 *
 * @param start The block number from which to start freeing consecutive blocks.
 * @param count The number of consecutive blocks to be freed.
 *
 * @return 0 on success, -1 on failure.
 */
int bfs_clear_blocks(bfs_block_t start, uint32_t count);

/**
 * @brief Clear a range of bits in a bitmap.
 *
 * @param bitmap A pointer to the bitmap to be modified.
 * @param start The starting bit index to begin clearing (inclusive).
 * @param count The number of bits to clear in the range.
 */
void bitmap_clear_n(uint8_t* bitmap, uint32_t start, uint32_t count);

/**
 * @brief Get a free block from the bfs.
 *
 * @param num_blocks The number of blocks to get.
 *
 * @return The block number of the allocated block if successful, or -1
 * if an error occurs during the allocation process or no free blocks are
 * available.
 */
int bfs_get_free_blocks(uint32_t num_blocks);

/**************************************************************
 * bfs_directory.c
 * Function definitions for working with BFS directory entries.
 **************************************************************/

/**
 * Retrieves a directory entry for a specified file or directory from a given path.
 * 
 * @param target A pointer to a bfs_dir_entry structure where the found directory 
 *               entry will be copied. The memory for this structure should be 
 *               allocated by the caller.
 *
 * @param path Path to target file. The path can be absolute or relative.
 *
 * @return Returns 0 on successful retrieval of the directory entry. Returns 1 
 *         if an error occurs, such as if the path is invalid, if any part of the 
 *         path does not exist in the file system, or if memory allocation fails.
 */
int get_file_from_path(struct bfs_dir_entry* target, const char* path);

/**
 * @brief Find a file with a given name in a specified directory within the BFS.
 *
 * This function searches for a file with the specified name within a given directory in the BFS.
 *
 * @param filename The name of the file to search for.
 * @param directory A pointer to the directory structure (array of directory entries) to search in.
 *
 * @return The index of the found file in the directory (0 based index). If failed, it will return -1.
 */
int find_file(char* filename, struct bfs_dir_entry* directory);

/**
 * @brief Expands a pathname represented as a C string.
 *
 * @param in The input pathname to be expanded.
 * @return A dynamically allocated C string representing the expanded absolute pathname.
 *         The caller is responsible for freeing the memory allocated for the result using `free()`.
 *         If the input is invalid (empty or NULL), this function returns NULL.
 *
 * @note This function allocates memory for the result, and it's the caller's responsibility to free that memory.
 * @note The returned pathname is absolute, starting either from the root directory or the current working directory.
 */
char* expand_pathname(const char* in);

/**
 * @brief Recursively removes a directory and its contents.
 *
 * This function is responsible for removing a directory and all its subdirectories
 * and files within BFS. It recursively traverses the directory structure and clears
 * the associated data blocks and directory entries.
 *
 * @param entry A pointer to the directory entry to be removed.
 * @return 0 if the directory and its contents are successfully removed, 1 otherwise.
 *
 * @note This won't delete the root directory.
 */
int bfs_remove_dir(struct bfs_dir_entry* entry);

/**************************************************************
 * bfs_file.c
 * Functions for creating & modifying BFS files.
 **************************************************************/

/**
 * @brief Create a directory entry with the given name, size, and attributes.
 *
 * This function is responsible for initializing a directory entry structure with the specified
 * name, size, position, file type, and timestamps.
 *
 * @param dir_entry A pointer to the directory entry structure to be initialized.
 * @param name The name of the directory entry.
 * @param size The size of the directory entry (in bytes).
 * @param pos The position (block number) of the directory entry.
 * @param type The type of the directory entry (e.g., file or directory).
 *
 * @return 0 on success.
 */
int bfs_create_dir_entry(struct bfs_dir_entry* dir_entry, char* name, uint64_t size, uint64_t pos, uint8_t type);

/**
 * @brief Create an extent table for a given buffer in the bfs.
 *
 * @param extent_block A pointer to the buffer where the extent table will be created or updated.
 *                    If NULL, a new buffer is allocated; otherwise, the existing buffer is resized.
 * @param size The size of the buffer (in bytes) for which the extent table is being created.
 *
 * @return 0 on success. 1 on failure.
 */
int bfs_create_extent(void* extent_block, int size);

/**
 * @brief Read data from an extent in the bfs and store it in a buffer.
 *
 * @param data A pointer to the buffer where the data will be stored. If NULL, a new buffer is allocated.
 * @param block_num The block number of the extent to be read.
 *
 * @return 0 on success. 1 on failure.
 */
int bfs_read_extent(void* buffer, bfs_block_t block_num);

/**
 * @brief Clears extents associated with a BFS directory entry.
 *
 * @param entry A pointer to the BFS directory entry for which extents need to be cleared.
 *
 * @return 0 on success, 1 on failure (e.g., if unable to read or free extents).
 */
int bfs_clear_extents(struct bfs_dir_entry* entry);

/**
 * @brief Retrieves an array of block numbers representing extents in a BFS file.
 *
 * @param block_num The block number containing the extent information.
 *
 * @return A dynamically allocated array of block numbers representing extents on success,
 *         NULL on failure (e.g., if unable to read the extent information).
 */
bfs_block_t* bfs_extent_array(bfs_block_t block_num);

/**
 * @brief Deletes a file from a BFS directory.
 *
 * @param directory A pointer to the BFS directory entry from which the file should be deleted.
 * @param entry A pointer to the BFS directory entry representing the file to be deleted.
 *
 * @return 0 on success, 1 on failure (e.g., if the file is not found, or extents cannot be cleared).
 */
int bfs_delete_file(struct bfs_dir_entry* directory, struct bfs_dir_entry* entry);

/**************************************************************
 * bfs_helpers.c
 * Helper funcions for Basic File System.
 **************************************************************/

/**
 * @brief Calculate number of blocks required to fit `bytes`.
 *
 * This function does ceiling division for `bytes` by `block_size`
 * such that there are enough blocks to account for any all of
 * `bytes`.
 *
 * @param bytes: The number of bytes that needs to fit in blocks.
 *
 * @return Returns the number of blocks required to accommodate the
 * specified byte count.
 */
int bytes_to_blocks(int bytes);

/**
 * @brief Prints the details of a BFS directory entry.
 *
 * @param dentry: Pointer to the directory entry to be printed.
 */
void print_dir_entry(struct bfs_dir_entry* dentry);

/**
 * @brief Writes the current Volume Control Block (VCB) to disk.
 *
 * @return 0 on success, 1 on failure (e.g., if unable to write the VCB to disk).
 */
int write_current_vcb();

/**
 * @brief Writes the current Group Descriptor Table (GDT) to disk.
 *
 * @return 0 on success, 1 on failure.
 */
int write_current_gdt();

/**
 * @brief Reloads the CWD from disk into the bfs_cwd global variable.
 */
void reload_cwd();

/**
 * @brief Extracts the parent directory and filename from a given path.
 *
 * This function takes a path as input and extracts the parent directory and filename
 * components. It is a helper function used in file and directory operations to separate
 * the path into its constituent parts.
 *
 * @param path The input path from which the parent directory and filename should be extracted.
 * @param parent_directory A pointer to a char pointer that will store the extracted parent directory.
 * @param filename A pointer to a char pointer that will store the extracted filename.
 *
 * @return 0 on success.
 */
int get_parent_directory_and_filename(const char* path, char** parent_directory, char** filename);

/**
 * Extracts and returns the last component of a given file path.
 *
 * @param pathname The file path as a null-terminated string. 
 *                 It should not be NULL or an empty string.
 *                 
 * @return A dynamically allocated string containing the last component 
 *         of the path.
 *         If the input is NULL or empty, returns an empty string.
 *         If no directory separator ('/') is present, returns a copy 
 *         of the entire path.
 *         
 * @note The caller must ensure to free this memory when it is no longer needed.
 */
char* get_filename_from_path(const char* pathname);

/**************************************************************
 * bfs_init.c
 * Functions for initializing the Basic File System.
 **************************************************************/

/**
 * @brief Initializes the Volume Control Block (VCB) for the Basic File
 * System (BFS).
 *
 * @param name: Name of the volume. Maximum length is 63 characters.
 * @param num_blocks: Total number of blocks in the BFS volume.
 * @param block_size: Size of each block in bytes.
 *
 * @return 0 on successful initialization, non-zero otherwise.
 */
int bfs_vcb_init(char *name, uint64_t num_blocks, uint64_t block_size);

/**
 * @brief Initializes the Group Descriptor Table (GDT)
 *
 * This function populates the GDT with block group descriptors. Each
 * descriptor points to a bitmap that keeps track of free blocks within the
 * block group. The block group size is inferred from the VCB and each block
 * group contains block_size * 8 blocks. The bitmap's location for each group
 * is calculated, the first block of the group (which contains the bitmap)
 * is marked as used, and the bitmap is written to disk.
 *
 * @param gdt: Buffer where the group descriptor data will be stored.
 *
 * @return 0 on successful initialization of the GDT, non-zero otherwise.
 */
int bfs_gdt_init(struct block_group_desc *gdt);

/**
 * @brief Generates a UUID (Universal Unique Identifier).
 *
 * @param uuid: Pointer to a buffer that will store the generated UUID.
 * This buffer should be at least 16 bytes in size.
 */
void bfs_generate_uuid(uint8_t *uuid);

/******************************************************************************
*
* fs_delete.c
*/
int free_extents(uint8_t* buffer);

#endif
