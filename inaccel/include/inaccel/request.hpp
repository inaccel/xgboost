#ifndef REQUEST_HPP_
#define REQUEST_HPP_

#include <string>
#include <vector>
#include <type_traits>

#include "vector.hpp"
#include "environment.h"

namespace inaccel {

template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref>: std::true_type {};

static const int used_bits = 1;

static const std::string enum_table[32] = { "dirty", "intermediate", /*unused*/
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "" };

//dirty is a hidden flag
enum flag {
	NONE					= 0,
	INTERMEDIATE	= 1<<0
};

inline flag operator|(flag a, flag b)
{
    return static_cast<flag>(static_cast<int>(a) | static_cast<int>(b));
}

class wire;
class packet;

/** @struct argument_api
 *  @brief A virtual struct to match all types of arguments.
 *
 *  @details This struct is an api that all argument types
 *  should match.
 */
struct argument_api {
	// virtual destructor
	virtual ~argument_api() {}

	// true if the content is string
	// false if the content is bytes
	// used only by packet_view and string_view
	virtual bool content_is_printable() const  = 0;

	// returns the header for this argument as a string
	virtual std::string get_header() const = 0;

	// returns the content for this argument as a byte array
	virtual std::string get_content() const = 0;

	// used only by inaccel vectors
	virtual void unset_dirty() const = 0;

	// returns a copy of the argument
	virtual argument_api* clone() const = 0;
};

/** @class request
 *  @brief A high-level class to represent accelerated functions.
 *
 *  @details This class provides an intermediate representation of an
 *  accelerated function advertised to the %Coral FPGA resource manager. You
 *  should use this class to form an acceleration request which you will
 *  later submit to Coral FPGA resource manager. Your interaction with this
 *  class should ideally be limited to declaring the function you wish to execute
 *  and populating this function with arguments.
 */
class request {
	public:
		//TODO(elias): FIX THE LINK BELOW

		/** @brief Constructs a request for the %Coral FPGA resource manager.
		 *
 	 	*  @details Basically the available request types comprise all the
		 *  accelerated functions advertised to %Coral FPGA resource manager
		 *  (Check out <a href=" ">here</a> how to advertise your functions to
		 *  %Coral). The request is tightly coupled with the accelerated function
		 *  it will serve, and you should ensure that you provide a valid name.
		 *  Be aware that no validation is check is performed on whether a valid
		 *  function name is supplied. Therefore, check out
		 *  <a href=" ">examples</a> for proper usage.
		 *
		 */
		request();

		/** @brief Constructs a request for the %Coral FPGA resource manager.
		 *
 	 	*  @details Basically the available request types comprise all the
		 *  accelerated functions advertised to %Coral FPGA resource manager
		 *  (Check out <a href=" ">here</a> how to advertise your functions to
		 *  %Coral). The request is tightly coupled with the accelerated function
		 *  it will serve, and you should ensure that you provide a valid name.
		 *  Be aware that no validation is check is performed on whether a valid
		 *  function name is supplied. Therefore, check out
		 *  <a href=" ">examples</a> for proper usage.
		 *
		 */
		request(const request& req);

		/** @brief Constructs a request for the %Coral FPGA resource manager.
		 *
 	 	*  @details Basically the available request types comprise all the
		 *  accelerated functions advertised to %Coral FPGA resource manager
		 *  (Check out <a href=" ">here</a> how to advertise your functions to
		 *  %Coral). The request is tightly coupled with the accelerated function
		 *  it will serve, and you should ensure that you provide a valid name.
		 *  Be aware that no validation is check is performed on whether a valid
		 *  function name is supplied. Therefore, check out
		 *  <a href=" ">examples</a> for proper usage.
		 *
		 *  @param type The alias name of the accelerated function.
		 */
		request(const std::string type);

 		~request();

		/** @brief Adds a(n) (non vector) argument to the request.
		 *
		 *  @details You should use this method to provide scalars or structs as
		 *  arguments.
		 *
		 *  @param value The argument value to be added to the request
		 *  @returns The current %request object
		 */
		template <typename T>
		request& arg(const T& value, flag flags = NONE);

