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

#include "runtime.h"

// Packs a world struct.
cl_world PackWorld(_cl_world *_world) { return (cl_world)_world; }

// Unpacks a world struct.
_cl_world *UnpackWorld(cl_world world) { return (_cl_world *)world; }

// Packs an engine struct.
cl_engine PackEngine(_cl_engine *_engine) { return (cl_engine)_engine; }

// Unpacks an engine struct.
_cl_engine *UnpackEngine(cl_engine engine) { return (_cl_engine *)engine; }

// Transforms an engine to the world.
cl_world EngineToWorld(cl_engine engine) { return UnpackEngine(engine)->world; }

// Creates the world struct.
cl_world CreateWorld() {
  _cl_world *_world = (_cl_world *)malloc(sizeof(_cl_world));

  return PackWorld(_world);
}

// Obtains the platform id.
void GetPlatformID(cl_world world) {
  _cl_world *_world = UnpackWorld(world);

  _world->platform_id = INclGetPlatformID();
}

// Obtains the specified device id.
void GetDeviceID(cl_world world, cl_uint id) {
  _cl_world *_world = UnpackWorld(world);

  _world->device_id = INclGetDeviceID(_world->platform_id, id);
}

// Creates the context.
void CreateContext(cl_world world) {
  _cl_world *_world = UnpackWorld(world);

  _world->context = INclCreateContext(_world->device_id);
}

// Creates a program with the specified name.
void CreateProgram(cl_world world, const char *bitstream_name) {
  _cl_world *_world = UnpackWorld(world);

  _world->program = INclCreateProgramWithBinary(
      _world->context, 1, &_world->device_id, bitstream_name);

  INclBuildProgram(_world->program);
}

// Creates a command queue.
cl_command_queue CreateCommandQueue(cl_world world) {
  _cl_world *_world = UnpackWorld(world);

  return INclCreateCommandQueue(_world->context, _world->device_id);
}

// Blocks until all tasks in a command queue have been completed.
void BlockCommandQueue(cl_command_queue command_queue) {
  INclFlush(command_queue);
  INclFinish(command_queue);
}

// Releases a command queue.
void ReleaseCommandQueue(cl_command_queue command_queue) {
  BlockCommandQueue(command_queue);

  INclReleaseCommandQueue(command_queue);
}

// Allocates a memory buffer.
void *CreateBuffer(cl_world world, size_t size, cl_uint memory) {
  _cl_world *_world = UnpackWorld(world);

  cl_uint CL_MEM_EXT_PTR = 1 << 31;

  typedef struct {
    unsigned flags;
    void *obj;
    void *param;
  } cl_mem_ext_ptr_t;

  cl_uint CL_MEMORY = 1 << memory;

  cl_mem_ext_ptr_t buffer;
  buffer.flags = CL_MEMORY;
  buffer.obj = NULL;
  buffer.param = 0;

  return (void *)INclCreateBuffer(
      _world->context, CL_MEM_READ_WRITE | CL_MEM_EXT_PTR, size, &buffer);
}

// Enqueues a memory copy operation to device.
void EnqueueMemcpyTo(cl_command_queue command_queue, void *dst_ptr,
                     size_t offset, void *src_ptr, size_t size) {
  INclEnqueueWriteBuffer(command_queue, (cl_mem)dst_ptr, offset, size, src_ptr,
                         0, NULL, NULL);
}

// Enqueues a memory copy operation from device.
void EnqueueMemcpyFrom(cl_command_queue command_queue, void *src_ptr,
                       size_t offset, void *dst_ptr, size_t size) {
  INclEnqueueReadBuffer(command_queue, (cl_mem)src_ptr, offset, size, dst_ptr,
                        0, NULL, NULL);
}

// Frees a memory buffer.
void ReleaseBuffer(cl_world world, void *ptr) {
  INclReleaseMemObject((cl_mem)ptr);
}

