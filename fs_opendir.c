#include "bfs.h"

// This is a private structure used only by fs_opendir, fs_readdir, and
// fs_closedir Think of this like a file descriptor but for a directory - one
// can only read from a directory.	This structure helps you (the file system)
// keep track of which directory entry you are currently processing so that
// everytime the caller calls the function readdir, you give the next entry in
// the directory

// typedef struct {
// 	unsigned short d_reclen; /* length of this record */
//   	unsigned int totalEntries; // no longer used
// 	unsigned short dirEntryPosition; /* which directory entry position, like file pos */
// 	// DE *	directory;			/* Pointer to the loaded directory you want to
// 	// iterate */
// 	struct fs_diriteminfo *di; /* Pointer to the structure you return from read */
// } fdDir;

fdDir* fs_opendir(const char* pathname) 
{
	struct bfs_dir_entry* dir_entry;
	if (!get_file_from_path(dir_entry, (char *) pathname)) {
		fprintf(stderr, "Unable to get file from path: %s\n", pathname);
		return NULL;
	}

	// read extents into buffer
	// check to make sure path points to directory not file
	if (dir_entry->file_type != 0) {
		fprintf(stderr, "File named %s is not a directory\n", pathname);
		return NULL;
	}

	struct bfs_dir_entry* directory_arr = malloc(bfs_vcb->block_size * dir_entry->len);

	// dir entry is file . 
	if (LBAread(directory_arr, dir_entry->len, dir_entry->location) != dir_entry->len) {
		fprintf(stderr, "Error reading directory at %ld\n", dir_entry->location);
		return NULL;
	}

	fdDir* dirp = malloc(sizeof(fdDir));
	dirp->d_reclen = sizeof(fdDir);
	dirp->dirEntryPosition = 0;
	dirp->directory = directory_arr;

	return dirp;
}
