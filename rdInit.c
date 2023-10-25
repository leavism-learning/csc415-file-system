/**************************************************************
 * Class:  CSC-415-01 Fall 2021
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs:
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: rdInit.c
 *
 * Description: Initializes root directory
 *
 **************************************************************/

#include <bfs.h>
#include <fsLow.h>

#define START_BLOCK 1  // using 1 as starting block for root for now, 0 for vcb

#define DENTRY_COUNT 50 // for testing purposes

// Current status: In progress, does not work

// init_dentry : initial # of dentry
int rd_init(int init_dentry) {
    // Allocate memory -> determine how much from dentry struct
    int bytes_needed = DENTRY_COUNT * sizeof(struct direntry_s);
    int blocks_needed = (bytes_needed + (BLOCK_SIZE - 1)) / BLOCK_SIZE;
    bytes_needed = BLOCK_SIZE * blocks_needed;
    int total_dentry_count = bytes_needed / sizeof(struct direntry_s);
    struct direntry_s* dir = malloc(bytes_needed); // array to hold 
    // Allocate the blocks in vcb -> check how we are doing free space
    // initialize directory entry
    for (int i = 0; i < blocks_needed; i++) {
        dir[i].name[0] = '\0';
        // check if we need a value to see in direntry for labeling if it is used or not
    }
    // initializing . for root
    strcpy(dir[0].name, ".");
    dir[0].size = bytes_needed; // prof has us using total dentry count
    dir[0].location = START_BLOCK;
    dir[0].file_type = 0;
    time_t t = time;
    dir[0].date_created = time;
    dir[0].date_accessed = time;
    dir[0].date_modified = time;

    // .. for root
    // putting here for now, may be good to separate out into another function for reuseability
    struct direntry_s* root = &dir[0];
    strcpy(dir[1].name, "..");
    dir[1].size = root->size;
    dir[1].location = root->location;
    dir[1].file_type = root->file_type;
    dir[1].date_created = time;
    dir[1].date_accessed = time;
    dir[1].date_modified = time;

    LBAwrite(dir, blocks_needed, START_BLOCK);
    free(dir);

    return START_BLOCK;
}
