#include "bfs.h"

// Returns the current directory item info.
// Subsequent calls retrieves the next directory item info.
struct fs_diriteminfo* fs_readdir(fdDir* dirp)
{
	if (dirp == NULL || dirp->di == NULL) {
		fprintf(stderr, "Error: dirp or dirp.di is null.");
		return NULL;
	}

	// TODO: Load the directory item array from disk
	struct bfs_dir_entry* dir_head = dirp->directory;
	LBAread(di_array, num_blocks, dirp->di_start_position); // TODO: Michelle is working on fs_opendir.
	// TODO start here
	// The di_start_position changes based on
	// the CWD, which is changed when we CD (or
	// call fs_opendir)

	while (di_array[dirp->di_offset].) {
	}
	// Assuming that dirp->di points to an array of fs_diriteminfo
	// structures. Then we should fetch the directory entry corresponding to
	// dirEntryPosition
	struct fs_diriteminfo* currentEntry = dirp->di[dirp->dirEntryPosition];

	// Increment the position for the next call
	dirp->dirEntryPosition++;

	LBAwrite(di_array, num_blocks, dirp->di_start_position);

	// Return the current entry
	return currentEntry;
}
