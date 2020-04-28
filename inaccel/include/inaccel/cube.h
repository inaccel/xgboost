#ifndef CUBE_H_
#define CUBE_H_

#define META_BYTES 4096

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @file cube.h
 *  @brief Custom free function
 *
 *  @details This function is used to free memory allocated with cube_alloc.
 *
 *  @returns On success 0 is returned. If ptr is NULL no operation is performed
 *  and zero is returned. On failure -1 is returned and errno is set
 *  appropriately.
 */
int cube_free(void *ptr);

/**
 *  @brief Custom function for getting the dirty byte of a memory mapped file
 *
 *  @details Given a pointer allocated with cube_alloc, this function
 *  returns a pointer to the dirty byte.
 *
 *  @returns On success, the pointer to the dirty byte is returned. On
 *  failure, NULL pointer is returned
 */
char *cube_getdirty(const void *ptr);

/**
 *  @brief Custom function for getting the id of a memory mapped file
 *
 *  @details Given a pointer allocated with cube_alloc, this function
 *  returns a pointer to the file id string. Internally, the string is created
 *  using strdup function so the user has to explicitly free the returned
 *  pointer.

 *  @returns On success, the pointer to the file string id is returned. On
 *  failure, NULL pointer is returned
 */
char *cube_getid(const void *ptr);

/**
 *  @brief Custom malloc function
 *
 *  @details This function allocates shared memory using mmapped files. It is
 *  intended for use with InAccel Coral framework. Any requests sent to InAccel
 *  Coral must have its pointers allocated with cube_alloc
 *
 *  @returns On success the newly allocated pointer is returned, otherwise NULL
 *  is returned and errno is set appropriately. NULL may also be returned by a
 *  successful call to cube_alloc() with a size of zero
 */
void *cube_alloc(unsigned long size);

/**
 *  @brief Custom realloc function
 *
 *  @details This function is used to reallocate memory allocated with cube_alloc.
 *  If a NULL pointer and a size greater than zero are given, the function allocates new memory.
 *  If a non-NULL pointer and a size of zero are given, the function deallocates the memory
 *  pointed to by the input pointer.
 *
 *  @returns On success the reallocated pointer is returned, otherwise NULL
 *  is returned and errno is set appropriately. NULL may also be returned by a
 *  successful call to cube_realloc() with a size of zero.
 */
void *cube_realloc(void *ptr, unsigned long new_size);

/**
 *  @brief rename function
 *
 *  @details This function is used to rename the underlying cube pointed to by ptr.
 *  The pointer should always be valid and allocated with cube_alloc.
 *
 *  @returns On success 0 is returned. On failure -1 is returned and errno is set
 *  appropriately.
 */
int cube_rename( void *ptr);

#ifdef __cplusplus
}
#endif

#endif	// CUBE_H_
