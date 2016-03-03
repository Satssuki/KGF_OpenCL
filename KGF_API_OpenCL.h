/**
This library was created by Marios katsigiannis and is used for the F21DP CW.
Unfortunately due to time constrains, I am unable to add all the features that I want
(multiple program objects, low level data driven configuration, etc...) thus there is
no support for multiple programs or multiple kernels per program. There can only be one
program with one kernel per OCL_API_kernel object.
*/

#ifndef OPENCL_H
#define OPENCL_H



	#ifdef __cplusplus
		extern "C" {
	#endif

			#include <stdint.h>
			#include <CL/cl.h>

			enum OCL {
				KGF_IC_OCL_get_platforms, //get all available platforms
				KGF_IC_OCL_get_devices, //get all available devices
				KGF_IC_OCL_get_context, //creates the context and a command queue
				KGF_IC_OCL_create_buffer, // creates an OpenCL buffer
			};
			enum OCL_API_device_type {
				dt_default = CL_DEVICE_TYPE_DEFAULT,
				dt_cpu = CL_DEVICE_TYPE_CPU,
				dt_gpu = CL_DEVICE_TYPE_GPU,
				//dt_accelarator = CL_DEVICE_TYPE_ACCELERATOR,
				//dt_custom = CL_DEVICE_TYPE_CUSTOM,
				dt_all = CL_DEVICE_TYPE_ALL,
			};
			enum OCL_API_buffer_flags {
				bf_read_write = CL_MEM_READ_WRITE, // the buffer will be retrieved after kernel execution
				bf_write_only = CL_MEM_WRITE_ONLY, // the buffer will be retrieved after kernel execution
				bf_read_only = CL_MEM_READ_ONLY, // the buffer will not be retrieved after kernel execution
				
				//the following will be implemented outside of the CW!

				//bf_use_host = CL_MEM_USE_HOST_PTR,
				//bf_alloc_host = CL_MEM_ALLOC_HOST_PTR,
				//bf_copy_host = CL_MEM_COPY_HOST_PTR,
			};

			struct OCL_API_buffer;
			struct OCL_API_kernel;

			struct OCL_API_platforms {
				cl_platform_id *platforms;
				cl_uint platforms_size; //platforms capacity
				cl_uint num_platforms; //platforms returns, set by the system
				cl_int errcode_ret;
			};
			// zeros out the parameter
			void OCL_API_platforms_init(OCL_API_platforms* platforms); // nulls out everything
			void OCL_API_platforms_set_size(OCL_API_platforms* platforms, uint32_t new_size); // re/allocates space to fill platforms
			cl_platform_id* OCL_API_platforms_get(OCL_API_platforms* platforms, uint32_t index); // retrieves the specific record
			void OCL_API_platforms_clear(OCL_API_platforms* platforms); // deallocates the pointers in the structure


			struct OCL_API_devices {
				OCL_API_platforms* platforms;
				cl_uint selected_platform;
				cl_uint device_type; // check OCL_API_device_type for accptable values and combinations
				cl_device_id *devices;
				cl_uint devices_size; // devices capacity
				cl_uint num_devices; // devices returned
				cl_int errcode_ret;
			};
			// zeros out the parameter, set the platforms, the selected platform and the device_type
			void OCL_API_devices_init(OCL_API_devices* devices, OCL_API_platforms* platforms, cl_uint selected_platform, cl_uint device_type); // nulls out everything
			// zeros out the parameter// zeros out the parameter
			void OCL_API_devices_init_null(OCL_API_devices* devices); // nulls out everything
			//(re-)allocates space for the devices
			void OCL_API_devices_set_size(OCL_API_devices* devices, uint32_t new_size); // re/allocates space to fill devices
			cl_device_id* OCL_API_devices_get(OCL_API_devices* devices, uint32_t index); // retrieves the specific record
			void OCL_API_devices_clear(OCL_API_devices* devices); // deallocates the pointers in the structure

			struct OCL_API_context {
				OCL_API_devices* devices;
				cl_uint selected_device;

				cl_context context;
				cl_command_queue command_queue;

				OCL_API_buffer* buffers; //based on the flags, a buffer will be transfered or not
				cl_uint buffers_size;

				OCL_API_kernel* kernels;
				cl_uint kernels_size;

				cl_int errcode_ret;
			};
			// inits the pointer, to create a context call OCL_create_context
			void OCL_API_context_init(OCL_API_context* c, OCL_API_devices* devices, cl_uint selected_device, cl_uint buffers_size, cl_uint kernels_size);
			//zeros out the pointer
			void OCL_API_context_init_null(OCL_API_context* c);
			//deletes any dynamically allocated memory
			void OCL_API_context_clear(OCL_API_context* c);
			//returns the buffer at index "index", or null if it does not exist
			OCL_API_buffer* OCL_API_context_get_buffer(OCL_API_context* c, cl_int index);
			//returns the kernel at index "index", or null if it does not exist
			OCL_API_kernel* OCL_API_context_get_kernel(OCL_API_context* c, cl_int index);


			struct OCL_API_buffer {
				OCL_API_context* context;
				cl_mem buffer_object; //will be created by the system
				cl_uint flags;
				size_t size;
				void *host_ptr;
				cl_int error_ret;
			};
			void OCL_API_buffer_init(OCL_API_buffer* buffer, OCL_API_context* context, cl_uint flags, size_t size, void *host_ptr); //flags are based on OCL_API_buffer_flags

			struct OCL_API_kernel_argument {
				size_t index;
				size_t size;
				void* ptr;
				bool is_buffer;
			};
			//inits the structure to hold a value. If the argument is a buffer use OCL_API_kernel_argument_init_buffer. To set the argument call OCL_set_kernel_arg
			void OCL_API_kernel_argument_init(OCL_API_kernel_argument* arg, size_t index, size_t size, bool is_buffer, void* ptr);
			//inits the structure to hold a buffer, to set the argument call OCL_set_kernel_arg
			void OCL_API_kernel_argument_init_buffer(OCL_API_kernel_argument* arg, size_t index, OCL_API_buffer* buffer);

			struct OCL_API_kernel {
				OCL_API_context* context;
				cl_program program;
				cl_uint count; // the amount of strings and lengths there is.
				char **sources; //strings containing the kernel source code, null terminated
				size_t *lengths;//the length of the source strings
				cl_kernel kernel;
				cl_int errcode_ret;

				OCL_API_kernel_argument* args;
				size_t args_count;
				char *kernel_name;
				size_t groups_dimension;
				size_t* global;
				size_t* local;

			};
			//inits the kernel. to compile it use OCL_create_kernel
			void OCL_API_kernel_init(OCL_API_kernel* kernel, OCL_API_context* context, char* name, size_t sources_count, size_t args_count, size_t groups_dimension);
			//copies the source code of the Kernel
			bool OCL_API_kernel_set_source(OCL_API_kernel* kernel, size_t index, char* source, size_t length);
			//loads the kernel source code from a file
			bool OCL_API_kernel_load_source(OCL_API_kernel* kernel, size_t index, char* filename);
			// returns the argument structure of the kernel, or null,, if it does not exist
			OCL_API_kernel_argument* OCL_API_kernel_get_argument(OCL_API_kernel* kernel, size_t index);
			//deallocate the memory
			void OCL_API_kernel_destroy(OCL_API_kernel* kernel);



			// use the functions below in the order given, and combine them with the above functions to
			// make OpenCL work
			void OCL_get_platforms(OCL_API_platforms* p); // populate the platforms, the structure must be initialized, and the size set
			void OCL_get_devices(OCL_API_devices* d); // populate the devices, the structure must be initialized, and the size set
			void OCL_create_context(OCL_API_context* c); //creates the context based on the selected context. In this implementation, one context uses only one device
			void OCL_create_buffer(OCL_API_buffer* buffer); //creates the buffer in the context
			void OCL_write_buffer(OCL_API_buffer* buffer);// sends the buffer data from the host to the device
			void OCL_read_buffer(OCL_API_buffer* buffer);// reads the buffer data from the device to the host
			void OCL_read_buffer_partly(OCL_API_buffer* buffer, size_t size); //does not work as intended, do not use
			//OCL_read_buffer_partly(buffer, size) //does not work at all
			void OCL_create_kernel(OCL_API_kernel* kernel);//compiles the code of the kernel, builds the program and sets up the kernel object
			bool OCL_set_kernel_arg(OCL_API_kernel* kernel, OCL_API_kernel_argument* arg); //sends the argument data to the device
			bool OCL_set_kernel_args(OCL_API_kernel* kernel); //calls OCL_set_kernel_arg for all the aeguments of the kernel 
			void OCL_get_kernel_arg(OCL_API_kernel* kernel, OCL_API_kernel_argument* arg); //retrieves the kernel argument from the device. only acts on buffers
			void OCL_get_kernel_arg_part(OCL_API_kernel* kernel, OCL_API_kernel_argument* arg, size_t size);//does not work as intended, do not use
			void OCL_get_kernel_args(OCL_API_kernel* kernel);//calls OCL_get_kernel_arg for all the aeguments of the kernel 
			//this is the final needed call: RUN IT
			void OCL_run_kernel(OCL_API_kernel* kernel); //runs the kernel. Its arguments must have been set prior to calling this function
			//destroy stuff here first:
			void OCL_release_kernel(OCL_API_kernel* kernel); //deletes the kernel from the context, call OCL_kernel_destoy to release from host
			void OCL_release_buffer(OCL_API_buffer* buffer);//deletes the buffer from the context, call OCL_buffer_destoy to release from host
			void OCL_release_context(OCL_API_context* c); //releases the context. call OCL_API_context_clear to deallocate

			#if !defined(_WIN32) && defined(__GNUC__)
				void OCL_kernel_run_timed(OCL_API_kernel* kernel, timespec* start, timespec* stop);
			#endif
	#ifdef __cplusplus
		}
	#endif

#endif
