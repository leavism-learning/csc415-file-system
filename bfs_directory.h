/**************************************************************
 * Class:  CSC-415-01 Fall 2021
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: bfs_directory.h
 *
 * Description: Function definitions for bfs_directory
 *
 **************************************************************/

#include "mfs.h"

int bfs_create_root(struct bfs_dir_entry* buffer, int lba_pos);

int bfs_init_directory();

void bfs_create_here(struct bfs_dir_entry* here, int lba_pos);
