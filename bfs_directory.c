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
 * Create a directory entry with given name & size
 */
int bfs_create_dir_entry(struct bfs_dir_entry* dir_entry, char* name, uint64_t size, uint64_t pos, uint8_t type)
{
	dir_entry->size = size;
	dir_entry->len = bytes_to_blocks(size);
	strcpy(dir_entry->name, name);
	dir_entry->location = pos;
	time_t current_time = time(NULL);
	dir_entry->file_type = type;
	dir_entry->date_created = current_time;
	dir_entry->date_modified = current_time;
	dir_entry->date_accessed = current_time;
	return 0;
}

char* fs_getcwd(char* pathname, size_t size) 
{
	return bfs_path;
}

// linux chdir
int fs_setcwd(char* pathname)
{
	char* true_path = expand_pathname(pathname);
	printf("true path is %s\n", true_path);
	struct bfs_dir_entry file;
	if (get_file_from_path(&file, pathname)) {
		return 1;
	}

	free(bfs_path);
	bfs_path = strdup(true_path);

	printf("got file %s with len %d and loc %ld\n", file.name, file.len, file.location);

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
	if (get_file_from_path(&file, pathname)) {
		return 0;
	}
	return !file.file_type;
}

// given a string, return a path 
char* expand_pathname(const char* in)
{
	if (strlen(in) < 1) {
		return NULL;
	}

    char* input = strdup(in);  // Duplicate the input to avoid modifying the original
    char* result = NULL;  // Resulting expanded path

    if (input[0] == '/') {
        // Absolute path, start from root
        result = strdup("/");
    } else {
        // Relative path, start from current working directory
        result = strdup(bfs_path);
    }

    // Tokenize the input path
    char* token = strtok(input, "/");
    while (token != NULL) {
        if (strcmp(token, "..") == 0) {
            // Move up one level (parent directory)
            char* last_slash = strrchr(result, '/');
            if (last_slash != NULL) {
                *last_slash = '\0';  // Remove the last component
            }
        } else if (strcmp(token, ".") != 0) {
            result = realloc(result, strlen(result) + strlen(token) + 2);
			// Skip any "." characters in path
            // Concatenate other components to the result
            if (strcmp(result, "/") != 0) {
                // Add '/' only if the current result is not the root directory
                strcat(result, "/");
            }
            strcat(result, token);
        }

        token = strtok(NULL, "/");
	}
	
	// empty string is root
	if (strlen(result) == 0) {
		free(result);
		result = strdup("/");
	}

    // Clean up dynamically allocated memory for the original input
    free(input);

    return result;
}

// return directory entry from file path
int get_file_from_path(struct bfs_dir_entry* target, const char* path)
{
	char* parent_path = expand_pathname(path);
	char* last_slash = strrchr(parent_path, '/');
	char* filename = NULL;
	if (last_slash != NULL) {
		filename = strdup(last_slash + 1);
		*last_slash = '\0';
	}
	if (strlen(parent_path) < 1) {
		parent_path = strdup("/");
	}
	if (strlen(filename) < 1) {
		filename = strdup(".");
	}

	char* tok = strtok(parent_path, "/");

	// start at root
	struct bfs_dir_entry* current_dir = malloc(bfs_vcb->root_len * bfs_vcb->block_size);
	LBAread(current_dir, bfs_vcb->root_len, bfs_vcb->root_loc);

	while (tok != NULL) {
		int index = find_file(tok, current_dir);
		// Handle if file not found or is not a directory
		if (index == -1 || current_dir[index].file_type != 0) {
			fprintf(stderr, "Matching file for %s not found", tok);
			free(parent_path);
			return 1;
		}

		// Find the directory entry of the current token
		
		struct bfs_dir_entry target_dir = current_dir[index];

		current_dir = realloc(current_dir, target_dir.size);
		if (LBAread(current_dir, target_dir.len, target_dir.location) != target_dir.len) {
			fprintf(stderr, "LBAread error in get_file_from_filepath\n");
			free(parent_path);
			return 1;
		}
		tok = strtok(NULL, "/");
	}


	// Find the final file/directory in the path
	int i = find_file(filename, current_dir);
	if (i == -1) {
		free(parent_path);
		return 1;
	}

	memcpy(target, &current_dir[i], sizeof(struct bfs_dir_entry));

	free(parent_path);
	parent_path = NULL;

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

// Returns the current directory item info.
// Subsequent calls retrieves the next directory item info.
struct fs_diriteminfo* fs_readdir(fdDir* dirp)
{
	if (dirp == NULL) { 
		fprintf(stderr, "Error: dirp is null.");
		return NULL;
	}

	struct bfs_dir_entry diritem = (dirp->directory)[dirp->dirEntryPosition++];
	if (diritem.name[0] == '\0') {
		return NULL;
	}

	dirp->di->d_reclen = sizeof(struct fs_diriteminfo);
	dirp->di->fileType = diritem.file_type;
	strcpy(dirp->di->d_name, diritem.name);
	return dirp->di;
}

fdDir* fs_opendir(const char* pathname) 
{
	struct bfs_dir_entry* dir_entry = malloc(sizeof(struct bfs_dir_entry));
	if (get_file_from_path(dir_entry, (char *)pathname)) {
		fprintf(stderr, "Unable to get file fram path %s\n", pathname);
		return NULL;
	}

	// read extents into buffer
	// check to make sure path points to directory not file
	if (dir_entry->file_type != 0) {
		fprintf(stderr, "File named %s is not a directory\n", pathname);
		return NULL;
	}

	struct bfs_dir_entry* directory_arr = malloc(bfs_vcb->block_size * dir_entry->len);

	// dir entry is file . 
	if (LBAread(directory_arr, dir_entry->len, dir_entry->location) != dir_entry->len) {
		fprintf(stderr, "Error reading directory at %ld\n", dir_entry->location);
		return NULL;
	}

	fdDir* dirp = malloc(sizeof(fdDir));
	dirp->d_reclen = sizeof(fdDir);;
	dirp->dirEntryPosition = 0;
	dirp->directory = directory_arr;
	dirp->di = malloc(sizeof(struct fs_diriteminfo));

	return dirp;
}

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

