#include "mfs.h"

int bfs_create_root(struct bfs_dir_entry* buffer, int lba_pos);

int bfs_init_directory();

void bfs_create_here(struct bfs_dir_entry* here, int lba_pos);
