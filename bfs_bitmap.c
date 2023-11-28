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
 * Description: Functions to manipulate and query
 * the bitmap of the Basic File System (BFS).
 **************************************************************/

#include "bfs.h"

int bit_check(uint8_t byte, uint8_t position)
{
	return byte & (1 << position);
}

// set bit in give byte
void bit_set(uint8_t* byte, uint8_t position)
{
	*byte |= (1 << position);
}

int block_bit_set(uint8_t *bitmap, uint8_t position)
{
	if (position >= bfs_vcb->block_size || position < 0) {
		fprintf(stderr, "Error: position is out of bounds.\n");
		return -1;
	}

	bit_set(&bitmap[position / 8], position % 8);

	return 0;
}

void bit_clear(uint8_t* byte, uint8_t position)
{
	*byte &= ~(1 << position);
}

void bit_toggle(uint8_t* byte, uint8_t position)
{
	*byte ^= 1 << position;
}

void bitmap_clear_n(uint8_t* bitmap, uint32_t start, uint32_t count)
{
	for (int i = start; i < count; i++) {
		bit_clear(&bitmap[i / 8], i % 8);
	}
}

int bitmap_find_avail_bit(uint8_t* bitmap)
{
	for (int byte_index = 0; byte_index < bfs_vcb->block_size; byte_index++) {
		for (int bit_index = 0; bit_index < 8; bit_index++) {
			if (bit_check(bitmap[byte_index], bit_index) == 0) {
				return (byte_index * 8) + bit_index;
			}
		}
	}
	return -1;
}

// return location of num_blocks consecutive blocks in given bitmap
int bitmap_find_avail_bits(uint8_t* bitmap, int bitmap_size, int num_blocks)
{
	int head = -1; // start location of consecutive block list
	int count = 0; // size count of consecutive block list

	for (int byte_index = 0; byte_index < bitmap_size; byte_index++) {
		for (int bit_index = 0; bit_index < 8; bit_index++) {
			// if bit is 0,
			if (bit_check(bitmap[byte_index], bit_index) == 0) {
				count++;
			if (head < 0) 
					head = (byte_index * 8) + bit_index;

				if (count == num_blocks) {
					return head;
				}
			} else {
				head = -1;
				count = 0;
			}
		}
	}
	return -1;
}

int bfs_set_block(bfs_block_t block_num)
{
	// Find which block group the block belongs to
	int block_group = block_num / bfs_vcb->block_group_size;
	uint8_t *bitmap = malloc(bfs_vcb->block_size);

	if (LBAread(bitmap, 1, bfs_gdt[block_group].bitmap_location) != 1) {
		fprintf(stderr, "Error: Unable to read bitmap for block %ld\n", block_num);
		free(bitmap);
		return 1;
	}

	// Index in block group is block number - ( group size * group number )
	int block_index = block_num - (bfs_vcb->block_group_size * block_group);
	if (block_bit_set(bitmap, block_index) != 0) {
		fprintf(stderr, "Error: Unabled to set bit in block\n");
		free(bitmap);
		return 1;
	}

	if (LBAwrite(bitmap, 1, bfs_gdt[block_group].bitmap_location != -1)) {
		fprintf(stderr, "Error: Unable to write to block %ld\n", block_num);
		free(bitmap);
		return 1;
	}

	free(bitmap);
	return 0;
}

// returns first block of num_blocks consecutive free blocks and marks them as used
int bfs_get_free_blocks(uint32_t num_blocks)
{
	for (int i = 0; i < bfs_vcb->block_group_count; i++) {

		struct block_group_desc* block_group = &bfs_gdt[i];

		// if there are enough free blocks in the block group, read that block group's
		// bitmap and find the first available block
		if (block_group->free_blocks_count >= num_blocks) {

			uint8_t* bitmap = malloc(bfs_vcb->block_size);
			if (bitmap == NULL) {
				perror("malloc");
				return -1;
			}

			if (LBAread(bitmap, 1, block_group->bitmap_location) != 1) {
				fprintf(stderr, "Unable to LBAread bitmap\n");
				free(bitmap);
				return -1;
			}

			// block index in that block group
			int b_idx = bitmap_find_avail_bits(bitmap, bfs_vcb->block_size, num_blocks);

			// set blocks as used
			for (int i = 0; i < num_blocks; i++) {
				block_bit_set(bitmap, b_idx + i);
			}
			block_group->free_blocks_count -= num_blocks;

			if (LBAwrite(bitmap, 1, block_group->bitmap_location) != 1) {
				fprintf(stderr, "Unable to LBAwrite bitmap\n");
				free(bitmap);
				return -1;
			}

			free(bitmap);

			return idx_to_bnum(b_idx, i);
		}
	}
	return -1;
}

// free n consecutive blocks starting from block_num
int bfs_clear_blocks(bfs_block_t start, uint32_t count)
{
	int block_group = start / bfs_vcb->block_group_size;
	uint8_t* bitmap = malloc(bfs_vcb->block_size);

	if (LBAread(bitmap, 1, block_group) != 1) {
		fprintf(stderr, "Unable to read %d in bfs_clear_blocks", block_group);
		free(bitmap);
		bitmap = NULL;
		return 1;
	}

	bitmap_clear_n(bitmap, start, count);

	if (LBAwrite(bitmap, 1, block_group) != 1) {
		fprintf(stderr, "Unable to write %d in bfs_clear_blocks", block_group);
		free(bitmap);
		bitmap = NULL;
		return 1;
	}

	free(bitmap);
	bitmap = NULL;
	return 0;
}

int bfs_get_free_block()
{
	for (int i = 0; i < bfs_vcb->block_group_count; i++) {

		struct block_group_desc* block_group = &bfs_gdt[i];

		// if there are free blocks in the block group, read that block group's
		// bitmap and find the first available block
		if (block_group->free_blocks_count > 0) {

			uint8_t *bitmap = malloc(bfs_vcb->block_size);
			if (bitmap == NULL) {
			perror("malloc");
				return -1;
			}

			if(LBAread(bitmap, 1, block_group->bitmap_location) != 1) {
				free(bitmap);
				return -1;
			}

			// block index in that block group
			int b_idx = bitmap_find_avail_bit(bitmap);

			// set bit as used
			block_bit_set(bitmap, b_idx);
			block_group->free_blocks_count--;

			if(LBAwrite(bitmap, 1, block_group->bitmap_location) != 1) {
				free(bitmap);
				return -1;
			}

			free(bitmap);

			return idx_to_bnum(b_idx, i);
		}
	}

	return -1;
}

int idx_to_bnum(int index, int block_group) 
{
	return index + bfs_gdt[block_group].bitmap_location;
}

//int gdt_free_blocks()
