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

#ifndef RUNTIME_API_H
#define RUNTIME_API_H

#include "runtime.h"

// InAccel function calls.
class InAccel {

public:
  // Creates the world.
  static cl_world create_world(int device_id);

  // Allocates a new buffer.
  static void *malloc(cl_world world, size_t size, int memory_id);

  // Transfers data to a previously allocated buffer.
  static void memcpy_to(cl_world world, void *dst_ptr, size_t offset,
                        void *src_ptr, size_t size);

  // Creates a new program.
  static void create_program(cl_world world, const char *bitstream_name);

  // Creates a new egine.
  static cl_engine create_engine(cl_world world, const char *kernel_name);

  // Sets an engine argument using a buffer.
  static void set_engine_arg(cl_engine engine, int index, void *buffer);

  // Sets an engine argument using an int value.
  static void set_engine_arg(cl_engine engine, int index, int value);

  // Sets an engine argument using a long value.
  static void set_engine_arg(cl_engine engine, int index, long value);

  // Sets an engine argument using a float value.
  static void set_engine_arg(cl_engine engine, int index, float value);

  // Sets an engine argument using a double value.
  static void set_engine_arg(cl_engine engine, int index, double value);

  // Runs an engine.
  static void run_engine(cl_engine engine);

  // Awaits an engine.
  static void await_engine(cl_engine engine);

  // Releases an engine.
  static void release_engine(cl_engine engine);

  // Releases a program.
  static void release_program(cl_world world);

  // Transfers data from a previously allocated buffer.
  static void memcpy_from(cl_world world, void *src_ptr, size_t offset,
                          void *dst_ptr, size_t size);

  // Frees a buffer.
  static void free(cl_world world, void *ptr);

  // Releases the world.
  static void release_world(cl_world world);
};

#endif
