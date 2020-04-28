#ifndef REQUEST_H
#define REQUEST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _request *request;
typedef struct _packet *packet;

/** @brief Create a request for a kernel type.
 *
 *  @param r_type The request type.
 *  @returns A request.
 */
request request_create(const char *r_type);

/** @brief Deallocate a request.
 *
 *  @param req The request.
 */
void request_free(request req);

/** @brief Change or set the request type.
 *
 *  @param req The request.
 *  @param r_type The request type.
 */
void request_type(request req, const char *r_type);

/** @brief Add an argument at index.
 *
 *  @param req The request.
 *  @param index The index at which to add the argument. If the argument at
 *  that index does not exist, it is created. If the argument at that index
 *  exists, it is replaced with the new argument
 *  @param size The argument's size in Bytes.
 *  @param value A pointer that points to the argument.
 */
void request_arg(request req, unsigned long index, unsigned long size, void *value);

/** @brief Convert a request to string format for debug.
 *
 *  @param request The request.
 *  @returns A pointer to a string. NULL in case of error.
 */
char *request_packet_view(request req);

/** @brief Convert a request to string format for debug.
 *
 *  @param request The request.
 *  @returns A pointer to a string. NULL in case of error.
 */
packet request_pack(request req);

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_H