		/** @brief Adds a(n) (non vector) argument the request at the specified
		 *  index.
		 *
		 *  @details You should use this method to provide scalars or structs as
		 *  arguments.
		 *
		 *  @param index The index of the argument to be added
		 *  @param value The argument value to be added to the request
		 *  @returns The current %request object
		 */
		template <typename T>
		request& arg(unsigned index, const T& value, flag flags = NONE);

		/** @brief Provides a string representation of the request
		 *  @details You should not directly invoke this method since Coral will
		 *  automatically invoke it in case of a request submission. It should be
		 *  used only for debug purposes.
		 *  @returns A string representation of the request intended for the
		 *  FPGA resource manager
		 */
		std::string string_view() const;

		/** @brief Modifies the request type.
		 *
		 *  @details You should use this method to choose one from the available
		 *  accelerated functions advertised to coral. This method can also be
		 *  used to modify the type of a request in case you want to invoke
		 *  another accelerator with the same input arguments
		 *
		 *  @param new_type The new alias name of the accelerated function
		 */
		inline void type(std::string new_type) { _type = new_type; }

		/** @brief Provides a string representation of the request to be
		 *  submitted
		 *
		 *  @details This method is intended only for debug purposes
		 *  @returns A string representation of the request to be submitted
		 */
		std::string packet_view() const;

		/** @brief For all arguments that are inaccel vectors, unsets the
		 *  dirty bit.
		 *
		 *  @details This method is automatically called by wire::finalize()
		 */
		void cleanup() const;

	private:
		/* internal class to keep each argument*/

		/* name of the kernel (accelerator) associated with this request */
		std::string _type;

		/* list containing pairs of (argument-type, argument-value) - both as string */
		std::vector<argument_api*> arguments;

		/* @brief Packs the request arguments into a packet */
		packet pack() const;

		/* only friend methods below can access request private members/methods */
		friend wire *submit(request& req);
		friend void wait(wire *session);
};

/** @struct argument<generic>
 *  @brief A struct to match all arguments that are not std vectors,
 *  std strings or inaccel vectors.
 */
template<class _Tp, class Enable = void>
struct argument : argument_api {
	// reference to the actual argument
	const _Tp& value;

	// constructor
	argument(const _Tp& in, unsigned flags_in): value(in) {
		if(std::is_pointer<_Tp>::value)
			throw std::runtime_error("Invalid argument, pointers not supported, use references or vectors");
		if(flags_in)
			throw std::runtime_error("Invalid argument, flags supported only for inaccel vectors");
	}

	// copy constructor
	argument(const argument& arg): value(arg.value) { }

	// for the generic type, the content is not printable as is
	inline bool content_is_printable() const { return false; }

	// for the generic type, the header is the size of the content in Bytes
	inline std::string get_header() const {
		return std::to_string(sizeof(value));
	}

	// for the generic type, the content is a Byte array
	// (using the std string container)
	inline std::string get_content() const {
		return std::string(reinterpret_cast<const char*>(&value), sizeof(value));
	}

	// for the generic type this function is unused
	inline void unset_dirty() const {}

