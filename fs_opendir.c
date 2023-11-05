#include "mfs.h"
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

fdDir *fs_opendir(const char *pathname) {
    struct bfs_dir_entry* dir_entry = malloc(sizeof(struct bfs_dir_entry));
    if(!get_file_from_path(dir_entry, pathname)) {
		fprintf(stderr, "Unable to get file from path: \n", pathname);
        return 1;
    }
    // check to make sure path points to directory not file
    if(dir_entry->file_type != 0) {
		fprintf(stderr, "Not a directory\n");
        return 1;
    }
    fdDir* dirp = malloc(sizeof(fdDir));
    memcpy(dirp->di, dir_entry, sizeof(struct bfs_dir_entry)); 
    dirp->d_reclen = sizeof(sizeof(struct bfs_dir_entry));
    dirp->dirEntryPosition = dir_entry->location;
    // TODO: check with the updated fields in fdDir struct

    // free??
    free(dir_entry);
    return dirp;
}