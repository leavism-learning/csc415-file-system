#include "bfs_helpers.h"

int bytes_to_blocks(int bytes, int block_size)
{
	return (bytes + block_size - 1) / block_size;
}

void print_dir_entry(struct bfs_dir_entry* bde)
{
	printf("name: %s  size: %ld  location: %ld  type: %d\n", bde->name, bde->size, 
			bde->location, bde->file_type);
}
