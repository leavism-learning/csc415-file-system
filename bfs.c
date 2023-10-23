#include "bfs.h"
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

/*
 * Creates a volume with the given name. Returns 0 on success, non-zero on failure
 * TODO generate uuid
 */
int vcb_init(struct vcb_s* vcb, char* name) {
  vcb->block_size   = BLOCK_SIZE;
  vcb->block_count  = BLOCK_COUNT;
  vcb->free_blocks  = BLOCK_COUNT;
  vcb->block_head   = 0;        

  if (strlen(name) > 63) {
    return 1;
  }
  strcpy(vcb->volume_name, name);

  return 0;
}

/*
 * Initialize a directory entry. 
 */
int create_dentry(struct vcb_s* vcb, struct direntry_s* dentry, char* name, int size, int type) {
  dentry->vcb = (uint64_t) &vcb;
  dentry->size = size;
  dentry->file_type = type;

  // block size is size / BLOCK_SIZE, plus one block if there is a remainder
    dentry->num_blocks = size / BLOCK_SIZE;
    if (size % BLOCK_SIZE != 0)
      dentry->num_blocks++;

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
