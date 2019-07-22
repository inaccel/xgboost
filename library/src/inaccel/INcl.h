/*
Copyright © 2019 InAccel

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef INCL_H
#define INCL_H

#include <CL/opencl.h>
#include <stdint.h>

// InAccelCL world struct (Type).
typedef struct{
	cl_platform_id platform_id;
	cl_device_id device_id;
	cl_context context;
	cl_program program;
} _cl_world;

// InAccelCL world struct (API Type).
typedef uintptr_t cl_world;

// InAccelCL engine struct (Type).
typedef struct{
	cl_world world;

	cl_command_queue command_queue;
	cl_kernel kernel;
} _cl_engine;

// InAccelCL engine struct (API Type).
typedef uintptr_t cl_engine;

// Builds a program executable from the program binary.
void INclBuildProgram(cl_program program);

// Creates a buffer object.
cl_mem INclCreateBuffer(cl_context context, cl_mem_flags flags, size_t size, void *host_ptr);

// Create a command-queue on a specific device.
cl_command_queue INclCreateCommandQueue(cl_context context, cl_device_id device);

// Creates an OpenCL context.
cl_context INclCreateContext(const cl_device_id device);

// Creates a kernel object.
cl_kernel INclCreateKernel(cl_program program, const char *kernel_name);

// Creates a program object for a context, and loads specified binary data into the program object.
cl_program INclCreateProgramWithBinary(cl_context context, cl_uint num_devices, const cl_device_id *device_list, const char *binary_name);

// Enqueues a command to map a region of the buffer object given by buffer into the host address space and returns a pointer to this mapped region.
void *INclEnqueueMapBuffer(cl_command_queue command_queue, cl_mem buffer, cl_map_flags map_flags, size_t cb, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);

// Enqueues a command to indicate which device a set of memory objects should be associated with.
void INclEnqueueMigrateMemObjects(cl_command_queue command_queue, cl_uint num_mem_objects, const cl_mem *mem_objects, cl_mem_migration_flags flags, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);

// Enqueues a command to execute a kernel on a device.
void INclEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t *global_work_size, const size_t *local_work_size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);

// Enqueue commands to read from a buffer object to host memory.
void INclEnqueueReadBuffer(cl_command_queue command_queue, cl_mem buffer, size_t offset, size_t cb, void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);

// Enqueues a command to execute a kernel on a device.
void INclEnqueueTask(cl_command_queue command_queue, cl_kernel kernel, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);

// Enqueue commands to write to a buffer object from host memory.
void INclEnqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer, size_t offset, size_t cb, const void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);

// Blocks until all previously queued OpenCL commands in a command-queue are issued to the associated device and have completed.
void INclFinish(cl_command_queue command_queue);

// Issues all previously queued OpenCL commands in a command-queue to the device associated with the command-queue.
void INclFlush(cl_command_queue command_queue);

// Obtain specified device, if available.
cl_device_id INclGetDeviceID(cl_platform_id platform, cl_uint id);

// Obtain the list of devices available on a platform.
void INclGetDeviceIDs(cl_platform_id platform, cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices);

// Get specific information about the OpenCL device.
void INclGetDeviceInfo(cl_device_id device, cl_device_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);

// Obtain platform, if available.
cl_platform_id INclGetPlatformID();

// Obtain the list of platforms available.
void INclGetPlatformIDs(cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms);

// Get specific information about the OpenCL platform.
void INclGetPlatformInfo(cl_platform_id platform, cl_platform_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);

// Decrements the command_queue reference count.
void INclReleaseCommandQueue(cl_command_queue command_queue);

// Decrement the context reference count.
void INclReleaseContext(cl_context context);

// Decrements the event reference count.
void INclReleaseEvent(cl_event event);

// Decrements the kernel reference count.
void INclReleaseKernel(cl_kernel kernel);

// Decrements the memory object reference count.
void INclReleaseMemObject(cl_mem memobj);

// Decrements the program reference count.
void INclReleaseProgram(cl_program program);

// Used to set the argument value for a specific argument of a kernel.
void INclSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value);

// Waits on the host thread for commands identified by event objects to complete.
void INclWaitForEvents(cl_uint num_events, const cl_event *event_list);

// Returns a message related to the error code.
const char *INclCheckErrorCode(cl_int errcode);

#endif
