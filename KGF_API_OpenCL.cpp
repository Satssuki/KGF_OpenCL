#include "KGF_API_OpenCL.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

void OCL_API_platforms_init(OCL_API_platforms* platforms) {
	memset(platforms, 0, sizeof(OCL_API_platforms));
}
void OCL_API_platforms_set_size(OCL_API_platforms* platforms, uint32_t new_size) {
	if (platforms->platforms) {
		delete[] platforms->platforms;
		platforms->platforms = NULL;
	}
	platforms->num_platforms = 0;
	platforms->platforms_size = new_size;
	if (platforms->platforms_size)
		platforms->platforms = new cl_platform_id[platforms->platforms_size];
}
cl_platform_id* OCL_API_platforms_get(OCL_API_platforms* platforms, uint32_t index) {
	if (index < platforms->num_platforms)
		return &platforms->platforms[index];
	return NULL;
}
void OCL_API_platforms_clear(OCL_API_platforms* platforms) {
	OCL_API_platforms_set_size(platforms, 0);
}

void OCL_API_devices_init(OCL_API_devices* devices, OCL_API_platforms* platforms, cl_uint selected_platform, cl_uint device_type) {
	memset(devices, 0, sizeof(OCL_API_devices));
	devices->platforms = platforms;
	devices->selected_platform = selected_platform;
	devices->device_type = device_type;
}
void OCL_API_devices_init_null(OCL_API_devices* devices) {
	OCL_API_devices_init(devices, NULL, 0, dt_default);
}
void OCL_API_devices_set_size(OCL_API_devices* devices, uint32_t new_size) {
	if (devices->devices) {
		delete[] devices->devices;
		devices->devices = NULL;
	}
	devices->num_devices = 0;
	devices->devices_size = new_size;
	if (devices->devices_size)
		devices->devices = new cl_device_id[devices->devices_size];
}
cl_device_id* OCL_API_devices_get(OCL_API_devices* devices, uint32_t index) {
	if (index < devices->num_devices)
		return &devices->devices[index];
	return NULL;
}
void OCL_API_devices_clear(OCL_API_devices* devices) {
	OCL_API_devices_set_size(devices, 0);
}

void OCL_API_context_init(OCL_API_context* c, OCL_API_devices* devices, cl_uint selected_device,
								cl_uint buffers_size, cl_uint kernels_size) {
	memset(c, 0, sizeof(OCL_API_context));
	c->devices = devices;
	c->selected_device = selected_device;
	c->buffers_size = buffers_size;
	if (c->buffers_size)
		c->buffers = new OCL_API_buffer[c->buffers_size];
	c->kernels_size = kernels_size;
	if (c->kernels_size)
		c->kernels = new OCL_API_kernel[c->kernels_size];
}
void OCL_API_context_init_null(OCL_API_context* c) {
	OCL_API_context_init(c, NULL, 0, 0, 0);
}
void OCL_API_context_clear(OCL_API_context* c) {
	if (c->buffers_size)
		delete[] c->buffers;
	if (c->kernels_size)
		delete[] c->kernels;
	c->buffers_size = 0;
	c->kernels_size = 0;
}
OCL_API_buffer* OCL_API_context_get_buffer(OCL_API_context* c, cl_int index) {
	if (!c->buffers || index >= c->buffers_size)
		return NULL;
	return &c->buffers[index];
}
OCL_API_kernel* OCL_API_context_get_kernel(OCL_API_context* c, cl_int index) {
	if (!c->kernels || index >= c->kernels_size)
		return NULL;
	return &c->kernels[index];
}
void OCL_API_buffer_init(OCL_API_buffer* buffer, OCL_API_context* context, cl_uint flags, size_t size, void *host_ptr) {
	memset(buffer, 0, sizeof(OCL_API_buffer));
	buffer->context = context;
	buffer->flags = flags;
	buffer->size = size;
	buffer->host_ptr = host_ptr;
}
void OCL_API_kernel_init(OCL_API_kernel* kernel, OCL_API_context* context, char* name, size_t sources_count, size_t args_count, size_t groups_dimension) {
	memset(kernel, 0, sizeof(OCL_API_kernel));
	kernel->context = context;
	kernel->count = sources_count;
	if (kernel->count) {
		kernel->sources = new char*[kernel->count];
		kernel->lengths = new size_t[kernel->count];
		memset(kernel->lengths, 0, sizeof(size_t) * kernel->count); //set lengths to 0
	}
	if(name) {
		size_t size = strlen(name) + 1;
		kernel->kernel_name = new char[size];
		memcpy(kernel->kernel_name, name, size - 1);
		kernel->kernel_name[size - 1] = 0;
	}
	if (args_count) {
		kernel->args_count = args_count;
		kernel->args = new OCL_API_kernel_argument[kernel->args_count];
	}
	if (groups_dimension) {
		kernel->groups_dimension = groups_dimension;
		kernel->local = new size_t[kernel->groups_dimension];
		kernel->global = new size_t[kernel->groups_dimension];
	}
}
bool OCL_API_kernel_set_source(OCL_API_kernel* kernel, size_t index, char* source, size_t length) {
	if (!kernel || kernel->count <= index)
		return false;
	if(kernel->lengths[index])
		delete[] kernel->sources[index];
	kernel->sources[index] = new char[length + 1];
	kernel->lengths[index] = length;
	if(source && length)
		memcpy(kernel->sources[index], source, sizeof(char)* kernel->lengths[index]);
	kernel->sources[index][kernel->lengths[index]] = 0; //null terminate
	return true;
}
bool OCL_API_kernel_load_source(OCL_API_kernel* kernel, size_t index, char* filename) {
	FILE *f = fopen(filename, "rb");
	if (!f)
		return false;
	fseek(f, 0, SEEK_END); // get to the end
	uint32_t size = ftell(f); //get the position of the end -> filesize
	fseek(f, 0, SEEK_SET); //reset to start in order to read
	OCL_API_kernel_set_source(kernel, index, NULL, size); // create the buffer, but do not fill the data
	fread(kernel->sources[index], 1, size, f); // load the sources directly to the buffer.
	fclose(f);
	return true;
}
OCL_API_kernel_argument* OCL_API_kernel_get_argument(OCL_API_kernel* kernel, size_t index) {
	if (kernel && kernel->args_count > index)
		return &kernel->args[index];
	return NULL;
}
void OCL_API_kernel_destroy(OCL_API_kernel* kernel) {
	if (kernel->count) {
		for (size_t i = 0; i < kernel->count; i++)
			if (kernel->lengths[i])
				delete[] kernel->sources[i];
		delete[] kernel->sources;
		delete[] kernel->lengths;
	}
	kernel->count = 0;
	if (kernel->args)
		delete[] kernel->args;
	if(kernel->kernel_name)
		delete[] kernel->kernel_name;
	if (kernel->global)
		delete[] kernel->global;
	if (kernel->local)
		delete[] kernel->local;
}
void OCL_API_kernel_argument_init(OCL_API_kernel_argument* arg, size_t index, size_t size, bool is_buffer, void* ptr) {
	arg->index = index;
	arg->size = size;
	arg->ptr = ptr;
	arg->is_buffer = is_buffer;
}
void OCL_API_kernel_argument_init_buffer(OCL_API_kernel_argument* arg, size_t index, OCL_API_buffer* buffer) {
	OCL_API_kernel_argument_init(arg, index, 0, true, buffer);
}




