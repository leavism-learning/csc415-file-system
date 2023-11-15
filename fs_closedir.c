#include "bfs.h"

int fs_closedir(fdDir *dirp)
{
	if (dirp != NULL) {

		if (dirp->di != NULL) {
			free(dirp->di);
		}
		if (dirp->directory != NULL) {
			free(dirp->directory);
		}

		free(dirp);
	}

	return 0;
}
