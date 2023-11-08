/**************************************************************
 * Class:  CSC-415-01 Fall 2021
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File Syttem
 *
 * File: bfs_directory.c
 *
 * Description: Directory functions for Basic File System (BFS).
 *
 **************************************************************/

#include "bfs.h"

/*
* Create a directory (dir entry array) at the specified position
*/
int bfs_create_directory(bfs_block_t pos, bfs_block_t parent)
{
	struct bfs_dir_entry* buffer = malloc(bfs_vcb->block_size);
	if (buffer == NULL) {
		perror("malloc in bfs_create_directory");
		return 1;
	}

	if (LBAread(buffer, 1, pos) != 1) {
		fprintf(stderr, "Unable to LBAread pos %ld in bfs_create_dir\n", pos);
		return 1;
	}

	// create . and .. directory entries
	struct bfs_dir_entry here;
	bfs_create_file(&here, ".", bfs_vcb->block_size, pos, 0);
	struct bfs_dir_entry parend_dir;
	bfs_create_file(&parend_dir, "..", bfs_vcb->block_size, pos, 0);

	buffer[0] = here;
	buffer[1] = parend_dir;

	if (LBAwrite(buffer, 1, pos) != 1) {
		fprintf(stderr, "Unable to LBAwrite pos %ld in bfs_create_dir\n", pos);
	}

	return 0;
}

char* fs_getcwd(char* pathname, size_t size) 
{
	return bfs_path;
}

// linux chdir
int fs_setcwd(char* pathname)
{
	struct bfs_dir_entry file;
	if (get_file_from_path(&file, pathname)) {
		fprintf(stderr, "Unable to get file from path %s\n", pathname);
		return 1;
	}
	printf("got block %ld\n", file.location);

	int b = bytes_to_blocks(file.size);
	bfs_cwd = realloc(bfs_cwd, b * bfs_vcb->block_size);
	if (LBAread(bfs_cwd, b, file.location) != b) {
		fprintf(stderr, "LBAread failed at %ld in fs_setcwd\n", file.location);
		return 1;
	}

	return 0;
}

// return 1 if file, 0 otherwise
int fs_isFile(char* filename) 
{
	struct bfs_dir_entry file;
	if (!get_file_from_path(&file, filename)) {
		return 0;
	}
	return file.file_type;
}

// return 1 if directory, 0 otherwise
int fs_isDir(char* pathname)
{
	struct bfs_dir_entry file;
	if (!get_file_from_path(&file, pathname)) {
		return 0;
	}
	return !file.file_type;
}

// return directory entry from file path
int get_file_from_path(struct bfs_dir_entry* target, char* path)
{
	char* filepath = strdup(path);
	struct bfs_dir_entry* current_dir;
	printf("reading from positon %ld\n", bfs_vcb->root_loc);

	// if first char of filepath is /, it is absolute, so we should start 
	// traversing from root. otherwise ,start traversing from cwd
	if (filepath[0] == '/') {
		current_dir = malloc(bfs_vcb->block_size * bfs_vcb->root_len);
		LBAread(current_dir, bfs_vcb->root_len, bfs_vcb->root_loc);
	} else {
		current_dir = malloc(bfs_cwd[0].size);
		memcpy(current_dir,&bfs_cwd[0],bfs_cwd[0].size);
	}

	// get array of strings representing each entry in the filename ex.
	// "/home/student/" -> { "/", "home", "student"}
	char* tokens[strlen(filepath) * sizeof(char*)];
	char* tok = strtok(filepath, "/");
	int tok_count = 0;

	// if tok == null, it is root
	if (tok == NULL) {
		struct bfs_dir_entry* root = malloc(bfs_vcb->block_size * bfs_vcb->root_len);
		if (LBAread(root, bfs_vcb->root_len, bfs_vcb->root_loc) != bfs_vcb->root_len) {
			fprintf(stderr, "Unable to LBAread %ld in get_file_from_path\n", bfs_vcb->root_loc);
			return 1;
		}
		memcpy(target, root, sizeof(struct bfs_dir_entry));
		free(root);
		free(filepath);
		return 0;
	}

	while (tok != NULL) {
		tokens[tok_count++] = tok;
		tok = strtok(NULL, "/");
	}

	// navigate to penultimate token
	for( int i = 0; i < tok_count - 1; i++) {

		int index = find_file(tokens[i], current_dir);
		if (index == -1 || current_dir[i].file_type != 0) {
			// error case
			fprintf(stderr, "matching file for %s not found", tokens[i]);
			free(filepath);
			return 1;
		}
		
		struct bfs_dir_entry target_dir = current_dir[i];

		if (LBAread(current_dir, bytes_to_blocks(target_dir.size), 
			  target_dir.location)) {
			fprintf(stderr, "LBAread error in get_file_from_filepath\n");
			free(filepath);
			return 1;
		}
	}
	
	int i = find_file(tokens[tok_count - 1], current_dir);
	if (i == -1) {
		fprintf(stderr, "Unable to find file %s\n", tokens[tok_count - 1]);
		free(filepath);
		return 1;
	}

	memcpy(target, &current_dir[i], sizeof(struct bfs_dir_entry));

	free(filepath);
	filepath = NULL;

	free(current_dir);
	current_dir = NULL;

	return 0;
}

// return index of file with given name in given directory, or -1
int find_file(char* filename, struct bfs_dir_entry* directory)
{
	// number of files in given directory
	int num_files = directory->size / sizeof(struct bfs_dir_entry);
	// look for files except . and ..
	for (int i = 0; i < num_files; i++) {
		if (strcmp(filename, directory[i].name) == 0) {
			return i;
		}
	}

	return -1;
}
