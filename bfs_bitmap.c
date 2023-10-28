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
			if (bit_check == 0) {
				return (i * 8) + j;
			}
		}
	}
	return -1;
}
