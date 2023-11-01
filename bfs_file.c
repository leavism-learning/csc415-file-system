/**************************************************************
 * Class:  CSC-415-01 Fall 2021
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: bfs_file.c
 *
 * Description: Functions for working with BFS files.
 *
 **************************************************************/

#include "bfs.h"

/*
 * Create a file with given name & size
 */
int bfs_create_file(struct bfs_dir_entry* dir_entry, char* name, uint64_t size, 
					uint64_t pos, uint8_t type)
{
	dir_entry->size = size;
	dir_entry->location = pos;
	strcpy(dir_entry->name, name);
	dir_entry->location = pos;
	time_t current_time = time(NULL);
	dir_entry->file_type = 0;
	dir_entry->date_created = current_time;
	dir_entry->date_modified = current_time;
	dir_entry->date_accessed = current_time;
  	return 0;
}

/*
 * Given a buffer, create extend table for that buffer
 * WIP: currently only allocated one block for each file
int bfs_create_extent(void *buffer, int size)
{
  struct bfs_extent_header header;
  header.eh_entries = 1;
  header.eh_depth = 1;

  struct bfs_extent ext;
  ext.ext_block = bfs_get_free_block();

  if (ext.ext_block == -1) {
    fprintf(stderr, "Error: No free blocks found\n");
    return 1;
  }
  ext.ext_len = 1;

  int index = sizeof(struct bfs_extent_header);
  memcpy(buffer, (void *)&header, index);
  memcpy(buffer index, (void *)&ext, sizeof(bfs_extent));

  return 0;
}
*/
