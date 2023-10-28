#include "bfs.h"

/*
 * Create a file with given name & size
 */
int bfs_create_file(uint64_t size)
{
	struct bfs_dir_entry dentry;
	if (create_dir_entry(&dentry, 
	// block for data
	// block for extent
	// block for direntry
	
}

/*
 * Given a buffer, create extend table for that buffer
 * WIP: currently only allocated one block for each file
 */
bfs_create_extent(void* buffer, int size)
{
	struct bfs_extent_header header;
	header.eh_entries = 1;
	header.eh_depth = 1;

	struct bfs_extent ext;
	ext.ext_block = bfs_get_free_block();
	
	if (ext.ext_block == -1) {
		fprintf(stderr, "Error: No free blocks found\n");
		return 1;
	}
	ext.ext_len = 1;

	int index = sizeof(struct bfs_extent_header);
	memcpy(buffer, (void *) &header, index);
	memcpy(buffer index , (void * ) &ext, sizeof(bfs_extent));

	return 0;
}
}
