#include "KGF_OpenCL_easy.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

void matmul_ported() {
	const cl_uint MySize = 1024;
	
	// create our input
	float *in_a = new float[MySize * MySize];                /* Original data set given to device.  */
	float *in_b = new float[MySize * MySize];                /* Original data set given to device.  */
	float *out = new float[MySize * MySize];             /* Results returned from device.  */
	int correct;                       /* Number of correct results returned.  */
	for (int i = 0; i < MySize*MySize; i++) {
		in_a[i] = rand() / (float)RAND_MAX;
		in_b[i] = rand() / (float)RAND_MAX;
		out[i] = 0;
	}

	//setup opencl
	void *all_buffers[] = { in_a, in_b, out };
	size_t all_buffer_sizes[] = { MySize * MySize * sizeof(float), MySize * MySize * sizeof(float), MySize * MySize * sizeof(float) };
	OCL_API_buffer_flags all_buffer_flags[] = { bf_read_only, bf_read_only, bf_write_only };
	char *kernel_filenames[] = { "matmul.cl", };
	char *kernel_names[] = { "matmul", };
	size_t kernel_args_count[] = { 4 };

	OCL_easy ocl;
	// GPU based openCL, 3 buffers, 1 kernel
	if (!OCL_easy_init(&ocl, dt_gpu, /*buffers*/3, /*kernels*/1, all_buffers,
		all_buffer_sizes, all_buffer_flags, kernel_filenames, kernel_names,
		kernel_args_count, 2)) {
		system("PAUSE");
		return;
	}
	//set the work group
	ocl.context.kernels[0].global[0] = MySize;
	ocl.context.kernels[0].global[1] = MySize;
	ocl.context.kernels[0].local[0] = MySize / 32;
	ocl.context.kernels[0].local[1] = MySize / 32;

	//set the arg values
	OCL_API_kernel_argument* arg;
	//setup input and output
	size_t argi;
	for (argi = 0; argi < 3; argi++) {
		arg = OCL_API_kernel_get_argument(&ocl.context.kernels[0], argi);
		if (arg)
			OCL_API_kernel_argument_init_buffer(arg, argi, &ocl.context.buffers[argi]);
	}
	//setup count
	arg = OCL_API_kernel_get_argument(&ocl.context.kernels[0], 3);
	if (arg)
		OCL_API_kernel_argument_init(arg, argi, sizeof(cl_uint), false, (void*)&MySize);
	//run it
	OCL_easy_run(&ocl, 0);
	printKernelTime(&ocl);
	// we now have the results right into the result buffers
	OCL_easy_destroy(&ocl);

	/* Validate our results.  */
	correct = 0;
	float sum;
	for (int i = 0; i < MySize; i++) {
		for (int j = 0; j < MySize; j++) {
			sum = 0.0;
			for (int k = 0; k < MySize; k++) {
				sum += in_a[i*MySize + k] * in_b[k*MySize + j];
			}
			if (fabs(out[i*MySize + j] - sum) < 0.0001)
				correct++;
		}
	}
	/* Print a brief summary detailing the results.  */
	printf("Computed %d/%d %2.0f%% correct values\n", correct, MySize*MySize,
		(float)(MySize*MySize) / correct*100.f);


	delete[] in_a, in_b, out;
}
/*
int main() {
	matmul_ported();
	return 0;
}
*/