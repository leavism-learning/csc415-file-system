#include "bfs.h"
#include "fsLow.h"

// initialize the filesystem after boot
// read first block of memory. if it is a valid VCB proceed, otherwise create a new VCB.
int init_filesystem()
{
	printf("Creating filesystem\n");

	struct vcb_s* vcb = malloc(BLOCK_SIZE);

	// read first block of memory
	LBAread(vcb, 1, 0);

	if (vcb->magic == 0x434465657A) {

		printf("Existing filesystem found!\n");

	} else { 

		char* volume_name = "volume0";

		if (vcb_init(vcb, volume_name)) {
			printf("Failed to create volume\n");
			return 1;
		} 

		printf("Sucessfully created volume %s\n", vcb->volume_name);

		if (LBAwrite(vcb, 1, 0) != 1) {
			fprintf(stderr, "Error: Unable to LBAwrite VCB to disk\n");
			return 1;
		}
	}

	return 0;
}

int main() 
{
	init_filesystem();
	return 0;
}
