#ifndef ITERATOR_HPP_
#define ITERATOR_HPP_

#include <cstddef>
#include "handler.hpp"

namespace inaccel {
	template<typename _Tp>
	struct iterator_base
	: public std::iterator<std::random_access_iterator_tag, _Tp> {
		_Tp * _M_p;

		iterator_base(_Tp * __x)
		: _M_p(__x) { }

		void _M_bump_up() {
			++_M_p;
		}

		void _M_bump_down() {
			--_M_p;
		}

		void _M_incr(ptrdiff_t __i) {
			_M_p += __i ;
		}

		bool operator==(const iterator_base& __i) const
		{ return _M_p == __i._M_p; }

		bool operator<(const iterator_base& __i) const {
			return _M_p < __i._M_p;
		}

		bool operator!=(const iterator_base& __i) const
		{ return !(*this == __i); }

		bool operator>(const iterator_base& __i) const
		{ return __i < *this; }

		bool operator<=(const iterator_base& __i) const
		{ return !(__i < *this); }

		bool operator>=(const iterator_base& __i) const
		{ return !(*this < __i); }

		_Tp* base() const
		{ return _M_p; }
	};

  template<>
	struct iterator_base<bool>
	: public std::iterator<std::random_access_iterator_tag, bool> {
		_Bit_type * _M_p;
		unsigned int _M_offset;

		iterator_base(_Bit_type * __x, unsigned int __y)
		: _M_p(__x), _M_offset(__y) { }

		void _M_bump_up() {
			if (_M_offset++ == int(_S_word_bit) - 1) {
				_M_offset = 0;
				++_M_p;
			}
		}

		void _M_bump_down() {
			if (_M_offset-- == 0)
			{
				_M_offset = int(_S_word_bit) - 1;
				--_M_p;
			}
		}

		void _M_incr(ptrdiff_t __i) {
			ptrdiff_t __n = __i + _M_offset;
			_M_p += __n / int(_S_word_bit);
			__n = __n % int(_S_word_bit);
			if (__n < 0) {
				__n += int(_S_word_bit);
				--_M_p;
			}
			_M_offset = static_cast<unsigned int>(__n);
		}

		bool operator==(const iterator_base& __i) const
		{ return _M_p == __i._M_p && _M_offset == __i._M_offset; }

		bool operator<(const iterator_base& __i) const {
			return _M_p < __i._M_p
			 || (_M_p == __i._M_p && _M_offset < __i._M_offset);
		}

		bool operator!=(const iterator_base& __i) const
		{ return !(*this == __i); }

		bool operator>(const iterator_base& __i) const
		{ return __i < *this; }

		bool operator<=(const iterator_base& __i) const
		{ return !(__i < *this); }

		bool operator>=(const iterator_base& __i) const
		{ return !(*this < __i); }
	};

	template<typename _Tp>
	inline ptrdiff_t operator-(const iterator_base<_Tp>& __x,
		const iterator_base<_Tp>& __y) {
		return (__x._M_p - __y._M_p);
	}

	inline ptrdiff_t operator-(const iterator_base<bool>& __x,
		const iterator_base<bool>& __y) {
		return (int(_S_word_bit) * (__x._M_p - __y._M_p)
			+ __x._M_offset - __y._M_offset);
	}

	template<typename _Tp>
	struct simple_iterator : public iterator_base<_Tp> {
		typedef _Tp&	reference;
		typedef _Tp*	pointer;

		typedef typename iterator_base<_Tp>::difference_type difference_type;

		simple_iterator() : iterator_base<_Tp>(0) { }

		simple_iterator(_Tp * __x)
		: iterator_base<_Tp>(__x) { }

		simple_iterator _M_const_cast() const
		{ return *this; }

		reference operator*() const
		{ return *this->_M_p; }

		pointer operator->() const
		{ return this->_M_p; }

		simple_iterator& operator++() {
			iterator_base<_Tp>::_M_bump_up();
			return *this;
		}

		simple_iterator operator++(int) {
			simple_iterator __tmp = *this;
			iterator_base<_Tp>::_M_bump_up();
			return __tmp;
		}

