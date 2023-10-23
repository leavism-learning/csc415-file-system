#include "bfs.c"

int init_filesystem()
{
	printf("Creating filesystem\n");

	char* name = "my_volume";
	struct vcb_s* vcb = malloc(BLOCK_SIZE);

	if (vcb_init(vcb, name)) {
		printf("Failed to create volume\n");
		return 1;
	} 

	printf("Sucessfully created volume %s\n", vcb->volume_name);

	return 0;
}

int main() 
{
	init_filesystem();
	return 0;
}
