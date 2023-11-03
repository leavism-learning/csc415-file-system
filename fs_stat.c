// Separate files for holding functions and helper functions of
// fs_stat for now
// Integrate into the rest of the code later



#include "mfs.h"
#include "bfs.h"
#include "fsLow.h"


// TODO: delete later
// for reference
/*struct bfs_dir_entry {
  uint64_t size;               // file size in bytes (max 16384 PiB)
  bfs_block_t location;        // lba position of file extents
  uint8_t file_type;           // 0 if directory, otherwise file
  time_t date_created;         // file creation time
  time_t date_modified;        // last time file was modified
  time_t date_accessed;        // last time file was read
  char name[MAX_FILENAME_LEN]; // file name
};
// This is the strucutre that is filled in from a call to fs_stat
struct fs_stat {
  off_t st_size;         total size, in bytes 
  blksize_t st_blksize; /* blocksize for file system I/O 
  blkcnt_t st_blocks;   /* number of 512B blocks allocated 
  time_t st_accesstime; /* time of last access 
  time_t st_modtime;    /* time of last modification 
  time_t st_createtime; /* time of last status change 
};

*/

int fs_stat(const char *path, struct fs_stat *buf) {
  // TODO: test to make sure this works
  struct bfs_dir_entry* dir_entry;
  get_file_from_path(dir_entry, path);
  buf->st_blksize = MINBLOCKSIZE;
	buf->st_accesstime = dir_entry->date_accessed;
	buf->st_size = dir_entry->size;
	buf->st_createtime = dir_entry->date_created;
	buf->st_modtime = dir_entry->date_modified;
  buf->st_blocks = bytes_to_blocks(dir_entry->size);
}
