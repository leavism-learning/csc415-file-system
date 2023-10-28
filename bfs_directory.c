/*
 * Directory functions for BFS
 * Directories are limit to one block in size
 */
#include "mfs.h"

int bfs_create_root()
{

	// create here
	struct direntry_s here;
	here.size = bfs_vcb->block_size;
	
	// create parent
}

int bfs_init_directory()
{
	return 0;
}
