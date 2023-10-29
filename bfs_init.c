/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: bfs_init.c
 *
 * Description: Implementation of core functionality for the Basic File
 * System (BFS), including volume initialization, group descriptor
 * management, and directory entry operations.
 **************************************************************/
#include "bfs.h"

/*
 * Creates a volume with the given name. Returns 0 on success, non-zero on
 * failure
 */
int bfs_vcb_init(char *name, uint64_t num_blocks, uint64_t block_size) {
  bfs_vcb->block_size = block_size;
  bfs_vcb->block_count = num_blocks;
  bfs_vcb->magic = 0x4465657A;
  bfs_vcb->block_group_size = block_size * 8;

  bfs_vcb->block_group_count = num_blocks / bfs_vcb->block_group_size;
  if (num_blocks % bfs_vcb->block_group_size)
    bfs_vcb->block_group_count++;

  int gdt_bytes = bfs_vcb->block_group_count * sizeof(struct block_group_desc);
  bfs_vcb->gdt_size = bytes_to_blocks(gdt_bytes, block_size);

  if (strlen(name) > 63)
    return 1;
  strcpy(bfs_vcb->volume_name, name);

  bfs_generate_uuid(bfs_vcb->uuid);

  return 0;
}

/*
 * Populate the group descriptor table
 * Group descriptor table contains block_size / sizeof(block_group_desc) block
 * groups Each block group contains block_size * 8 blocks.
 *
 * uint8_t* gdt: buffer to store gdt data
 */
int bfs_gdt_init(struct block_group_desc *gdt) {
  // bitmap location is directly after the gdt
  int bitmap_pos = bfs_vcb->gdt_size + 1;

  for (int i = 0; i < bfs_vcb->block_group_count; i++) {

    // create block group descriptor
    struct block_group_desc descriptor;
    descriptor.bitmap_location = bitmap_pos;
    descriptor.free_blocks_count = bfs_vcb->block_group_size - 1;
    descriptor.dirs_count = 0;

    uint8_t *bitmap = calloc(bfs_vcb->block_size, 1);

    // set 1st block of group because it contains the bitmap
    block_bit_set(bitmap, 0);

    // write bitmap to disk
    if (LBAwrite(bitmap, 1, bitmap_pos) != 1) {
      fprintf(stderr, "Error: Unable to LBAwrite bitmap %d to disk\n",
              bitmap_pos);
      return 1;
    }

    free(bitmap);

    gdt[i] = descriptor;
    bitmap_pos += bfs_vcb->block_group_size;
  }
  return 0;
}

/*
 * Initialize a directory entry.
 */
int create_dir_entry(struct bfs_dir_entry *dentry, char *name, int size,
                     int type) {
  // dentry->vcb = (uint64_t) &vcb;
  dentry->size = size;
  dentry->file_type = type;

  time_t current_time = time(NULL);
  dentry->date_created = current_time;
  dentry->date_modified = current_time;
  dentry->date_accessed = current_time;

  if (strlen(name) > MAX_FILENAME_LEN - 1) {
    return 1;
  }
  strcpy(dentry->name, name);

  return 0;
}

void bfs_generate_uuid(uint8_t *uuid) {
  // fill buffer with random values
  // get_random_bytes(uuid, 16);
  getrandom(uuid, 16, 0);

  // set the four most significant bits of the 7th byte to 0100
  uuid[6] &= 0x0F;
  uuid[6] |= 0x40;

  // set the two most significant bits of the 9th byte to 10
  uuid[8] &= 0x3F;
  uuid[8] |= 0x80;
}