	inline argument* clone() const {
		return new argument(*this);
	}
};

/** @struct argument<std::vector>
 *  @brief A struct to match all std vectors.
 */
template<class _Tp>
struct argument<_Tp,
	typename std::enable_if<is_specialization<_Tp, std::vector>::value>::type>
: argument_api {
	// reference to the actual argument
	const _Tp& value;

	// constructor
	argument(const _Tp& in, unsigned flags_in): value(in) {
		if(std::is_pointer<typename _Tp::value_type>::value)
			throw std::runtime_error("Invalid argument, vectors of pointers not supported");
		if(flags_in)
			throw std::runtime_error("Invalid argument, flags supported only for inaccel vectors");
	}

	// copy constructor
	argument(const argument& arg): value(arg.value) { }

	// for std vectors, the content is not printable as is
	// instead of std::vector<char> please use std::string for printable content
	inline bool content_is_printable() const { return false; }

	// for std vectors, the header is the size of the underlying memory in Bytes
	inline std::string get_header() const {
		return std::to_string(value.size() * sizeof(typename _Tp::value_type));
	}

	// for std vectors, the content is a Byte array
	// (using the std string container)
	inline std::string get_content() const {
		return std::string(reinterpret_cast<const char*>(value.data()),
												value.size() * sizeof(typename _Tp::value_type));
	}

	// for std vectors this function is unused
	inline void unset_dirty() const {}

	inline argument* clone() const {
		return new argument(*this);
	}
};

/** @struct argument<std::string>
 *  @brief A struct to match all std strings.
 */
template<class _Tp>
struct argument<_Tp,
	typename std::enable_if<is_specialization<_Tp, std::basic_string>::value>::type>
: argument_api {
	// reference to the actual argument
	const _Tp& value;

	// constructor
	argument(const _Tp& in, unsigned flags_in): argument_api(), value(in) {
		if(flags_in)
			throw std::runtime_error("Invalid argument, flags supported only for inaccel vectors");
	}

	// copy constructor
	argument(const argument& arg): value(arg.value) { }

	// for std strings, the content is printable
	inline bool content_is_printable() const { return true; }

	// for std strings, the header is the size of the underlying memory in Bytes
	inline std::string get_header() const {
		return std::to_string(value.size() * sizeof(typename _Tp::value_type));
	}

	// for std strings, the content is the string
	inline std::string get_content() const {
		return value;
	}

	// for std strings this function is unused
	inline void unset_dirty() const {}

	inline argument* clone() const {
		return new argument(*this);
	}
};

/** @struct argument<inaccel::vector>
 *  @brief A struct to match all inaccel vectors.
 */
template<class _Tp>
struct argument<_Tp,
	typename std::enable_if<is_specialization<_Tp, inaccel::vector>::value>::type>
: argument_api {
	// reference to the actual argument
	const _Tp& value;
	// the flags for this argument
	unsigned flags;

	// constructor
	argument(const _Tp& in, unsigned flags_in)
	: value(in), flags(flags_in) {
		if(std::is_pointer<typename _Tp::value_type>::value)
			throw std::runtime_error("Invalid argument, vectors of pointers not supported");
	}

	// copy constructor
	argument(const argument& arg): value(arg.value), flags(arg.flags) { }

	// for inaccel vectors, the content is printable
	inline bool content_is_printable() const { return true; }

	// for inaccel vectors, the header is the keyword 'Cube'
	// and if one or more flags are set, a comma separated list of the
	// flag keywords as defined in enum_table, wrapped in <>
	inline std::string get_header() const {
		std::string parsed_flags = "Cube";
		// concatenate dirty status with flags
		unsigned long valid_bits_mask = (1 << used_bits) -1;
		unsigned long tmp_flags = ((valid_bits_mask & flags)<<1) | value.is_dirty();
		if(tmp_flags) {
			parsed_flags.append("<");
			for (int i = 0; i < (used_bits + 1); i++) {
				// create current flag mask
				unsigned curr_bit_mask = 1<<i;
				// check current flag
				if(tmp_flags & curr_bit_mask) {
					parsed_flags.append(inaccel::enum_table[i]);
					// clear the flag from the tmp flags
					tmp_flags &= (~curr_bit_mask);
					// if no other flags are left break
					if(!tmp_flags) break;
					else parsed_flags.append(",");
				}
				if(!tmp_flags) break;
			}
			parsed_flags.append(">");
		}
		return parsed_flags;
	}

	// for inaccel vectors, the content is the cube id as returned by the c api
	inline std::string get_content() const {
		return std::string(cube_getid(value.cbegin()._M_p));
	}

	// for inaccel vectors set the dirty bit to false
	inline void unset_dirty() const { value.unset_dirty(); }

	inline argument* clone() const {
		return new argument(*this);
	}
};

template <class T>
request& request::arg(const T& value, flag flags) {
	arguments.push_back(new argument<T>(value, flags));
	return *this;
}

template <class T>
request& request::arg(unsigned index, const T& value, flag flags) {
	if(index >= arguments.size()) {
		size_t i = arguments.size();
		arguments.resize(index+1);
		for(; i < arguments.size(); i++)
			arguments[i] = nullptr;
	}
	arguments[index] = new argument<T>(value, flags);
	return *this;
}

} // namespace inaccel

#endif  // REQUEST_HPP_
