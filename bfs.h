#ifndef BFS_H
#define BFS_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_FILENAME_LEN 64
#define BLOCK_SIZE 512
#define BLOCK_COUNT 8
#define NUM_FILES 2048
#define ROOT_INODE 1;

/*
 * Volume Control Block
 * The block and bitmap records which entries in the block table are in use. One bit 
 * represents the usage status of one data block
 */
struct vcb_s {
  uint32_t       block_size;              // block size
  uint32_t       block_count;             // number of blocks
  uint32_t       free_blocks;             // bitmap of unused blocks
  uint64_t       block_head;              // location of first block
  char           volume_name[64];         // volume name
  uint8_t        uuid[16];                // volume signature 
};

/*
 * Directory Entry stores first-class information about a file
 */
struct direntry_s {
  uint64_t  vcb;                          // address of the volume control block
  char      name[MAX_FILENAME_LEN];       // file name
  uint32_t  size;                         // file size in bytes
  uint64_t  location;                     // file location in memory
  uint8_t   file_type;                    // 0 if directory, otherwise file 
  uint8_t   num_blocks;                   // number of blocks used by the file
  time_t    date_created;                 // file creation time       
  time_t    date_modified;                // last time file was modified
  time_t    date_accessed;                // last time file was read
};

/*
 * Creates a volume with the given name. Returns 0 on success, non-zero on failure
 */
int vcb_init(struct vcb_s* vcb, char* name);

/*
 * Initialize a directory entry. Returns 0 on success, non-zero on failure
 */
int create_dentry(struct vcb_s* vcb, struct direntry_s* dentry, char* name, int size, int type);

int is_valid_volname(char* string);

#endif
