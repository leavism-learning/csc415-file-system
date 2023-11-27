#include "bfs.h"

int fs_mkdir(const char *pathname, mode_t mode)
{
	char* parent_path = expand_pathname(pathname);
	char* last_slash = strrchr(parent_path, '/');
	char* filename = NULL;
	if (last_slash != NULL) {
		filename = strdup(last_slash + 1);
		*last_slash = '\0';
	}
	if (strlen(parent_path) < 1) {
		parent_path = strdup("/");
	}

	struct bfs_dir_entry parent_entry;
	if (get_file_from_path(&parent_entry, parent_path)) {
		fprintf(stderr, "Unable to get parent file from path %s\n", parent_path);
		free(parent_path);
		free(filename);
		return 1;
	}
	
	struct bfs_dir_entry* parent_dir = malloc(parent_entry.size);

	struct bfs_dir_entry dentry;

	bfs_block_t pos = bfs_get_free_blocks(INIT_DIR_LEN);
	bfs_create_dir_entry(&dentry, filename, bfs_vcb->block_size * INIT_DIR_LEN, pos, 0);

	// directory needs 2 things: directory entry array (initially has ., .. and \0)

	// create directory entry for parent directory
	if (LBAread(parent_dir, parent_entry.len, parent_entry.location) != parent_entry.len) {
		fprintf(stderr, "Error reading from parent dir %ld", parent_entry.location);
		free(parent_dir);
		free(filename);
		free(parent_path);
		return 1;
	}

	if (find_file(filename, parent_dir) != -1) {
		fprintf(stderr, "Error: File already exists\n");
		free(parent_dir);
		free(filename);
		free(parent_path);
		return 1;
	}


	int i = 0;
	struct bfs_dir_entry d = parent_dir[i];
	while (d.name[0] != '\0') {
		i++;
		d = parent_dir[i];
	}
	
	parent_dir[i] = dentry;
	parent_dir[i+1] = d;

	if (LBAwrite(parent_dir, parent_dir->len, parent_dir->location) != parent_dir->len) {
		fprintf(stderr, "Error writing parent_dir dir %s to location %ld\n", parent_dir->name, parent_dir->location);
	}

	reload_cwd();

	// create directory entries for new directory
	struct bfs_dir_entry here;
	struct bfs_dir_entry parent_dentry;
	bfs_create_dir_entry(&here, ".", bfs_vcb->block_size * INIT_DIR_LEN, pos, 0);
	bfs_create_dir_entry(&parent_dentry, "..", bfs_vcb->block_size * parent_dir[0].len, parent_dir[0].location, 0);
	struct bfs_dir_entry* new_dir_array = malloc(here.size);
	if (LBAread(new_dir_array, here.len, here.location) != here.len) {
		fprintf(stderr, "LBAread error in mkdir\n");
		return 1;
	}

	struct bfs_dir_entry nulldir;
	nulldir.name[0] = '\0';

	new_dir_array[0] = here;
	new_dir_array[1] = parent_dentry;
	new_dir_array[2] = nulldir;

	if (LBAwrite(new_dir_array, here.len, here.location) != here.len) {
		fprintf(stderr, "LBAwrite error in mkdir\n");
		return 1;
	}

	free(parent_dir);

	return 0;
}
