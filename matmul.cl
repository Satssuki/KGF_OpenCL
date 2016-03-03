__kernel void matmul(
	__global float* inputA,
	__global float* inputB,
	__global float* output,
	const unsigned int count)
{
	int tx = get_global_id(0);
	int ty = get_global_id(1);
	output[ty * count + tx] = 0.0;
	for(int j=0; j<count; j++)
		output[ty * count + tx] += inputA[ty * count + j] * inputB[j * count + tx];
}
