/*
 * bfs_bitmap.c 
 * functions for working with block group bitmap
 */

/*
 * Get value (0 or 1 of a bit at a certain position in the given byte
 * byte: byte to be read
 * pos: position in byte, 0-7
 */
#include <bfs_bitmap.h>

// returns 0 if bit is unset, 1 if bit is set
int bit_check(uint8_t byte, uint8_t pos)
{
	return byte & ( 1 << pos );
	//return (byte >> pos) & 0x1;
}

/*
 * Set value of bit at given position.
 *
 */
int bit_set(uint8_t byte, uint8_t pos)
{
	return byte |= 1 << pos;
	//return byte | 0x1 << pos;
}

/*
 * Set value of bit at given position in block. 
 * Assumes that pos is valid 
 */
void block_bit_set(uint8_t* block, uint8_t pos)
{
	int index = pos % 8;

	uint8_t byte = block[pos / 8];
	byte = bit_set(byte, index);

	block[pos / 8] = byte;
}

int bit_clear(uint8_t byte, uint8_t pos)
{
	return byte &= ~(1 << pos);
}

int bit_toggle(uint8_t byte, uint8_t pos)
{
	return byte ^= 1 << pos;
}

// find empty blocks in bitmap
// array: buffer to store return values (block #)
// count: number of blocks to find
// returns 0 if the operation was sucessful, otherwise 1
/*
int get_empty_blocks(uint8_t* array, int count)
{
	int blocks_found = 0;

	for (int i = 0; i < block_size * 8; i++) {
		if (i % 8) {
			for (int j = 0; j < 8; j++) {
				if (bit_check == 0)
			}
		}
	}
	return 1;
}
*/

/*
 * Return position of first empty block in bitmap, or -1
 * bitmap: block bitmap
 * size: block size
 */
int get_empty_block(uint8_t* bitmap, int size)
{
	for (int i = 0; i < size; i++) { 
		for (int j = 0; j < 8; j++) {
			if (bit_check(bitmap[i], j) == 0) {
				return (i * 8) + j;
			}
		}
	}
	return -1;
}

/*
 * Mark a block as in use. Returns 0 if sucessful, or nonzero on failure
 */
int bfs_set_block(bfs_block_t block_num)
{
	// find out which block group the block belongs to 
	int block_group = block_num / bfs_vcb->block_group_size;
	uint8_t* bitmap = malloc(bfs_vcb->block_size);

	if (LBAread(bitmap, 1, bfs_gdt[block_group].bitmap_location) != 1) {
		fprintf(stderr, "Error: Unable to read bitmap for block %ld\n", block_num);
		return 1;
	}
	
	// index in block group is is block # - ( group size * group # )
	int block_index = block_num - (bfs_vcb->block_group_size * block_group);
	block_bit_set(bitmap, block_index);
	
	if (LBAwrite(bitmap, 1, bfs_gdt[block_group].bitmap_location != -1)) {
		fprintf(stderr, "Error: Unable to write to block %ld\n", block_num);
		return 1;
	}

	free(bitmap);

	return 0;

}

/*
 * Return block num of first available block or -1 and mark that block as used
 */
int bfs_get_free_block() 
{
	for (int i = 0; i < bfs_vcb->block_group_count; i++) {

		struct block_group_desc block_group = bfs_gdt[i];
		
		// if there are free blocks in the block group, read that block group's 
		// bitmap and find the first available block
		if (block_group.free_blocks_count > 0) {

			uint8_t* bitmap = malloc(bfs_vcb->block_size);

			LBAread(bitmap, 1, block_group.bitmap_location);

			// block index in that block group
			int b_idx = get_empty_block(bitmap, bfs_vcb->block_size);

			free(bitmap);

			return b_idx + bfs_vcb->gdt_size + 1 + (bfs_vcb->block_group_count * i);
		}
	}

	return -1;
}
