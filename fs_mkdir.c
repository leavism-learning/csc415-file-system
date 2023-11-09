#include "bfs.h"

#define INIT_DIR_SIZE 16

int fs_mkdir(const char *pathname, mode_t mode)
{
	struct bfs_dir_entry* parent = malloc(sizeof(struct bfs_dir_entry));
	struct bfs_dir_entry dentry;
	char* lastElement;
	//the whole path until the new dir to make
	char* parentPath = "";
	char* delim = "/";

	char* copy_pathname = strdup(pathname);

	//for relative path
	if(copy_pathname[0] != '/') {
		parent = bfs_cwd;
		lastElement = copy_pathname;
	}
	//for absolute path
	else {
		int pathLength = strlen(copy_pathname);
		parentPath = strdup(copy_pathname);
		char* token1 = strtok(copy_pathname, delim);
		char* token2;
		printf("ParentPath: %s\n", parentPath);

		while(token1 != NULL) {
			token2 = strtok(NULL, delim);
			if(token2 == NULL) {
				lastElement = token1;
				break;
			}
			token1 = token2;
		}

		printf("Size of last element: %d			%s\n", strlen(lastElement), lastElement);

		parentPath[pathLength - strlen(lastElement)] = '\0';
		

		if(get_file_from_path(parent, parentPath) != 0) {
			fprintf(stderr, "Incorrect parent path!");
		}
		// printf("ParentPath: %s, 		Parent DE name: %s\n", parentPath, parent->name);

	}

	if(create_dir_entry(&dentry, lastElement, sizeof(struct bfs_dir_entry), 0) != 0) {
		fprintf(stderr, "Name provided is too long!");
	}

	// bfs_create_file()

	//parent[] = dentry

	//TODO: have to finish and use bfs_create_directory

	dentry.location = bfs_get_free_block();
	bfs_create_directory(dentry.location, parent->location);

	free(parent);
}
