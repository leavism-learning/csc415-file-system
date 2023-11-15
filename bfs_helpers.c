/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Names: Griffin Evans, Sukrit Dev Dhawan, Michelle Lang, Giahuy Dang
 * Student IDs: 922498210, 922432027, 917386319, 922722304
 * GitHub Name: griffinevans
 * Group Name: Team CDeez
 * Project: Basic File System
 *
 * File: bfs_helpers.c
 *
 * Description: A set of helper functions when working with the
 * Basic File System (BFS).
 **************************************************************/

#include "bfs.h"

int bytes_to_blocks(int bytes)
{
	return (bytes + bfs_vcb->block_size - 1) / bfs_vcb->block_size;
}

void print_dir_entry(struct bfs_dir_entry *dentry)
{
	printf("name: %s  size: %ld  location: %ld  type: %d\n", dentry->name,
		dentry->size, dentry->location, dentry->file_type);
}

int write_current_vcb()
{
	if(LBAwrite(bfs_vcb, 1, 0) != 1) {
		fprintf(stderr, "Error: Unable to write GDT to disk\n");
		return 1; 
	}
	return 0;
}

int write_current_gdt()
{
	if(LBAwrite(bfs_gdt, bfs_vcb->gdt_len, 1) != 1) {
		fprintf(stderr, "Error: Unable to write GDT to disk\n");
		return 1;
	}
	return 0;
}

char* get_filename_from_path(const char* pathname) {
	// Handles NULL or empty pathname
	if (pathname == NULL || *pathname == '\0') {
		char* filename = malloc(sizeof(char));
		if (filename) *filename = '\0';
		return filename;
	}

	// Finds last ocrrence of '/'
	const char* last_slash = strchr(pathname, '/');
	if (last_slash == NULL) {
		return strdup(pathname);
	}

	return strdup(last_slash + 1);
}