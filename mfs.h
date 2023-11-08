/**************************************************************
 * Class:  CSC-415
 * Name: Professor Bierman
 * Student ID: N/A
 * Project: Basic File System
 *
 * File: mfs.h
 *
 * Description:
 *	This is the file system interface.
 *	This is the interface needed by the driver to interact with
 *	your filesystem.
 *
 **************************************************************/
#ifndef _MFS_H
#define _MFS_H

#include <ctype.h>
#include <dirent.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "b_io.h"

#include "fsLow.h"

#define FT_REGFILE DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK DT_LNK

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif

#ifndef bfs_block_t
typedef uint64_t bfs_block_t;
#endif

#define MAX_FILENAME_LEN 80
#define NUM_FILES 2048
#define ROOT_INODE 1;

/*
 * vcb_s: Volume Control Block
 * The VCB is the first block of the file system, which contains basic
 * parameters of the system.
 */
struct vcb_s {
	uint32_t block_size;		// block size in bytes
	uint64_t block_count;		// number of blocks
	char volume_name[64];		// volume name
	uint8_t uuid[16];			// volume signature
	uint32_t magic;				// magic signature
	uint32_t block_group_size;	// number of blocks per block group
	uint32_t block_group_count; // number of block groups
	uint32_t gdt_len;			// size (in blocks) for group descriptor table
	bfs_block_t root_loc;		// location of root directory
	uint32_t root_len;			// size (in blocks) of root directory
};

/*
 * block_group_desc: Block Group Descriptor Table
 * The second block of the file system, after the Volume Control Block, is the
 * Block Group Descriptor Table, which contains an array of descriptors for
 *each block group.
 *
 * The first block of each block group is that block group's bitmap, where each
 * bit indicates if a block is available (0) or in use (1). Each block group
 * contains BLOCK_SIZE * 8 blocks.
 *
 * A block group descriptor contains the following information (positions in
 * bytes): 0-3:  The address (lbaPosition) of block usage bitmap for that group
 *	 4-8:  The number of unallocated blocks in the group
 *	 8-12: The number of directories in the block group
 */
struct block_group_desc {
	bfs_block_t bitmap_location;
	uint32_t free_blocks_count;
	uint32_t dirs_count;
};

/*
 * bfs_extent_header: Header for BFS Extent Table blocks
 *
 * An extent refers to a group of contiguous blocks.
 */
struct bfs_extent_header {
	uint32_t eh_entries; // number of entries after the header
	uint32_t eh_max;	 // max number of entries after header
	uint16_t eh_depth;	 // depth of this extent node in the extent tree. 0
						 // = this extent node points to data blocks;
						 // otherwise, this extent node points to other
						 // extent nodes.
	uint16_t unused;
};

struct bfs_extent_idx {
	// This index node covers file blocks from 'block' onward.
	uint64_t idx_start;
	// block number of the extent node that is the next level lower in the
	// tree. The tree node pointed to can be either another internal node or
	// a leaf node
	uint64_t idx_block;
};

struct bfs_extent {
	uint64_t ext_block;		// first block number that this extent covers
	uint64_t ext_len;		// number of blocks covered by extent
};

/*
 * Directory Entry stores first-class information about a file. 128 byte size
 */
struct bfs_dir_entry {
	uint64_t size;				 // file size in bytes (max 16384 PiB)
	uint32_t len;				 // file length in blocks 
	bfs_block_t location;		 // lba position of file extents
	uint8_t file_type;			 // 0 if directory, otherwise file
	time_t date_created;		 // file creation time
	time_t date_modified;		 // last time file was modified
	time_t date_accessed;		 // last time file was read
	char name[MAX_FILENAME_LEN]; // file name
};

// This structure is returned by fs_readdir to provide the caller with
// information about each file as it iterates through a directory
struct fs_diriteminfo {
	unsigned short d_reclen; /* length of this record */
	unsigned char fileType;
	char d_name[256]; /* filename max filename is 255 characters */
};

// This is a private structure used only by fs_opendir, fs_readdir, and
// fs_closedir Think of this like a file descriptor but for a directory - one
// can only read from a directory.	This structure helps you (the file
// system) keep track of which directory entry you are currently processing so
// that everytime the caller calls the function readdir, you give the next
// entry in the directory
typedef struct {
	// TODO:  Fill in this structure with what your open/read directory needs
	unsigned short d_reclen;			/* length of this record */
	uint64_t di_offset;					// current directory item index
	uint64_t di_start_position;			//
	unsigned short dirEntryPosition;	/* which directory entry position, like file pos */
	// DE *	directory;					/* Pointer to the loaded directory you want to iterate */
	struct fs_diriteminfo* di;			/* Pointer to the structure you return from read */
} fdDir;

extern struct vcb_s *bfs_vcb;
extern struct block_group_desc *bfs_gdt;
extern struct bfs_dir_entry *bfs_cwd;
extern char *bfs_path;

// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);

// Directory iteration functions
fdDir *fs_opendir(const char *pathname);
struct fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);

// Misc directory functions
char *fs_getcwd(char *pathname, size_t size);
int fs_setcwd(char *pathname); // linux chdir
int fs_isFile(char *filename); // return 1 if file, 0 otherwise
int fs_isDir(char *pathname);  // return 1 if directory, 0 otherwise
int fs_delete(char *filename); // removes a file

// This is the strucutre that is filled in from a call to fs_stat
struct fs_stat {
	off_t st_size;		  /* total size, in bytes */
	blksize_t st_blksize; /* blocksize for file system I/O */
	blkcnt_t st_blocks;   /* number of 512B blocks allocated */
	time_t st_accesstime; /* time of last access */
	time_t st_modtime;	  /* time of last modification */
	time_t st_createtime; /* time of last status change */

	/* add additional attributes here for your file system */
};

int fs_stat(const char *path, struct fs_stat *buf);

#endif