		simple_iterator& operator--() {
			iterator_base<_Tp>::_M_bump_down();
			return *this;
		}

		simple_iterator operator--(int) {
			simple_iterator __tmp = *this;
			iterator_base<_Tp>::_M_bump_down();
			return __tmp;
		}

		simple_iterator& operator+=(difference_type __i) {
			iterator_base<_Tp>::_M_incr(__i);
			return *this;
		}

		simple_iterator& operator-=(difference_type __i) {
			*this += -__i;
			return *this;
		}

		simple_iterator operator+(difference_type __i) const {
			simple_iterator __tmp = *this;
			return __tmp += __i;
		}

		simple_iterator operator-(difference_type __i) const {
			simple_iterator __tmp = *this;
			return __tmp -= __i;
		}

		reference operator[](difference_type __i) const
		{ return *(*this + __i); }
	};

	template<typename _Tp>
	struct const_simple_iterator : public iterator_base<_Tp> {
		typedef const _Tp&																		reference;
		typedef const _Tp&																		const_reference;
		typedef _Tp*																					pointer;
		typedef const_simple_iterator<_Tp>										const_iterator;

		typedef typename iterator_base<_Tp>::difference_type	difference_type;

		const_simple_iterator() : iterator_base<_Tp>(0) { }

		const_simple_iterator(_Tp * __x)
		: iterator_base<_Tp>(__x) { }

		const_simple_iterator(const simple_iterator<_Tp>& __x)
		: iterator_base<_Tp>(__x._M_p) { }

		simple_iterator<_Tp> _M_const_cast() const
		{ return simple_iterator<_Tp>(this->_M_p); }

		const_reference operator*() const
		{ return *this->_M_p; }

		const_simple_iterator& operator++() {
			iterator_base<_Tp>::_M_bump_up();
			return *this;
		}

		const_simple_iterator operator++(int) {
			const_simple_iterator __tmp = *this;
			iterator_base<_Tp>::_M_bump_up();
			return __tmp;
		}

		const_simple_iterator& operator--() {
			iterator_base<_Tp>::_M_bump_down();
			return *this;
		}

		const_simple_iterator operator--(int) {
			const_simple_iterator __tmp = *this;
			iterator_base<_Tp>::_M_bump_down();
			return __tmp;
		}

		const_simple_iterator& operator+=(difference_type __i) {
			iterator_base<_Tp>::_M_incr(__i);
			return *this;
		}

		const_simple_iterator& operator-=(difference_type __i) {
			*this += -__i;
			return *this;
		}

		const_simple_iterator operator+(difference_type __i) const {
			const_simple_iterator __tmp = *this;
			return __tmp += __i;
		}

		const_simple_iterator operator-(difference_type __i) const {
			const_simple_iterator __tmp = *this;
			return __tmp -= __i;
		}

		const_reference operator[](difference_type __i) const
		{ return *(*this + __i); }
	};

	template<typename _Tp>
	struct handler_iterator : public iterator_base<_Tp> {
		typedef handler<_Tp>	reference;
		typedef handler<_Tp>*	pointer;

		typedef typename iterator_base<_Tp>::difference_type difference_type;

		bool* _M_dirty;

		handler_iterator() : iterator_base<_Tp>(0), _M_dirty(0) { }

		handler_iterator(_Tp * __x, bool* __dirty)
		: iterator_base<_Tp>(__x), _M_dirty(__dirty) { }

		handler_iterator _M_const_cast() const
		{ return *this; }

		reference operator*() const
		{ return reference(this->_M_p, this->_M_dirty); }

		handler_iterator& operator++() {
			iterator_base<_Tp>::_M_bump_up();
			return *this;
		}

		handler_iterator operator++(int) {
			handler_iterator __tmp = *this;
			iterator_base<_Tp>::_M_bump_up();
			return __tmp;
		}

		handler_iterator& operator--() {
			iterator_base<_Tp>::_M_bump_down();
			return *this;
		}

		handler_iterator operator--(int) {
			handler_iterator __tmp = *this;
			iterator_base<_Tp>::_M_bump_down();
			return __tmp;
		}

