/**************************************************************
 * Class:  CSC-415-01 Fall 2021
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: bfs_directory.c
 *
 * Description: Directory functions for Basic File System (BFS).
 *
 **************************************************************/

#include "bfs.h"

int bfs_create_root(struct bfs_dir_entry *buffer, int lba_pos) {
  // first, create the root directory.
  // then, create the . and .. directory entries

  // create here (.)
  struct bfs_dir_entry here;
  bfs_create_here(&here, lba_pos);

  // parent (..) is same as . for root
  struct bfs_dir_entry parent;
  bfs_create_here(&parent, lba_pos);
  strcpy(parent.name, "..");

  buffer[0] = here;
  buffer[1] = parent;

  return 0;
}

int bfs_init_directory() { return 0; }

/*
 * Create the . directory
 */
void bfs_create_here(struct bfs_dir_entry *here, int lba_pos) {
  here->size = bfs_vcb->block_size;
  here->location = lba_pos;
  strcpy(here->name, ".");
  here->file_type = 0;
  time_t current_time = time(NULL);
  here->date_created = current_time;
  here->date_modified = current_time;
  here->date_accessed = current_time;
}
