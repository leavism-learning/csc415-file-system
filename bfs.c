/*
 * Functions for BFS system.
 * Author: Griffin Evans
 */

#include "bfs.h"

#include "fsLow.h"

/*
 * Creates a volume with the given name. Returns 0 on success, non-zero on failure
 */
int bfs_vcb_init(char* name, uint64_t num_blocks, uint64_t block_size) 
{
	fs_vcb->block_size   	= block_size;
	fs_vcb->block_count  	= num_blocks;
	fs_vcb->magic 	  	= 0x4465657A;
	fs_vcb->block_group_size 	= block_size * 8;
	
	fs_vcb->block_group_count  = num_blocks / fs_vcb->block_group_size;
	if (num_blocks % fs_vcb->block_group_size) 
		fs_vcb->block_group_count++;

	int gdt_bytes = fs_vcb->block_group_count * sizeof(struct block_group_desc);
	fs_vcb->gdt_size = bytes_to_blocks(gdt_bytes, block_size);

	if (strlen(name) > 63)
		return 1;
	strcpy(fs_vcb->volume_name, name);

	bfs_generate_uuid(fs_vcb->uuid);

	return 0;
}

/*
 * Create the group descriptor table
 * Group descriptor table contains block_size / 24 block groups
 * Each block group contains BLOCK_SIZE * 8 blocks.
 *
 * uint8_t* gdt: buffer to store gdt data  
 * uint8_t  num_blocks: number of blocks in the system 
 */
int bfs_gdt_init(struct block_group_desc* gdt) 
{
	// 0th block is VCB, block after the VCB is the GDT
	int lba_pos = 1;

	// first block is directly after the gdt
	int block_group_pos = fs_vcb->gdt_size + 1; 

	for (int i = 0; i < fs_vcb->block_group_count; i++) {

		struct block_group_desc descriptor;
		descriptor.bitmap_location = block_group_pos;
		descriptor.free_blocks_count = fs_vcb->block_group_size;
		descriptor.dirs_count = 0;

		uint8_t* bitmap = calloc(fs_vcb->block_size, 1);
		// set first block as used
		bitmap[0] = bit_set(bitmap[0], 0);
		if (LBAwrite(bitmap, 1, block_group_pos) != 1) {
			fprintf(stderr, "Error: Unable to LBAwrite bitmap %d to disk\n", 
					block_group_pos);
			return 1;
		}

		gdt[i] = descriptor;
		block_group_pos += fs_vcb->block_group_size;
	}
	return 0;
}


/*
 * Initialize a directory entry. 
 */
int create_dentry(struct direntry_s* dentry, char* name, int size, int type) 
{
	//dentry->vcb = (uint64_t) &vcb;
	dentry->size = size;
	dentry->file_type = type;

	// block size is size / BLOCK_SIZE, plus one block if there is a remainder
	dentry->num_blocks = size / fs_vcb->block_size;
	if (size % fs_vcb->block_size != 0)
		dentry->num_blocks++;

	time_t current_time = time(NULL);
	dentry->date_created = current_time;
	dentry->date_modified = current_time;
	dentry->date_accessed = current_time;

	if (strlen(name) > MAX_FILENAME_LEN - 1) {
		return 1;
	}
	strcpy(dentry->name, name);


	return 0;
}

void bfs_generate_uuid(uint8_t* uuid)
{
	// fill buffer with random values
	//get_random_bytes(uuid, 16);
	getrandom(uuid, 16, 0);

	// set the four most significant bits of the 7th byte to 0100
	uuid[6] &= 0x0F;
	uuid[6] |= 0x40;

	// set the two most significant bits of the 9th byte to 10
	uuid[8] &= 0x3F;
	uuid[8] |= 0x80;
}

int init_directory(int is_root)
{

}

/*
 * Return lba position of first available block or -1
 */
int bfs_get_first_block(struct block_group_desc* gdt) 
{
	
	for (int i = 0; i < fs_vcb->block_group_count; i++) {
		if (gdt[i].free_blocks_count > 0) {

			uint8_t* bitmap = malloc(fs_vcb->block_size);
			LBAread(bitmap, 1, gdt[i].bitmap_location);

			return get_empty_block(bitmap, fs_vcb->block_size);
		}
	}

	return -1;
}

