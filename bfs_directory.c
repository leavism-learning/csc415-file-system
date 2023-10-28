/*
 * Directory functions for BFS
 * Directories are limit to one block in size
 */
#include "bfs_directory.h"

int bfs_create_root(int lba_pos)
{
	// create here (.) 
	struct bfs_dir_entry here;
	bfs_create_here(&here, lba_pos);

	// parent (..) is same as . for root
	struct bfs_dir_entry parent;
	bfs_create_here(&parent, lba_pos);
	strcpy(parent.name, "..");

	return 0;
}

int bfs_init_directory()
{
	return 0;
}

/*
 * Create the . directory
 */
void bfs_create_here(struct bfs_dir_entry* here, int lba_pos)
{
	here->size = bfs_vcb->block_size;
	here->location = lba_pos;
	strcpy(here->name, ".");
	here->file_type = 0;
	time_t current_time = time(NULL);
	here->date_created = current_time;
	here->date_modified = current_time;
	here->date_accessed = current_time;
}
