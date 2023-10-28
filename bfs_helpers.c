int bytes_to_blocks(int bytes, int block_size)
{
	return (bytes + block_size - 1) / block_size;
}
