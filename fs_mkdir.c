#include "bfs.h"

#define INIT_DIR_SIZE 16

int fs_mkdir(const char *pathname, mode_t mode)
{
	struct bfs_dir_entry parent;
	struct bfs_dir_entry dentry;
	char* lastElement;
	//the whole path until the new dir to make
	char* parentPath = "";
	char* delim = "/";

	char* copy_pathname = (char*) pathname;

	//for relative path
	if(copy_pathname[0] != '/') {
		parent = *bfs_cwd;
		lastElement = copy_pathname;
	}
	//for absolute path
	else {
		char* token1 = strtok(copy_pathname, delim);
		char* token2;

		while(token1 != NULL) {
			token2 = strtok(NULL, delim);
			if(token2 == NULL) {
				lastElement = token1;
				break;
			}
			strcat(parentPath, delim);
			strcat(parentPath, token1);
			token1 = token2;
		}

		if(get_file_from_path(&parent, parentPath) != 0) {
			fprintf(stderr, "Incorrect parent path!");
		}

	}

	if(create_dir_entry(&dentry, lastElement, sizeof(struct bfs_dir_entry), 0) != 0) {
		fprintf(stderr, "Name provided is too long!");
	}

	//TODO: have to finish and use bfs_create_directory

	dentry.location = bfs_get_free_block();
	bfs_create_directory(dentry.location, parent.location);

}
