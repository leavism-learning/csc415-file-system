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

uint8_t bit_set(uint8_t byte, uint8_t position)
{
  return byte |= (1 << position);
}

int block_bit_set(uint8_t *block, uint8_t position)
{
  if (position >= bfs_vcb->block_size || position < 0) {
    fprintf(stderr, "Error: position is out of bounds.\n");
    return -1;
  }

  int index = position % 8;
  uint8_t byte = block[position / 8];
  byte = bit_set(byte, index);

  block[position / 8] = byte;

  return 0;
}

uint8_t bit_clear(uint8_t byte, uint8_t position)
{
  return byte &= ~(1 << position);
}

uint8_t bit_toggle(uint8_t byte, uint8_t position)
{
  return byte ^= 1 << position;
}

int get_empty_block(uint8_t *bitmap, int size)
{
  for (int byte_index = 0; byte_index < size; byte_index++) {
    for (int bit_index = 0; bit_index < 8; bit_index++) {
      if (bit_check(bitmap[byte_index], bit_index) == 0) {
        return (byte_index * 8) + bit_index;
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
    return 1;
  }

  // Index in block group is block number - ( group size * group number )
  int block_index = block_num - (bfs_vcb->block_group_size * block_group);
  if (block_bit_set(bitmap, block_index) != 0) {
    fprintf(stderr, "Error: Unabled to set bit in block\n");
    return 1;
  }

  if (LBAwrite(bitmap, 1, bfs_gdt[block_group].bitmap_location != -1)) {
    fprintf(stderr, "Error: Unable to write to block %ld\n", block_num);
    return 1;
  }

  free(bitmap);

  return 0;
}

int bfs_get_free_block()
{
  for (int i = 0; i < bfs_vcb->block_group_count; i++) {

    struct block_group_desc block_group = bfs_gdt[i];

    // if there are free blocks in the block group, read that block group's
    // bitmap and find the first available block
    if (block_group.free_blocks_count > 0) {

      uint8_t *bitmap = malloc(bfs_vcb->block_size);

      LBAread(bitmap, 1, block_group.bitmap_location);

      // block index in that block group
      int b_idx = get_empty_block(bitmap, bfs_vcb->block_size);

      free(bitmap);

      return b_idx + bfs_vcb->gdt_size + 1 + (bfs_vcb->block_group_count * i);
    }
  }

  return -1;
}