void OCL_get_platforms(OCL_API_platforms* p) {
	p->errcode_ret = clGetPlatformIDs(p->platforms_size, p->platforms, &p->num_platforms);
}
void OCL_get_devices(OCL_API_devices* d) {
	cl_platform_id* pid = OCL_API_platforms_get(d->platforms, d->selected_platform);
	if (!pid) {
		d->errcode_ret = CL_INVALID_PLATFORM;
		return;
	}
	d->errcode_ret = clGetDeviceIDs(*pid, (cl_device_type)d->device_type, d->devices_size, d->devices, &d->num_devices);
}
void OCL_create_context(OCL_API_context* c) {
	if (!c->devices ) {
		c->errcode_ret = CL_INVALID_DEVICE;
		return;
	}
	cl_device_id* did = OCL_API_devices_get(c->devices, c->selected_device);
	if (!did) {
		c->errcode_ret = CL_INVALID_DEVICE;
		return;
	}
	//for now only use one device in one context
	c->context = clCreateContext(NULL, 1 /*c->devices->num_devices*/, did /*c->devices->devices*/, NULL, NULL, &c->errcode_ret);
	if (!c->context || c->errcode_ret != CL_SUCCESS)
		return;
	c->command_queue = clCreateCommandQueue(c->context, *did, 0, &c->errcode_ret);
	if (!c->command_queue && c->errcode_ret == CL_SUCCESS)
		c->errcode_ret = CL_INVALID_QUEUE_PROPERTIES; // ensure that a faulty command queue does not go unnoticed
}
void OCL_create_buffer(OCL_API_buffer* buffer) {
	if(buffer->context->context && buffer->size, buffer->host_ptr)
		buffer->buffer_object = clCreateBuffer(buffer->context->context, buffer->flags, buffer->size, NULL, &buffer->error_ret);
}
void OCL_write_buffer(OCL_API_buffer* buffer) {
	if(buffer && buffer->buffer_object && buffer->context && buffer->context->command_queue)
		buffer->error_ret = clEnqueueWriteBuffer(buffer->context->command_queue, buffer->buffer_object,
												CL_TRUE, 0, buffer->size, buffer->host_ptr, 0, NULL, NULL);
}
void OCL_read_buffer(OCL_API_buffer* buffer) {
	buffer->error_ret = clEnqueueReadBuffer(buffer->context->command_queue, buffer->buffer_object, CL_TRUE,
		0, buffer->size, buffer->host_ptr, 0, NULL, NULL);
}
void OCL_read_buffer_partly(OCL_API_buffer* buffer, size_t size) {
	if (size > buffer->size)
		size = buffer->size;
	buffer->error_ret = clEnqueueReadBuffer(buffer->context->command_queue, buffer->buffer_object, CL_TRUE,
		0, size, buffer->host_ptr, 0, NULL, NULL);
}

