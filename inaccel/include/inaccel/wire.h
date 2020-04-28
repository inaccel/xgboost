#ifndef WIRE_H_
#define WIRE_H_

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wire *session;
typedef struct _packet *packet;

/** @brief Creates a wire connected to the given port and ip.
 *
 *  @param ip The ip to connect to.
 *  @param port The port to use.
 *  @returns A wire.
 */
session wire_open(const uint32_t ip, int port);

/** @brief Disconnects and deletes a connected wire.
 *
 *  @param wire The wire to disconnect.
 *  @returns 0 on success, -1 on failure.
 */
int wire_close(session wire);

/** @brief  Writes a packet to the output stream.
 *
 *  @param pckt The packet to be written.
 *  @returns The number of written bytes to stream.
 */
int wire_write(session wire, packet pckt);
/** @brief Reads a string from the wire.
 *
 *  @param str The wire to read from.
 *  @returns A pointer to the string read. NULL in case of error.
 */
char *wire_read_UTF(session wire);

/** @brief  Writes a string to the wire.
 *
 *  @param str The wire to write to.
 *  @param str The string to be written.
 *  @returns The length of written string to stream. -1 in case of error.
 */
int wire_write_UTF(session wire, const char *str);

/** @brief Reads a byte array from the the wire.
 *
 *  @param str The wire to read from.
 *  @param len The length of byte array to be read.
 *  @returns A pointer to the string read. NULL in case of error.
 */
char *wire_read_bytes(session wire, size_t len);

/** @brief  Writes a byte array to the the wire.
 *
 *  @param str The wire to write to.
 *  @param str The byte array to be written.
 *  @param len The length of the byte array to be written.
 *  @returns The length of written string to stream. -1 in case of error.
 */
int wire_write_bytes(session wire, const char *str, size_t len);

/** @brief Client synchronization point.
 *
 *  @param str The wire to synchronize.
 *  @returns 0 on success, -1 on failure.
 */
int wire_sync(session wire);

/** @brief Server synchronization point.
 *
 *  @param str The wire to send synchronization point.
 *  @returns 0 on success, -1 on failure.
 */
int wire_cnys(session wire);

#ifdef __cplusplus
}
#endif

#endif // WIRE_H_
