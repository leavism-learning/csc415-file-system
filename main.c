#include "bfs.c"

int main() 
{
	struct vcb_s* vcb = malloc(sizeof(struct vcb_s));
	char* name = "my_volume";
	if (vcb_init(vcb, name)) {
		printf("Failed to create volume\n");
	} else {
		printf("Sucessfully created volume %s\n", vcb->volume_name);
	}

	return 0;
}