void OCL_create_kernel(OCL_API_kernel* kernel) {
	kernel->program = clCreateProgramWithSource(kernel->context->context, kernel->count, (const char**)kernel->sources, kernel->lengths, &kernel->errcode_ret);
	if (!kernel->program || kernel->errcode_ret != CL_SUCCESS)
		return;
	kernel->errcode_ret = clBuildProgram(kernel->program, 1, OCL_API_devices_get(kernel->context->devices, kernel->context->selected_device), NULL, NULL, NULL);
	if (kernel->errcode_ret != CL_SUCCESS)
	{
		size_t len;
		char buffer[2048];
		clGetProgramBuildInfo(kernel->program, *OCL_API_devices_get(kernel->context->devices, kernel->context->selected_device), CL_PROGRAM_BUILD_LOG,
			sizeof(buffer), buffer, &len);
		printf("Error: Failed to build program executable!\n%s", buffer);
		return;
	}
	/* Create the compute kernel in the program.  */
	kernel->kernel = clCreateKernel(kernel->program, kernel->kernel_name, &kernel->errcode_ret);
	if (!kernel->kernel || kernel->errcode_ret != CL_SUCCESS) {
		kernel->kernel = NULL;
		printf("Error: Failed to buildfind kernel:%s\n", kernel->kernel_name);
		return;
	}
}

bool OCL_set_kernel_arg(OCL_API_kernel* kernel, OCL_API_kernel_argument* arg) {
	if (!arg->is_buffer)
		kernel->errcode_ret = clSetKernelArg(kernel->kernel, arg->index, arg->size, arg->ptr);
	else {
		OCL_API_buffer* buffer = (OCL_API_buffer*)arg->ptr;
		if (buffer->flags & bf_read_write || buffer->flags & bf_read_only) {
			OCL_write_buffer(buffer);
			if (buffer->error_ret != CL_SUCCESS)
				return false;
		}
		kernel->errcode_ret = clSetKernelArg(kernel->kernel, arg->index, sizeof(cl_mem), &buffer->buffer_object);
	}
	return !kernel->errcode_ret;
}
bool OCL_set_kernel_args(OCL_API_kernel* kernel) {
	for (size_t i = 0; i < kernel->args_count; i++)
		if(!OCL_set_kernel_arg(kernel, &kernel->args[i]))
			return false;
	return true;
}
void OCL_get_kernel_arg(OCL_API_kernel* kernel, OCL_API_kernel_argument* arg) {
	if (arg->is_buffer) {
		OCL_API_buffer* buffer = (OCL_API_buffer*)arg->ptr;
 		if (buffer->flags & bf_read_write || buffer->flags & bf_write_only)
			OCL_read_buffer(buffer);
	}
}
void OCL_get_kernel_arg_part(OCL_API_kernel* kernel, OCL_API_kernel_argument* arg, size_t size) {
	if (arg->is_buffer) {
		OCL_API_buffer* buffer = (OCL_API_buffer*)arg->ptr;
		if (buffer->flags & bf_read_write || buffer->flags & bf_write_only)
			OCL_read_buffer_partly(buffer, size);
	}
}
void OCL_get_kernel_args(OCL_API_kernel* kernel) {
	for (size_t i = 0; i < kernel->args_count; i++)
		OCL_get_kernel_arg(kernel, &kernel->args[i]);
}
void OCL_run_kernel(OCL_API_kernel* kernel) {
	//clock_gettime(CLOCK_REALTIME, &start);
	kernel->errcode_ret = clEnqueueNDRangeKernel(kernel->context->command_queue, kernel->kernel,
		kernel->groups_dimension, NULL, kernel->global, kernel->local, 0, NULL, NULL);
	if (kernel->errcode_ret != CL_SUCCESS)
		return;
	/* Wait for all commands to complete.  */
	kernel->errcode_ret = clFinish(kernel->context->command_queue);
	//clock_gettime(CLOCK_REALTIME, &stop);
}
#if !defined(_WIN32) && defined(__GNUC__)
void OCL_kernel_run_timed(OCL_API_kernel* kernel, timespec* start, timespec* stop) {
	clock_gettime(CLOCK_REALTIME, start);
	OCL_run_kernel(kernel);
	clock_gettime(CLOCK_REALTIME, stop);
}
#endif

void OCL_release_kernel(OCL_API_kernel* kernel) {
	kernel->errcode_ret = clReleaseKernel(kernel->kernel);
	kernel->errcode_ret = clReleaseProgram(kernel->program);
}
void OCL_release_buffer(OCL_API_buffer* buffer) {
	buffer->error_ret = clReleaseMemObject(buffer->buffer_object);
}
void OCL_release_context(OCL_API_context* c) {
	c->errcode_ret = clReleaseCommandQueue(c->command_queue);
	c->errcode_ret = clReleaseContext(c->context);
}