// Creates a kernel with the specified name.
cl_kernel CreateKernel(cl_world world, const char *kernel_name) {
  _cl_world *_world = UnpackWorld(world);

  return INclCreateKernel(_world->program, kernel_name);
}

// Sets a pointer kernel argument.
void SetKernelArgPointer(cl_kernel kernel, cl_uint arg_index,
                         const void *arg_value) {
  INclSetKernelArg(kernel, arg_index, sizeof(cl_mem), &arg_value);
}

// Sets a scalar kernel argument.
void SetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size,
                  const void *arg_value) {
  INclSetKernelArg(kernel, arg_index, arg_size, arg_value);
}

// Enqueues a kernel operation (Task mode).
void EnqueueKernel(cl_command_queue command_queue, cl_kernel kernel) {
  INclEnqueueTask(command_queue, kernel, 0, NULL, NULL);
}

// Enqueues a kernel operation (NDRangeKernel mode).
void EnqueueKernel(cl_command_queue command_queue, cl_kernel kernel,
                   const size_t *global_work_size,
                   const size_t *local_work_size) {
  INclEnqueueNDRangeKernel(command_queue, kernel, 3, global_work_size,
                           local_work_size, 0, NULL, NULL);
}

// Releases a kernel.
void ReleaseKernel(cl_kernel kernel) { INclReleaseKernel(kernel); }

// Creates an engine struct with the specified name.
cl_engine CreateEngine(cl_world world, const char *kernel_name) {
  _cl_engine *_engine = (_cl_engine *)malloc(sizeof(_cl_engine));

  _engine->world = world;

  _engine->command_queue = CreateCommandQueue(world);
  _engine->kernel = CreateKernel(world, kernel_name);

  return PackEngine(_engine);
}

// Blocks until all tasks in an engine struct have been completed.
void BlockEngine(cl_engine engine) {
  _cl_engine *_engine = UnpackEngine(engine);

  BlockCommandQueue(_engine->command_queue);
}

// Sets a pointer engine struct argument.
void SetEngineArgPointer(cl_engine engine, cl_uint arg_index,
                         const void *arg_value) {
  _cl_engine *_engine = UnpackEngine(engine);

  SetKernelArgPointer(_engine->kernel, arg_index, arg_value);
}

// Sets a scalar engine struct argument.
void SetEngineArg(cl_engine engine, cl_uint arg_index, size_t arg_size,
                  const void *arg_value) {
  _cl_engine *_engine = UnpackEngine(engine);

  SetKernelArg(_engine->kernel, arg_index, arg_size, arg_value);
}

// Enqueues an engine struct operation (Task mode).
void EnqueueEngine(cl_engine engine) {
  _cl_engine *_engine = UnpackEngine(engine);

  EnqueueKernel(_engine->command_queue, _engine->kernel);
}

// Enqueues an engine struct operation (NDRangeKernel mode).
void EnqueueEngine(cl_engine engine, const size_t *global_work_size,
                   const size_t *local_work_size) {
  _cl_engine *_engine = UnpackEngine(engine);

  EnqueueKernel(_engine->command_queue, _engine->kernel, global_work_size,
                local_work_size);
}

// Releases an engine struct.
void ReleaseEngine(cl_engine engine) {
  _cl_engine *_engine = UnpackEngine(engine);

  ReleaseCommandQueue(_engine->command_queue);
  ReleaseKernel(_engine->kernel);

  free(_engine);
}

// Releases a program.
void ReleaseProgram(cl_world world) {
  _cl_world *_world = UnpackWorld(world);

  INclReleaseProgram(_world->program);
}

// Releases the context.
void ReleaseContext(cl_world world) {
  _cl_world *_world = UnpackWorld(world);

  INclReleaseContext(_world->context);
}

// Releases the world struct.
void ReleaseWorld(cl_world world) {
  _cl_world *_world = UnpackWorld(world);

  free(_world);
}
