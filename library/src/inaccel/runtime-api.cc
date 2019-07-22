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

#include "runtime-api.h"

// Creates the world.
cl_world InAccel::create_world(int device_id) {
  cl_world world = CreateWorld();

  GetPlatformID(world);

  GetDeviceID(world, (cl_uint)device_id);

  CreateContext(world);

  return world;
}

// Allocates a new buffer.
void *InAccel::malloc(cl_world world, size_t size, int memory_id) {
  return CreateBuffer(world, size, (cl_uint)memory_id);
}

// Transfers data to a previously allocated buffer.
void InAccel::memcpy_to(cl_world world, void *dst_ptr, size_t offset,
                        void *src_ptr, size_t size) {
  cl_command_queue command_queue = CreateCommandQueue(world);

  EnqueueMemcpyTo(command_queue, dst_ptr, offset, src_ptr, size);

  ReleaseCommandQueue(command_queue);
}

// Creates a new program.
void InAccel::create_program(cl_world world, const char *bitstream_name) {
  CreateProgram(world, bitstream_name);
}

// Creates a new egine.
cl_engine InAccel::create_engine(cl_world world, const char *kernel_name) {
  return CreateEngine(world, kernel_name);
}

// Sets an engine argument using a buffer.
void InAccel::set_engine_arg(cl_engine engine, int index, void *buffer) {
  SetEngineArgPointer(engine, (cl_uint)index, buffer);
}

// Sets an engine argument using an int value.
void InAccel::set_engine_arg(cl_engine engine, int index, int value) {
  SetEngineArg(engine, (cl_uint)index, sizeof(int), &value);
}

// Sets an engine argument using a long value.
void InAccel::set_engine_arg(cl_engine engine, int index, long value) {
  SetEngineArg(engine, (cl_uint)index, sizeof(long), &value);
}

// Sets an engine argument using a float value.
void InAccel::set_engine_arg(cl_engine engine, int index, float value) {
  SetEngineArg(engine, (cl_uint)index, sizeof(float), &value);
}

// Sets an engine argument using a double value.
void InAccel::set_engine_arg(cl_engine engine, int index, double value) {
  SetEngineArg(engine, (cl_uint)index, sizeof(double), &value);
}

// Runs an engine.
void InAccel::run_engine(cl_engine engine) { EnqueueEngine(engine); }

// Awaits an engine.
void InAccel::await_engine(cl_engine engine) { BlockEngine(engine); }

// Releases an engine.
void InAccel::release_engine(cl_engine engine) { ReleaseEngine(engine); }

// Releases a program.
void InAccel::release_program(cl_world world) { ReleaseProgram(world); }

// Transfers data from a previously allocated buffer.
void InAccel::memcpy_from(cl_world world, void *src_ptr, size_t offset,
                          void *dst_ptr, size_t size) {
  cl_command_queue command_queue = CreateCommandQueue(world);

  EnqueueMemcpyFrom(command_queue, src_ptr, offset, dst_ptr, size);

  ReleaseCommandQueue(command_queue);
}

// Frees a buffer.
void InAccel::free(cl_world world, void *ptr) { ReleaseBuffer(world, ptr); }

// Releases the world.
void InAccel::release_world(cl_world world) {
  ReleaseContext(world);

  ReleaseWorld(world);
}
