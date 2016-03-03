#ifndef KGF_OPENCL_EASY_H
#define KGF_OPENCL_EASY_H

	#include <time.h>
	#include "KGF_API_OpenCL.h"

	#if defined(WIN32) && !defined(clock_gettime)
	#define clock_gettime(base, ts) timespec_get(ts, base);
	#define CLOCK_PROCESS_CPUTIME_ID TIME_UTC
	#define CLOCK_REALTIME TIME_UTC
	#endif


	struct OCL_easy {
		OCL_API_platforms platforms;
		OCL_API_devices devices;
		OCL_API_context context;
		struct timespec start, stop;
	};
	//initizes the OCL_easy structure and calls all the required OCL_* functions to get a working context
	//it also initializes the required buffers. void** buffers, size_t* buffer_sizes, OCL_API_buffer_flags*
	//buffer_flags must fave the same first dimension equals to buffers_count.
	//Then it initializes the kernels. char** sources_filenames, char** kernel_names, size_t*
	//args_count must have the same dimension equals kernels_count.
	bool OCL_easy_init(OCL_easy* ocl, OCL_API_device_type type, size_t buffers_count, size_t kernels_count,
		void** buffers, size_t* buffer_sizes, OCL_API_buffer_flags* buffer_flags,
		char** sources_filenames, char** kernel_names, size_t* args_count, size_t num_dimensions);
	// releases everything both from the device andd from the host
	void OCL_easy_destroy(OCL_easy* ocl);
	//sends all the arguments for the kernel at index kernel_index, executees it and returns all the buffer
	//arguments that are set with thhe write or read_write flag
	bool OCL_easy_run(OCL_easy* ocl, size_t kernel_index);
	//prints the time the kernel spent executing
	void printKernelTime(OCL_easy* ocl);

#endif