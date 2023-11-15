#include "bfs.h"

// Returns the current directory item info.
// Subsequent calls retrieves the next directory item info.
struct fs_diriteminfo* fs_readdir(fdDir* dirp)
{
	if (dirp == NULL || dirp->di == NULL) {
		fprintf(stderr, "Error: dirp or dirp.di is null.");
		return NULL;
	}

	struct bfs_dir_entry diritem = (dirp->directory)[dirp->dirEntryPosition++];

	// populate  fsdiriteminfo struct
	struct fs_diriteminfo* info = malloc(sizeof(struct fs_diriteminfo));

	info->d_reclen = sizeof(struct fs_diriteminfo);
	info->fileType = diritem.file_type;
	strcpy(info->d_name, diritem.name);
	return info;

}
