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

#ifndef RUNTIME_H
#define RUNTIME_H

#include "INcl.h"

// Packs a world struct.
cl_world PackWorld(_cl_world *_world);

// Unpacks a world struct.
_cl_world *UnpackWorld(cl_world world);

// Packs an engine struct.
cl_engine PackEngine(_cl_engine *_engine);

// Unpacks an engine struct.
_cl_engine *UnpackEngine(cl_engine engine);

// Transforms an engine to the world.
cl_world EngineToWorld(cl_engine engine);

// Creates the world struct.
cl_world CreateWorld();

// Obtains the platform id.
void GetPlatformID(cl_world world);

// Obtains the specified device id.
void GetDeviceID(cl_world world, cl_uint id);

// Creates the context.
void CreateContext(cl_world world);

// Creates a program with the specified name.
void CreateProgram(cl_world world, const char *bitstream_name);

// Creates a command queue.
cl_command_queue CreateCommandQueue(cl_world world);

// Blocks until all tasks in a command queue have been completed.
void BlockCommandQueue(cl_command_queue command_queue);

// Releases a command queue.
void ReleaseCommandQueue(cl_command_queue command_queue);

// Allocates a memory buffer.
void *CreateBuffer(cl_world world, size_t size, cl_uint memory);

// Enqueues a memory copy operation to device.
void EnqueueMemcpyTo(cl_command_queue command_queue, void *dst_ptr, size_t offset, void *src_ptr, size_t size);

// Enqueues a memory copy operation from device.
void EnqueueMemcpyFrom(cl_command_queue command_queue, void *src_ptr, size_t offset, void *dst_ptr, size_t size);

// Frees a memory buffer.
void ReleaseBuffer(cl_world world, void *ptr);

// Creates a kernel with the specified name.
cl_kernel CreateKernel(cl_world world, const char *kernel_name);

// Sets a pointer kernel argument.
void SetKernelArgPointer(cl_kernel kernel, cl_uint arg_index, const void *arg_value);

// Sets a scalar kernel argument.
void SetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value);

// Enqueues a kernel operation (Task mode).
void EnqueueKernel(cl_command_queue command_queue, cl_kernel kernel);

// Enqueues a kernel operation (NDRangeKernel mode).
void EnqueueKernel(cl_command_queue command_queue, cl_kernel kernel, const size_t *global_work_size, const size_t *local_work_size);

// Releases a kernel.
void ReleaseKernel(cl_kernel kernel);

// Creates an engine struct with the specified name.
cl_engine CreateEngine(cl_world world, const char *kernel_name);

// Blocks until all tasks in an engine struct have been completed.
void BlockEngine(cl_engine engine);

// Sets a pointer engine struct argument.
void SetEngineArgPointer(cl_engine engine, cl_uint arg_index, const void *arg_value);

// Sets a scalar engine struct argument.
void SetEngineArg(cl_engine engine, cl_uint arg_index, size_t arg_size, const void *arg_value);

// Enqueues an engine struct operation (Task mode).
void EnqueueEngine(cl_engine engine);

// Enqueues an engine struct operation (NDRangeKernel mode).
void EnqueueEngine(cl_engine engine, const size_t *global_work_size, const size_t *local_work_size);

// Releases an engine struct.
void ReleaseEngine(cl_engine engine);

// Releases a program.
void ReleaseProgram(cl_world world);

// Releases the context.
void ReleaseContext(cl_world world);

// Releases the world struct.
void ReleaseWorld(cl_world world);

#endif
