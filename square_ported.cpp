#include "KGF_OpenCL_easy.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>


void square_ported() {
	const cl_uint MySize = 10240000;

	// create our input
	float *input, *output;
	input = new float[MySize];
	output = new float[MySize];
	for (int i = 0; i < MySize; i++)
		input[i] = rand() / (float)RAND_MAX;


	//setup opencl
	void *all_buffers[] = { input, output };
	size_t all_buffer_sizes[] = { MySize * sizeof(float), MySize * sizeof(float) };
	OCL_API_buffer_flags all_buffer_flags[] = { bf_read_only, bf_write_only };
	char *kernel_filenames[] = { "square.cl", };
	char *kernel_names[] = { "square", };
	size_t kernel_args_count[] = { 3 };

	OCL_easy ocl;
	// GPU based openCL, 2 buffers, 1 kernel
	if (!OCL_easy_init(&ocl, dt_gpu, /*buffers*/2, /*kernels*/1, all_buffers,
						all_buffer_sizes, all_buffer_flags, kernel_filenames, kernel_names,
						kernel_args_count, 1))
		return;
	//set the work group
	ocl.context.kernels[0].global[0] = MySize;
	ocl.context.kernels[0].local[0] = 32;

	//set the arg values
	OCL_API_kernel_argument* arg;
	//setup input and output
	size_t argi;
	for (argi = 0; argi < 2; argi++) {
		arg = OCL_API_kernel_get_argument(&ocl.context.kernels[0], argi);
		if(arg)
			OCL_API_kernel_argument_init_buffer(arg, argi, &ocl.context.buffers[argi]);
	}
	//setup count
	arg = OCL_API_kernel_get_argument(&ocl.context.kernels[0], 2);
	if (arg)
		OCL_API_kernel_argument_init(arg, argi, sizeof(cl_uint), false, (void*)&MySize);
	//run it
	OCL_easy_run(&ocl, 0);
	printKernelTime(&ocl);
	// we now have the results right into the result buffers
	OCL_easy_destroy(&ocl);

	float t;
	long correct = 0;
	for (int i = 0; i < MySize; i++)
		if (output[i] == input[i] * input[i])
			correct++;
	/* Print a brief summary detailing the results.  */
	printf("Computed %d/%d %2.0f%% correct values\n", correct, MySize,
		MySize / correct*100.f);

	delete[] input;
	delete[] output;
}

/*
int main(int argc, char** argv) {
	square_ported();
	return 0;
}
*/