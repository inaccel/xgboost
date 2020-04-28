#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifndef WIRE_HPP_
#define WIRE_HPP_

#include <cstring>
#include <memory>

#include "packet.hpp"

namespace inaccel {

class request;

/** @class wire
 *  @brief A class implementing client wires.
 *
 *  @details A wire is an endpoint for communication between two processes.
 *  The two processes can exchange primitive Java data-type packets using the
 *  underlying input/output streams.
 */
class wire{
private:
		/** @brief Creates a wire given an ip and a port.
		 *
		 *  @details You should use this method to create a wire using an
		 *  ip and a port to connect to. Internally, to resolve a hostname to an
		 *  ip, you should use ip::getbyname method to return the ip in uint32_t
		 *  format to feed the sockaddr_in struct
		 *
		 *  @param ip The ip in byte order format.
		 *  @param port The port number
		 *  @returns A %wire object
		 */
		wire(const uint32_t ip, int port, const request& req);

		~wire();

			/** @brief Performs cleanup action before closing the wire.
			 *
			 *  @details  Used to unset the dirty bit in the arguments of
			 *  the underlying request that are inaccel vectors.
			 */
			void finalize();

		/** @brief Closes the wire.
		 *
		 *  @details  Once a wire has been closed, it is not available for
		 *  further networking use (i.e. can't be reconnected or rebound). A new
		 *  wire needs to be created. Closing this wire will also close the
		 *  wire's underlying streams.
		 */
		void close();

		/** @brief Reads a string from the input stream.
		 *
		 *  @returns A pointer to the string read.
		 */
		std::unique_ptr<char []> read_UTF();

		/** @brief  Writes a packet to the output stream.
		 *
		 *  @param pckt The packet to be written.
		 *  @returns The number of written bytes to stream.
		 */
		int write(packet pckt);

		/** @brief  Writes bytes to the output stream.
		 *
		 *  @param str The string which holds the bytes to be written.
		 *  @returns The number of written bytes to stream.
		 */
		int write_bytes(const std::string &str);

		/** @brief  Writes a string to the output stream.
		 *
		 *  @param str The string to be written.
		 *  @returns The length of written string to stream.
		 */
		int write_UTF(const std::string &str);

		/** @brief Client synchronization point.
		 *
		 * @return This wire.
		 */
		wire *sync();

		/** @brief Server synchronization point.
		 *
		 * @return This wire.
		 */
		wire *cnys();

		/** @brief The underlying socket file descriptor
		 */
		int socket_fd;

		/** @brief The request that created this wire
		 */
		const request &req_ref;

		/** Only the functions below have access to the wire private methods/
		 *  members.
		*/
		friend wire *submit(request& req);
		friend void wait(wire *session);
};

typedef wire *session;

} // namespace inaccel

#endif // WIRE_HPP_

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
