#include "bfs.h"

// recursive function to free extent memory
int free_extents(uint8_t* buffer) {
	struct bfs_extent_header header = ((struct bfs_extent_header*) buffer)[0];

	// if depth is 0, then the next section is a leaf node which points
	// to blocks that should be cleared
	if (header.eh_depth == 0) {
		for (int i = 0; i < header.eh_entries; i++) {
			// offset by 1 to skip header
			struct bfs_extent leaf = ((struct bfs_extent*) buffer)[1 + i];  
			bfs_clear_blocks(leaf.ext_block, leaf.ext_len);
			return 0;
		}
	}
	else {
		for (int i = 0; i < header.eh_entries; i++) {
			struct bfs_extent_idx idx = ((struct bfs_extent_idx*) buffer)[1];
			uint8_t* newbuf = malloc(bfs_vcb->block_size);

			if (LBAread(newbuf, 1, idx.idx_leaf) != 1) {
				fprintf(stderr, "unable to read extent block %ld\n", idx.idx_leaf);
				free(newbuf);
				return 1;
			}

			free_extents(newbuf);
			free(newbuf);
		}
	}
	return 0;
}

int fs_delete(char* filename) {
	struct bfs_dir_entry file;
	if (get_file_from_path(&file, filename)) {
		fprintf(stderr, "Unable to find file %s\n", filename);
		return 1;
	}

	if (file.file_type == 0) {
		fprintf(stderr, "Error: %s is a directory\n", filename);
		return 1;
	}

	// read extents to find which blocks must be freed
	uint8_t* buffer = malloc(bfs_vcb->block_size);

	if (LBAread(buffer, 1, file.location) != 1) {
		fprintf(stderr, "Unable to LBAread block %ld in fs_delete\n", file.location);
		free(buffer);
		return 1;
	}

	if (free_extents(buffer)) {
		free(buffer);
		return 1;
	}

	free(buffer);
	buffer = NULL;

	// TODO remove directory entry 
	
	return 0;
}

