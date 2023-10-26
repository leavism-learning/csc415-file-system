#include "bfs.h"

/*
 * Creates a volume with the given name. Returns 0 on success, non-zero on failure
 */
int vcb_init(struct vcb_s* vcb, char* name) 
{
	vcb->block_size   = BLOCK_SIZE;
	vcb->block_count  = BLOCK_COUNT;
	vcb->free_blocks  = BLOCK_COUNT;
	vcb->block_head   = 1;        
	vcb->magic 	  = 0x4465657A;

	if (strlen(name) > 63)
		return 1;

	strcpy(vcb->volume_name, name);

	bfs_generate_uuid(vcb->uuid);

	return 0;
}

/*
 * Initialize a directory entry. 
 */
int create_dentry(struct vcb_s* vcb, struct direntry_s* dentry, char* name, int size, int type) 
{
	dentry->vcb = (uint64_t) &vcb;
	dentry->size = size;
	dentry->file_type = type;

	// block size is size / BLOCK_SIZE, plus one block if there is a remainder
	dentry->num_blocks = size / BLOCK_SIZE;
	if (size % BLOCK_SIZE != 0)
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

