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

#include <malloc.h>
#include <string.h>

#include "INcl.h"

// Builds a program executable from the program binary.
void INclBuildProgram(cl_program program) {
  cl_int errcode_ret = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clBuildProgram %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Creates a buffer object.
cl_mem INclCreateBuffer(cl_context context, cl_mem_flags flags, size_t size,
                        void *host_ptr) {
  cl_int errcode_ret;
  cl_mem mem = clCreateBuffer(context, flags, size, host_ptr, &errcode_ret);
  if (errcode_ret != CL_SUCCESS || !mem) {
    fprintf(stderr, "Error: clCreateBuffer %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }

  return mem;
}

// Create a command-queue on a specific device.
cl_command_queue INclCreateCommandQueue(cl_context context,
                                        cl_device_id device) {
  cl_int errcode_ret;
  cl_command_queue command_queue =
      clCreateCommandQueue(context, device, 0, &errcode_ret);
  if (errcode_ret != CL_SUCCESS || !command_queue) {
    fprintf(stderr, "Error: clCreateCommandQueue %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }

  return command_queue;
}

// Creates an OpenCL context.
cl_context INclCreateContext(cl_device_id device) {
  cl_int errcode_ret;
  cl_context context = clCreateContext(0, 1, &device, NULL, NULL, &errcode_ret);
  if (errcode_ret != CL_SUCCESS || !context) {
    fprintf(stderr, "Error: clCreateContext %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }

  return context;
}

// Creates a kernel object.
cl_kernel INclCreateKernel(cl_program program, const char *kernel_name) {
  cl_int errcode_ret;
  cl_kernel kernel = clCreateKernel(program, kernel_name, &errcode_ret);
  if (errcode_ret != CL_SUCCESS || !kernel) {
    fprintf(stderr, "Error: clCreateKernel %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }

  return kernel;
}

// Creates a program object for a context, and loads specified binary data into
// the program object.
cl_program INclCreateProgramWithBinary(cl_context context, cl_uint num_devices,
                                       const cl_device_id *device_list,
                                       const char *binary_name) {
  FILE *file = fopen(binary_name, "rb");
  if (!file) {
    fprintf(stderr, "Error: fopen\n");
    throw EXIT_FAILURE;
  }

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *temp = (char *)malloc((size + 1) * sizeof(char));
  if (!temp) {
    fprintf(stderr, "Error: malloc\n");
    throw EXIT_FAILURE;
  }

  if (size != fread(temp, sizeof(char), size, file)) {
    free(temp);

    fprintf(stderr, "Error: fread\n");
    throw EXIT_FAILURE;
  }

  fclose(file);
  temp[size] = 0;

  char *binary = temp;

  cl_int errcode_ret;
  cl_program program = clCreateProgramWithBinary(
      context, num_devices, device_list, &size, (const unsigned char **)&binary,
      NULL, &errcode_ret);
  if (errcode_ret != CL_SUCCESS || !program) {
    fprintf(stderr, "Error: clCreateProgramWithBinary %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }

  free(temp);

  return program;
}

// Enqueues a command to map a region of the buffer object given by buffer into
// the host address space and returns a pointer to this mapped region.
void *INclEnqueueMapBuffer(cl_command_queue command_queue, cl_mem buffer,
                           cl_map_flags map_flags, size_t cb,
                           cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list, cl_event *event) {
  cl_int errcode_ret;
  void *ptr = clEnqueueMapBuffer(command_queue, buffer, CL_FALSE, map_flags, 0,
                                 cb, num_events_in_wait_list, event_wait_list,
                                 event, &errcode_ret);
  if (errcode_ret != CL_SUCCESS || !ptr) {
    fprintf(stderr, "Error: clEnqueueMapBuffer %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }

  return ptr;
}

// Enqueues a command to indicate which device a set of memory objects should be
// associated with.
void INclEnqueueMigrateMemObjects(cl_command_queue command_queue,
                                  cl_uint num_mem_objects,
                                  const cl_mem *mem_objects,
                                  cl_mem_migration_flags flags,
                                  cl_uint num_events_in_wait_list,
                                  const cl_event *event_wait_list,
                                  cl_event *event) {
  cl_int errcode_ret = clEnqueueMigrateMemObjects(
      command_queue, num_mem_objects, mem_objects, flags,
      num_events_in_wait_list, event_wait_list, event);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clEnqueueMigrateMemObjects %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Enqueues a command to execute a kernel on a device.
void INclEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel,
                              cl_uint work_dim, const size_t *global_work_size,
                              const size_t *local_work_size,
                              cl_uint num_events_in_wait_list,
                              const cl_event *event_wait_list,
                              cl_event *event) {
  cl_int errcode_ret = clEnqueueNDRangeKernel(
      command_queue, kernel, work_dim, NULL, global_work_size, local_work_size,
      num_events_in_wait_list, event_wait_list, event);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clEnqueueNDRangeKernel %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Enqueue commands to read from a buffer object to host memory.
void INclEnqueueReadBuffer(cl_command_queue command_queue, cl_mem buffer,
                           size_t offset, size_t cb, void *ptr,
                           cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list, cl_event *event) {
  cl_int errcode_ret =
      clEnqueueReadBuffer(command_queue, buffer, CL_FALSE, offset, cb, ptr,
                          num_events_in_wait_list, event_wait_list, event);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clEnqueueReadBuffer %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Enqueues a command to execute a kernel on a device.
void INclEnqueueTask(cl_command_queue command_queue, cl_kernel kernel,
                     cl_uint num_events_in_wait_list,
                     const cl_event *event_wait_list, cl_event *event) {
  cl_int errcode_ret = clEnqueueTask(
      command_queue, kernel, num_events_in_wait_list, event_wait_list, event);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clEnqueueTask %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Enqueue commands to write to a buffer object from host memory.
void INclEnqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer,
                            size_t offset, size_t cb, const void *ptr,
                            cl_uint num_events_in_wait_list,
                            const cl_event *event_wait_list, cl_event *event) {
  cl_int errcode_ret =
      clEnqueueWriteBuffer(command_queue, buffer, CL_FALSE, offset, cb, ptr,
                           num_events_in_wait_list, event_wait_list, event);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clEnqueueWriteBuffer %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Blocks until all previously queued OpenCL commands in a command-queue are
// issued to the associated device and have completed.
void INclFinish(cl_command_queue command_queue) {
  cl_int errcode_ret = clFinish(command_queue);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clFinish %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Issues all previously queued OpenCL commands in a command-queue to the device
// associated with the command-queue.
void INclFlush(cl_command_queue command_queue) {
  cl_int errcode_ret = clFlush(command_queue);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clFlush %s (%d)\n", INclCheckErrorCode(errcode_ret),
            errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Obtain specified device, if available.
cl_device_id INclGetDeviceID(cl_platform_id platform, cl_uint id) {
  cl_device_id device_id = (cl_device_id)malloc(sizeof(cl_device_id));
  if (!device_id) {
    fprintf(stderr, "Error: malloc\n");
    throw EXIT_FAILURE;
  }

  cl_uint num_devices;
  INclGetDeviceIDs(platform, 0, NULL, &num_devices);

  cl_device_id *devices =
      (cl_device_id *)malloc(num_devices * sizeof(cl_device_id));
  if (!devices) {
    fprintf(stderr, "Error: malloc\n");
    throw EXIT_FAILURE;
  }

  INclGetDeviceIDs(platform, num_devices, devices, NULL);

  cl_uint i;
  for (i = 0; i < num_devices; i++) {
    if (i == id) {
      device_id = devices[i];
      break;
    }
  }

  free(devices);

  if (i == num_devices) {
    fprintf(stderr, "Error: clGetDeviceID\n");
    throw EXIT_FAILURE;
  }

  return device_id;
}

// Obtain the list of devices available on a platform.
void INclGetDeviceIDs(cl_platform_id platform, cl_uint num_entries,
                      cl_device_id *devices, cl_uint *num_devices) {
  cl_int errcode_ret = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_entries,
                                      devices, num_devices);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clGetDeviceIDs %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Get specific information about the OpenCL device.
void INclGetDeviceInfo(cl_device_id device, cl_device_info param_name,
                       size_t param_value_size, void *param_value,
                       size_t *param_value_size_ret) {
  cl_int errcode_ret = clGetDeviceInfo(device, param_name, param_value_size,
                                       param_value, param_value_size_ret);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clGetDeviceInfo %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Obtain platform, if available.
cl_platform_id INclGetPlatformID() {
  cl_platform_id platform_id = (cl_platform_id)malloc(sizeof(cl_platform_id));
  if (!platform_id) {
    fprintf(stderr, "Error: malloc\n");
    throw EXIT_FAILURE;
  }

  cl_uint num_platforms;
  INclGetPlatformIDs(0, NULL, &num_platforms);

  cl_platform_id *platforms =
      (cl_platform_id *)malloc(num_platforms * sizeof(cl_platform_id));
  if (!platforms) {
    fprintf(stderr, "Error: malloc\n");
    throw EXIT_FAILURE;
  }

  INclGetPlatformIDs(num_platforms, platforms, NULL);

  cl_uint i;
  for (i = 0; i < num_platforms; i++) {
    size_t platform_name_size;
    INclGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, NULL,
                        &platform_name_size);

    char *platform_name = (char *)malloc(platform_name_size * sizeof(char));
    if (!platform_name) {
      fprintf(stderr, "Error: malloc\n");
      throw EXIT_FAILURE;
    }

    INclGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, platform_name_size,
                        platform_name, NULL);

    if (strstr(platform_name, "Xilinx")) {
      free(platform_name);

      platform_id = platforms[i];
      break;
    }

    free(platform_name);
  }

  free(platforms);

  if (i == num_platforms) {
    fprintf(stderr, "Error: clGetPlatformID\n");
    throw EXIT_FAILURE;
  }

  return platform_id;
}

// Obtain the list of platforms available.
void INclGetPlatformIDs(cl_uint num_entries, cl_platform_id *platforms,
                        cl_uint *num_platforms) {
  cl_int errcode_ret = clGetPlatformIDs(num_entries, platforms, num_platforms);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clGetPlatformIDs %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Get specific information about the OpenCL platform.
void INclGetPlatformInfo(cl_platform_id platform, cl_platform_info param_name,
                         size_t param_value_size, void *param_value,
                         size_t *param_value_size_ret) {
  cl_int errcode_ret = clGetPlatformInfo(platform, param_name, param_value_size,
                                         param_value, param_value_size_ret);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clGetPlatformInfo %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Decrements the command_queue reference count.
void INclReleaseCommandQueue(cl_command_queue command_queue) {
  cl_int errcode_ret = clReleaseCommandQueue(command_queue);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clReleaseCommandQueue %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Decrement the context reference count.
void INclReleaseContext(cl_context context) {
  cl_int errcode_ret = clReleaseContext(context);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clReleaseContext %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Decrements the event reference count.
void INclReleaseEvent(cl_event event) {
  cl_int errcode_ret = clReleaseEvent(event);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clReleaseEvent %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Decrements the kernel reference count.
void INclReleaseKernel(cl_kernel kernel) {
  cl_int errcode_ret = clReleaseKernel(kernel);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clReleaseKernel %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Decrements the memory object reference count.
void INclReleaseMemObject(cl_mem memobj) {
  cl_int errcode_ret = clReleaseMemObject(memobj);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clReleaseMemObject %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Decrements the program reference count.
void INclReleaseProgram(cl_program program) {
  cl_int errcode_ret = clReleaseProgram(program);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clReleaseProgram %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Used to set the argument value for a specific argument of a kernel.
void INclSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size,
                      const void *arg_value) {
  cl_int errcode_ret = clSetKernelArg(kernel, arg_index, arg_size, arg_value);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clSetKernelArg %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Waits on the host thread for commands identified by event objects to
// complete.
void INclWaitForEvents(cl_uint num_events, const cl_event *event_list) {
  cl_int errcode_ret = clWaitForEvents(num_events, event_list);
  if (errcode_ret != CL_SUCCESS) {
    fprintf(stderr, "Error: clWaitForEvents %s (%d)\n",
            INclCheckErrorCode(errcode_ret), errcode_ret);
    throw EXIT_FAILURE;
  }
}

// Returns a message related to the error code.
const char *INclCheckErrorCode(cl_int errcode) {
  switch (errcode) {
  case -1:
    return "CL_DEVICE_NOT_FOUND";
  case -2:
    return "CL_DEVICE_NOT_AVAILABLE";
  case -3:
    return "CL_COMPILER_NOT_AVAILABLE";
  case -4:
    return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
  case -5:
    return "CL_OUT_OF_RESOURCES";
  case -6:
    return "CL_OUT_OF_HOST_MEMORY";
  case -7:
    return "CL_PROFILING_INFO_NOT_AVAILABLE";
  case -8:
    return "CL_MEM_COPY_OVERLAP";
  case -9:
    return "CL_IMAGE_FORMAT_MISMATCH";
  case -10:
    return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
  case -11:
    return "CL_BUILD_PROGRAM_FAILURE";
  case -12:
    return "CL_MAP_FAILURE";
  case -13:
    return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
  case -14:
    return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
  case -15:
    return "CL_COMPILE_PROGRAM_FAILURE";
  case -16:
    return "CL_LINKER_NOT_AVAILABLE";
  case -17:
    return "CL_LINK_PROGRAM_FAILURE";
  case -18:
    return "CL_DEVICE_PARTITION_FAILED";
  case -19:
    return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
  case -30:
    return "CL_INVALID_VALUE";
  case -31:
    return "CL_INVALID_DEVICE_TYPE";
  case -32:
    return "CL_INVALID_PLATFORM";
  case -33:
    return "CL_INVALID_DEVICE";
  case -34:
    return "CL_INVALID_CONTEXT";
  case -35:
    return "CL_INVALID_QUEUE_PROPERTIES";
  case -36:
    return "CL_INVALID_COMMAND_QUEUE";
  case -37:
    return "CL_INVALID_HOST_PTR";
  case -38:
    return "CL_INVALID_MEM_OBJECT";
  case -39:
    return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
  case -40:
    return "CL_INVALID_IMAGE_SIZE";
  case -41:
    return "CL_INVALID_SAMPLER";
  case -42:
    return "CL_INVALID_BINARY";
  case -43:
    return "CL_INVALID_BUILD_OPTIONS";
  case -44:
    return "CL_INVALID_PROGRAM";
  case -45:
    return "CL_INVALID_PROGRAM_EXECUTABLE";
  case -46:
    return "CL_INVALID_KERNEL_NAME";
  case -47:
    return "CL_INVALID_KERNEL_DEFINITION";
  case -48:
    return "CL_INVALID_KERNEL";
  case -49:
    return "CL_INVALID_ARG_INDEX";
  case -50:
    return "CL_INVALID_ARG_VALUE";
  case -51:
    return "CL_INVALID_ARG_SIZE";
  case -52:
    return "CL_INVALID_KERNEL_ARGS";
  case -53:
    return "CL_INVALID_WORK_DIMENSION";
  case -54:
    return "CL_INVALID_WORK_GROUP_SIZE";
  case -55:
    return "CL_INVALID_WORK_ITEM_SIZE";
  case -56:
    return "CL_INVALID_GLOBAL_OFFSET";
  case -57:
    return "CL_INVALID_EVENT_WAIT_LIST";
  case -58:
    return "CL_INVALID_EVENT";
  case -59:
    return "CL_INVALID_OPERATION";
  case -60:
    return "CL_INVALID_GL_OBJECT";
  case -61:
    return "CL_INVALID_BUFFER_SIZE";
  case -62:
    return "CL_INVALID_MIP_LEVEL";
  case -63:
    return "CL_INVALID_GLOBAL_WORK_SIZE";
  case -64:
    return "CL_INVALID_PROPERTY";
  case -65:
    return "CL_INVALID_IMAGE_DESCRIPTOR";
  case -66:
    return "CL_INVALID_COMPILER_OPTIONS";
  case -67:
    return "CL_INVALID_LINKER_OPTIONS";
  case -68:
    return "CL_INVALID_DEVICE_PARTITION_COUNT";
  case -69:
    return "CL_INVALID_PIPE_SIZE";
  case -70:
    return "CL_INVALID_DEVICE_QUEUE";
  default:
    return "CL_INVALID_ERROR_CODE";
  }
}