		handler_iterator& operator+=(difference_type __i) {
			iterator_base<_Tp>::_M_incr(__i);
			return *this;
		}

		handler_iterator& operator-=(difference_type __i) {
			*this += -__i;
			return *this;
		}

		handler_iterator operator+(difference_type __i) const {
			handler_iterator __tmp = *this;
			return __tmp += __i;
		}

		handler_iterator operator-(difference_type __i) const {
			handler_iterator __tmp = *this;
			return __tmp -= __i;
		}

		reference operator[](difference_type __i) const
		{ return *(*this + __i); }
	};

	template<>
	struct handler_iterator<bool> : public iterator_base<bool> {
		typedef handler<bool>	reference;
		typedef handler<bool>* pointer;

		typedef typename iterator_base<bool>::difference_type difference_type;

		bool* _M_dirty;

		handler_iterator() : iterator_base<bool>(0, 0), _M_dirty(0) { }

		handler_iterator(_Bit_type * __x, unsigned int __y, bool* __dirty)
		: iterator_base<bool>(__x, __y), _M_dirty(__dirty) { }

		handler_iterator _M_const_cast() const
		{ return *this; }

		reference operator*() const
		{ return reference(_M_p, 1UL << _M_offset, _M_dirty); }

		handler_iterator& operator++() {
			_M_bump_up();
			return *this;
		}

		handler_iterator operator++(int) {
			handler_iterator __tmp = *this;
			_M_bump_up();
			return __tmp;
		}

		handler_iterator& operator--() {
			_M_bump_down();
			return *this;
		}

		handler_iterator operator--(int) {
			handler_iterator __tmp = *this;
			_M_bump_down();
			return __tmp;
		}

		handler_iterator& operator+=(difference_type __i) {
			_M_incr(__i);
			return *this;
		}

		handler_iterator& operator-=(difference_type __i) {
			*this += -__i;
			return *this;
		}

		handler_iterator operator+(difference_type __i) const {
			handler_iterator __tmp = *this;
			return __tmp += __i;
		}

		handler_iterator operator-(difference_type __i) const {
			handler_iterator __tmp = *this;
			return __tmp -= __i;
		}

		reference operator[](difference_type __i) const
		{ return *(*this + __i); }
	};

	template<typename _Tp>
	struct const_handler_iterator : public iterator_base<_Tp> {
		typedef const _Tp&				reference;
		typedef const _Tp&				const_reference;
		typedef const _Tp*				pointer;
		typedef const_handler_iterator<_Tp>										const_iterator;

		typedef typename iterator_base<_Tp>::difference_type difference_type;

		bool* _M_dirty;

		const_handler_iterator() : iterator_base<_Tp>(0), _M_dirty(0) { }

		const_handler_iterator(_Tp * __x, bool* __dirty)
		: iterator_base<_Tp>(__x), _M_dirty(__dirty) { }

		const_handler_iterator(const handler_iterator<_Tp>& __x)
		: iterator_base<_Tp>(__x._M_p), _M_dirty(__x._M_dirty) { }

		handler_iterator<_Tp> _M_const_cast() const
		{ return handler_iterator<_Tp>(this->_M_p, this->_M_dirty); }

		const_reference operator*() const
		{ return *this->_M_p; }

		const_handler_iterator& operator++() {
			iterator_base<_Tp>::_M_bump_up();
			return *this;
		}

		const_handler_iterator operator++(int) {
			const_handler_iterator __tmp = *this;
			iterator_base<_Tp>::_M_bump_up();
			return __tmp;
		}

		const_handler_iterator& operator--() {
			iterator_base<_Tp>::_M_bump_down();
			return *this;
		}

		const_handler_iterator operator--(int) {
			const_handler_iterator __tmp = *this;
			iterator_base<_Tp>::_M_bump_down();
			return __tmp;
		}

		const_handler_iterator& operator+=(difference_type __i) {
			iterator_base<_Tp>::_M_incr(__i);
			return *this;
		}

		const_handler_iterator& operator-=(difference_type __i) {
			*this += -__i;
			return *this;
		}

