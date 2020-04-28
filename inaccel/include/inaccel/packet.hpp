#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifndef PACKET_HPP_
#define PACKET_HPP_

#include <string>

namespace inaccel {

class request;
class wire;

/** @class packet
 *  @brief A class implementing a packet.
 *
 *  @details A packet contains a header and some contents.
 */
class packet{
	private:
		/** @brief Creates a packet to be sent through a wire
		 *
		 *  @details This constructor constructs a packet given a string
		 *  representation of a header and some contents
		 *
		 *  @param header The packet header
		 *  @param content The packet contents
		 */
		packet(std::string header, std::string content);

		~packet() { }

		/** @brief The underlying packet header
		*/
		std::string _header;

		/** @brief The underlying packet contents
		*/
		std::string _content;

		/** @brief Return the header of the packet
		 *
		 *  @details  The header is returned using its string representation
		 */
		std::string header();

		/** @brief Return the contents of the packet
		 *
		 *  @details  The contents are returned in a const char * buffer
		 */
		std::string content();

		/** Only the functions and classes below have access to the wire private
		 *  methods/members.
		*/
		friend class request;

		friend wire *submit(request& req);

		friend class wire;
};

} // namespace inaccel

#endif // PACKET_HPP_

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
