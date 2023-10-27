/*
 * Functions for BFS system.
 * Author: Griffin Evans
 */
#include "bfs.h"

/*
 * Creates a volume with the given name. Returns 0 on success, non-zero on failure
 */
int vcb_init(struct vcb_s* vcb, char* name, uint64_t num_blocks, uint64_t block_size) 
{
	vcb->block_size   	= block_size;
	vcb->block_count  	= num_blocks;
	vcb->magic 	  	= 0x4465657A;
	vcb->block_group_size 	= block_size * 8;
	vcb->block_group_count  = num_blocks / vcb->block_group_size;
	if (num_blocks % vcb->block_group_size) 
		vcb->block_group_count++;
	vcb->gdt_size		= block_size / sizeof(struct block_group_desc);
	if (block_size % sizeof(struct block_group_desc))
		vcb->gdt_size++;
	

	if (strlen(name) > 63)
		return 1;
	strcpy(vcb->volume_name, name);

	bfs_generate_uuid(vcb->uuid);

	return 0;
}

/*
 * Create the group descriptor table
 * Group descriptor table contains block_size / 24 block groups
 * Each block group contains BLOCK_SIZE * 8 blocks.
 *
 * vcb_s*   vcb: volume control block with essential system info
 * uint8_t* gdt: buffer to store gdt data  
 * uint8_t  num_blocks: number of blocks in the system 
 */
void init_gdt(struct vcb_s* vcb, struct block_group_desc* gdt) 
{
	// 0th block is VCB, block after the VCB is the GDT
	int lba_pos = 1;

	int block_group_pos = vcb->gdt_size; 

	for (int i = 0; i < vcb->block_group_count; i++) {

		struct block_group_desc descriptor;
		descriptor.bitmap_location = block_group_pos;
		descriptor.free_blocks_count = vcb->block_group_size;
		descriptor.dirs_count = 0;

		// TODO: init bitmap

		gdt[i] = descriptor;
		block_group_pos += vcb->block_group_size;
	}
}


/*
 * Initialize a directory entry. 
 */
int create_dentry(struct vcb_s* vcb, struct direntry_s* dentry, char* name, int size, int type) 
{
	//dentry->vcb = (uint64_t) &vcb;
	dentry->size = size;
	dentry->file_type = type;

	// block size is size / BLOCK_SIZE, plus one block if there is a remainder
	dentry->num_blocks = size / vcb->block_size;
	if (size % vcb->block_size != 0)
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
