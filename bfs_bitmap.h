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
 * Description: Function definitions for bfs_bitmap.
 **************************************************************/
#ifndef BFS_BITMAP
#define BFS_BITMAP

#include "bfs.h"

/*
 * Bitmap functions for BFS
 * Author: Griffin Evans
 */

/*
 * Get value of bit at given position
 */
int bit_check(uint8_t byte, uint8_t pos);

/*
 * Set value of bit at given position
 *
 */
int bit_set(uint8_t byte, uint8_t pos);

int bit_clear(uint8_t byte, uint8_t pos);

int bit_toggle(uint8_t byte, uint8_t pos);

int get_empty_block(uint8_t *bitmap, int size);

int bfs_get_free_block();

// set vaule of a bit at given position in block
void block_bit_set(uint8_t *block, uint8_t pos);

#endif
