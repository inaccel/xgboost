#ifndef ALLOCATOR_HPP_
#define ALLOCATOR_HPP_

#include <cerrno>
#include <cstring>
#include <vector>
#include <stdexcept>

#include "cube.h"

namespace inaccel {

/** @class allocator
 *  @brief Custom allocator intended for std::vector
 *  @details This class should by no means be considered as a standalone class
 *  since it is implemented to support the inaccel::vector data structure.
 */
template<typename T>
class allocator: public std::allocator<T> {
	public:
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef const T* const_pointer;
		typedef const T& const_reference;
		typedef T  value_type;
		typedef T* pointer;
		typedef T& reference;
		using propagate_on_container_swap = std::true_type;

		allocator() throw() { }
		~allocator() throw() { }

		template <typename U>
		struct rebind {
			typedef allocator<U> other;
		};

		inline pointer address(reference value) const {
			return &value;
		}

		inline const_pointer address(const_reference value) const {
			return &value;
		}

		inline void construct(pointer p, reference value) {
			new ((void*) p) T(value);
		}

		pointer allocate(size_type n, const void * hint = 0) {
			if(!n) return nullptr;
			size_t alloc_size = n * sizeof(T);

			pointer p = reinterpret_cast<pointer>(cube_alloc(alloc_size));
			if(p == nullptr) {
				std::string msg = std::string("inaccel::allocate failed: ") + strerror(errno);
				throw std::runtime_error(msg);
			}

			return p;
		}

		void deallocate(pointer p, size_type n) {
			if (cube_free(p)){
				std::string msg = std::string("inaccel::deallocate failed: ") + strerror(errno);
				throw std::runtime_error(msg);
			}
		}
};

} // namespace inaccel

#endif // ALLOCATOR_HPP_
