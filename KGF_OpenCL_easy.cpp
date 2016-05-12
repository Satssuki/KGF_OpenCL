#include "KGF_OpenCL_easy.h"
#include <stdio.h>

void logerror(char* msg) {
	fprintf(stderr, msg);
}
void log(char* msg) {
	fprintf(stdout, msg);
}

bool OCL_easy_init(OCL_easy* ocl, OCL_API_device_type type, size_t buffers_count, size_t kernels_count,
	void** buffers, size_t* buffer_sizes, OCL_API_buffer_flags* buffer_flags,
	char** sources_filenames, char** kernel_names, size_t* args_count, size_t num_dimensions)
{
	//get the platforms
	OCL_API_platforms_init(&ocl->platforms);
	OCL_API_platforms_set_size(&ocl->platforms, 1);
	OCL_get_platforms(&ocl->platforms);
	if (ocl->platforms.errcode_ret != CL_SUCCESS) {
		logerror("Failed to query platforms.");
		return false;
	}
	//get the devices
	OCL_API_devices_init(&ocl->devices, &ocl->platforms, 0, type);
	OCL_API_devices_set_size(&ocl->devices, 1);
	OCL_get_devices(&ocl->devices);
	if (ocl->devices.errcode_ret != CL_SUCCESS) {
		logerror("Failed to query devices.");
		return false;
	}
	//create the context
	OCL_API_context_init(&ocl->context, &ocl->devices, 0, buffers_count, kernels_count);
	OCL_create_context(&ocl->context);
	if (!ocl->context.context) {
		logerror("Failed to create context.");
		return false;
	}
	if (ocl->context.errcode_ret != CL_SUCCESS) {
		logerror("Failed to create command queue.");
		return false;
	}
	OCL_API_buffer *buffer;
	for (size_t i = 0; i < ocl->context.buffers_size; i++) {
		buffer = OCL_API_context_get_buffer(&ocl->context, i);
		if (buffer) {
			//initialize the buffer
			OCL_API_buffer_init(buffer, &ocl->context, buffer_flags[i], buffer_sizes[i], buffers[i]); //add ptr and size
			//create the buffer! 
			OCL_create_buffer(buffer);
			if (!buffer->buffer_object || buffer->error_ret != CL_SUCCESS) {
				printf("CL_ERROR:%d while creating buffer:%d", buffer->error_ret, i);
				return false;
			}
		}
	}
	OCL_API_kernel* kernel;
	//initialize, load and create all available kernels
	for (size_t i = 0; i < ocl->context.kernels_size; i++) {
		kernel = OCL_API_context_get_kernel(&ocl->context, i);
		OCL_API_kernel_init(kernel, &ocl->context, kernel_names[i], 1, args_count[i], num_dimensions);
		if (!OCL_API_kernel_load_source(kernel, 0, sources_filenames[i]))
			return false;
		OCL_create_kernel(kernel);
		if (!kernel->kernel || kernel->errcode_ret != CL_SUCCESS)
			return false;
	}
	return true;
}
void OCL_easy_destroy(OCL_easy* ocl) {
	if (!ocl)
		return;
	size_t i;
	for (i = 0; i < ocl->context.kernels_size; i++)
		OCL_release_kernel(&ocl->context.kernels[i]);
	for (i = 0; i < ocl->context.buffers_size; i++)
		OCL_release_buffer(&ocl->context.buffers[i]);
	OCL_release_context(&ocl->context);
}
bool OCL_easy_run(OCL_easy* ocl, size_t kernel_index) {
	OCL_API_kernel* kernel = OCL_API_context_get_kernel(&ocl->context, kernel_index);
	if (!kernel)
		return false;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ocl->start);
	if (!OCL_set_kernel_args(kernel))
		return false;
	OCL_run_kernel(kernel);
	if (kernel->errcode_ret != CL_SUCCESS) //not all args are e=set
		return false;
	OCL_get_kernel_args(kernel); //only write buffers are returned!
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ocl->stop);
	return true; //done!!!!
}
void printKernelTime(OCL_easy* ocl)
{
	double elapsed = (ocl->stop.tv_sec - ocl->start.tv_sec)*1000.0
		+ (ocl->stop.tv_nsec - ocl->start.tv_nsec) / 1000000.0;
	printf("time spent: %f msec\n", elapsed);
}