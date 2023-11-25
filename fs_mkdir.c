#include "bfs.h"

int fs_mkdir(const char *pathname, mode_t mode)
{
	struct bfs_dir_entry parent_entry;
	struct bfs_dir_entry dentry;
	char* trimmed_name;
	//the whole path until the new dir to make
	char* parent_path = "";
	char* delim = "/";

	char* copy_pathname = strdup(pathname);

	//for relative path
	if (copy_pathname[0] != '/') {
		if (bfs_cwd == NULL) {
			fprintf(stderr, "Error: CWD is null");
		}
		parent_entry = bfs_cwd[0];
		trimmed_name = copy_pathname;
	}
	//for absolute path
	else {
		int pathLength = strlen(copy_pathname);
		parent_path = strdup(copy_pathname);
		char* token1 = strtok(copy_pathname, delim);
		char* token2;
		printf("ParentPath: %s\n", parent_path);

		while (token1 != NULL) {
			token2 = strtok(NULL, delim);
			if (token2 == NULL) {
				trimmed_name = token1;
				break;
			}
			token1 = token2;
		}

		printf("Size of last element: %ld , %s\n", strlen(trimmed_name), trimmed_name);

		//truncates the given path to find the parent path
		parent_path[pathLength - strlen(trimmed_name)] = '\0';


		if (get_file_from_path(&parent_entry, parent_path) != 0) {
			fprintf(stderr, "Incorrect parent path!");
		}
		printf("ParentPath: %s, 		Parent DE name: %s\n", parent_path, parent_entry.name);

	}

	bfs_block_t pos = bfs_get_free_blocks(INIT_DIR_LEN);
	printf("pos: %ld\n", pos);
	printf("name: %s\n", trimmed_name);
	bfs_create_dir_entry(&dentry, trimmed_name, INIT_DIR_LEN, pos, 0);

	// directory needs 2 things: directory entry array (initially has ., .. and \0)
	printf("parent dir loc: %ld\n", parent_entry.location);
	if (bfs_create_directory(pos, parent_entry.location) != 0) {
		fprintf(stderr, "Unable to create dir entry for %s\n", pathname);
	}

	struct bfs_dir_entry* parent_dir = malloc(bfs_vcb->block_size * parent_entry.len);
	if (LBAread(parent_dir, parent_entry.len, parent_entry.location) != parent_entry.len) {
		fprintf(stderr, "Error reading from parent dir %ld", parent_entry.location);
		free(parent_dir);
		return 1;
	}

	int i = 0;
	struct bfs_dir_entry d = parent_dir[i];
	while (d.name[0] != '\0') {
		i++;
		d = parent_dir[i];
	}
	
	printf("size is %ld\n",sizeof(struct bfs_dir_entry));
	printf("putting entry %s at pos %d\n", dentry.name, i);
	struct bfs_dir_entry bde;
	parent_dir[i] = dentry;
	parent_dir[i+1] = d;

	if (LBAwrite(parent_dir, parent_dir->len, parent_dir->location) != parent_dir->len) {
		fprintf(stderr, "Error writing parent_dir dir %s to location %ld\n", parent_dir->name, parent_dir->location);
	}

	reload_cwd();

	free(parent_dir);

	return 0;
}