		const_handler_iterator operator+(difference_type __i) const {
			const_handler_iterator __tmp = *this;
			return __tmp += __i;
		}

		const_handler_iterator operator-(difference_type __i) const {
			const_handler_iterator __tmp = *this;
			return __tmp -= __i;
		}

		const_reference operator[](difference_type __i) const
		{ return *(*this + __i); }
	};

	template<>
	struct const_handler_iterator<bool> : public iterator_base<bool> {
		typedef bool								reference;
		typedef bool								const_reference;
		typedef const bool*					pointer;
		typedef const_handler_iterator<bool>										const_iterator;

		typedef typename iterator_base<bool>::difference_type difference_type;

		bool* _M_dirty;

		const_handler_iterator() : iterator_base<bool>(0, 0), _M_dirty(0) { }

		const_handler_iterator(_Bit_type * __x, unsigned int __y, bool* __dirty)
		: iterator_base(__x, __y), _M_dirty(__dirty) { }

		const_handler_iterator(const handler_iterator<bool>& __x)
		: iterator_base(__x._M_p, __x._M_offset), _M_dirty(__x._M_dirty) { }

		handler_iterator<bool> _M_const_cast() const
		{ return handler_iterator<bool>(_M_p, _M_offset, _M_dirty); }

		const_reference operator*() const
		{ return handler<bool>(_M_p, (1UL << _M_offset), _M_dirty); }

		const_handler_iterator& operator++() {
			_M_bump_up();
			return *this;
		}

		const_handler_iterator operator++(int) {
			const_handler_iterator __tmp = *this;
			_M_bump_up();
			return __tmp;
		}

		const_handler_iterator& operator--() {
			_M_bump_down();
			return *this;
		}

		const_handler_iterator operator--(int) {
			const_handler_iterator __tmp = *this;
			_M_bump_down();
			return __tmp;
		}

		const_handler_iterator& operator+=(difference_type __i) {
			_M_incr(__i);
			return *this;
		}

		const_handler_iterator& operator-=(difference_type __i) {
			*this += -__i;
			return *this;
		}

		const_handler_iterator operator+(difference_type __i) const {
			const_handler_iterator __tmp = *this;
			return __tmp += __i;
		}

		const_handler_iterator operator-(difference_type __i) const {
			const_handler_iterator __tmp = *this;
			return __tmp -= __i;
		}

		const_reference operator[](difference_type __i) const
		{ return *(*this + __i); }
	};

	template<typename _Tp>
	inline simple_iterator<_Tp> operator+(ptrdiff_t __n,
		const simple_iterator<_Tp>& __x)
	{ return __x + __n; }

	template<typename _Tp>
	inline const_simple_iterator<_Tp> operator+(ptrdiff_t __n,
		const const_simple_iterator<_Tp>& __x)
	{ return __x + __n; }

	template<typename _Tp>
	inline handler_iterator<_Tp> operator+(ptrdiff_t __n,
		const handler_iterator<_Tp>& __x)
	{ return __x + __n; }

	template<typename _Tp>
	inline const_handler_iterator<_Tp> operator+(ptrdiff_t __n,
		const const_handler_iterator<_Tp>& __x)
	{ return __x + __n; }

	inline void __fill_bvector(handler_iterator<bool> __first,
		handler_iterator<bool> __last, bool __x) {
		for (; __first != __last; ++__first)
			*__first = __x;
	}
} // namespace inaccel

namespace std _GLIBCXX_VISIBILITY(default)
{
	inline void fill(inaccel::handler_iterator<bool> __first,
		inaccel::handler_iterator<bool> __last, const bool& __x) {
		if (__first._M_p != __last._M_p) {
			std::fill(__first._M_p + 1, __last._M_p, __x ? ~0 : 0);
			__fill_bvector(__first,
				inaccel::handler_iterator<bool>(__first._M_p + 1, 0, __first._M_dirty)
				, __x);
			__fill_bvector(
				inaccel::handler_iterator<bool>(__last._M_p, 0, __last._M_dirty),
				__last, __x);
		}
		else __fill_bvector(__first, __last, __x);
	}
} // namespace std

#endif // ITERATOR_HPP_
