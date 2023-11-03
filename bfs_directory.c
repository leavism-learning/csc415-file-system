/**************************************************************
 * Class:  CSC-415-01 Fall 2021
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: bfs_directory.c
 *
 * Description: Directory functions for Basic File System (BFS).
 *
 **************************************************************/

#include "bfs.h"
#include "mfs.h"

/*
* Create a directory at the specified position
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

void bfs_create_here(struct bfs_dir_entry *here, int lba_position)
{
	here->size = bfs_vcb->block_size;
	here->location = lba_position;
	strcpy(here->name, ".");
	here->file_type = 0;
	time_t current_time = time(NULL);
	here->date_created = current_time;
	here->date_modified = current_time;
	here->date_accessed = current_time;
}

char* fs_getcwd(char* pathname, size_t size) 
{
	return bfs_path;
}

// linux chdir
int fs_setcwd(char* pathname)
{
	if (strcmp(pathname, "/") == 0) {
		// read root and set cwd
		bfs_cwd = realloc(bfs_cwd, bfs_vcb->block_size * bfs_vcb->root_size);
		if (LBAread(bfs_cwd, bfs_vcb->root_size, bfs_vcb->root_loc) != 1) {
			fprintf(stderr, "LBAread failed in fs_getwcd");
			return 1;
		}
	}

	return 0;
}

// return 1 if file, 0 otherwise
int fs_isFile(char *filename) 
{
	return 0;
}

// return 1 if directory, 0 otherwise
int fs_isDir(char *pathname)
{
	return 0;
}

// removes a file
int fs_delete(char *filename)
{
	return 0;
}

// return directory entry from file path
// TODO implement relative paths
int get_file_from_path(struct bfs_dir_entry* target, char* path)
{
	struct bfs_dir_entry* current_dir;

	// if first char of path is /, it is absolute, so we should start 
	// traversing from root. otherwise ,start traversing from cwd
	if (path[0] == '/') {
		current_dir = malloc(bfs_vcb->block_size * bfs_vcb->root_size);
		LBAread(current_dir, bfs_vcb->block_size * bfs_vcb->root_size, 
		  bfs_vcb->root_loc);
	} else {
		current_dir = malloc(bfs_cwd[0].size);
		memcpy(current_dir,&bfs_cwd[0],bfs_cwd[0].size);
	}

	// get array of strings representing each entry in the filename ex.
	// "/home/student/" -> { "/", "home", "student"}
	char* tokens[strlen(path) * sizeof(char*)];
	char* tok = strtok(path, "/");
	int tok_count = 0;

	while (tok != NULL) {
		tokens[tok_count++] = tok;
		tok = strtok(NULL, "/");
	}

	// navigate to penultimate token
	for( int i = 0; i < tok_count - 1; i++) {

		int index = find_file(tokens[i], current_dir);
		if (index == -1 || current_dir[i].file_type != 0) {
			return 1;
		}
		
		struct bfs_dir_entry target_dir = current_dir[i];

		if (LBAread(current_dir, bytes_to_blocks(target_dir.size), 
			  target_dir.location)) {
			fprintf(stderr, "LBAread error in get_file_from_path\n");
			return 1;
		}
	}
	
	int i = find_file(tokens[tok_count - 1], current_dir);
	if (i == -1) {
		return 1;
	}

	memcpy(target, &current_dir[i], sizeof(struct bfs_dir_entry));

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
	for (int i = 2; i < num_files; i++) {
		if (strcmp(filename, directory[i].name) == 0) {
			return i;
		}
	}

	return -1;
}
