#ifndef HANDLER_HPP_
#define HANDLER_HPP_

namespace inaccel {
	template <typename _Tp>
	struct handler {
		typedef _Tp value_type;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;

		bool* _dirty;
		pointer _ptr;

		handler(pointer ptr, bool* dirty) : _ptr(ptr), _dirty(dirty) {}
		operator value_type() { return (*_ptr); }
		operator value_type() const { return (*_ptr); }

		pointer base() { return _ptr; }

		//Assignment operators
		reference operator=(const_reference value) {
			(*_ptr) = value;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		reference operator=(const handler& value)
		{ return *this = value_type(value); }

		reference operator+=(const_reference value) {
			(*_ptr) += value;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		reference operator-=(const_reference value) {
			(*_ptr) -= value;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		reference operator*=(const_reference value) {
			(*_ptr) *= value;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		reference operator/=(const_reference value) {
			(*_ptr) /= value;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		reference operator%=(const_reference value) {
			(*_ptr) %= value;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		reference operator&=(const_reference value) {
			(*_ptr) &= value;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		reference operator|=(const_reference value) {
			(*_ptr) |= value;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		reference operator^=(const_reference value) {
			(*_ptr) ^= value;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		reference operator<<=(const_reference value) {
			(*_ptr) <<= value;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		reference operator>>=(const_reference value) {
			(*_ptr) >>= value;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		//Increment/decrement operators
		reference operator++() {
			(*_ptr)++;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		reference operator--() {
			(*_ptr)--;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		value_type operator++(int) {
			(*_ptr)++;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		value_type operator--(int) {
			(*_ptr)--;
			if(!*_dirty) *_dirty = true;
			return (*_ptr);
		}

		// //Arithmetic operators
		value_type operator+() const { return +(*_ptr); }

		value_type operator-() const { return -(*_ptr); }

		template <typename _Tp2>
		value_type operator+(const _Tp2 & value) const
		{ return (*_ptr) + value; }

		template <typename _Tp2>
		value_type operator-(const _Tp2 & value) const
		{ return (*_ptr) - value; }

		template <typename _Tp2>
		value_type operator*(const _Tp2 & value) const
		{ return (*_ptr) * value; }

		template <typename _Tp2>
		value_type operator/(const _Tp2 & value) const
		{ return (*_ptr) / value; }

		template <typename _Tp2>
		value_type operator%(const _Tp2 & value) const
		{ return (*_ptr) % value; }

		value_type operator~() const { return ~(*_ptr); }

		template <typename _Tp2>
		value_type operator&(const _Tp2 & value) const
		{ return (*_ptr) & value; }

		template <typename _Tp2>
		value_type operator|(const _Tp2 & value) const
		{ return (*_ptr) | value; }

		template <typename _Tp2>
		value_type operator^(const _Tp2 & value) const
		{ return (*_ptr) ^ value; }

		template <typename _Tp2>
		value_type operator<<(const _Tp2 & value) const
		{ return (*_ptr) << value; }

		template <typename _Tp2>
		value_type operator>>(const _Tp2 & value) const
		{ return (*_ptr) >> value; }

		//Logical operators
		value_type operator!() const { return !(*_ptr); }

		template <typename _Tp2>
		value_type operator&&(const _Tp2 & value) const
		{ return (*_ptr) && value; }

		template <typename _Tp2>
		value_type operator||(const _Tp2 & value) const
		{ return (*_ptr) || value; }

		//Comparison operators
		template <typename _Tp2>
		bool operator==(const _Tp2 & value) const { return (*_ptr) == value; }

		template <typename _Tp2>
		bool operator!=(const _Tp2 & value) const { return (*_ptr) != value; }

		template <typename _Tp2>
		bool operator<(const _Tp2 & value) const { return (*_ptr) < value; }

		template <typename _Tp2>
		bool operator>(const _Tp2 & value) const { return (*_ptr) > value; }

		template <typename _Tp2>
		bool operator<=(const _Tp2 & value) const { return (*_ptr) <= value; }

		template <typename _Tp2>
		bool operator>=(const _Tp2 & value) const { return (*_ptr) >= value; }
	};

	typedef unsigned long _Bit_type;
	enum { _S_word_bit = int(__CHAR_BIT__ * sizeof(_Bit_type)) };

	template<>
	struct handler<bool> {
		typedef _Bit_type value_type;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;

		_Bit_type* _ptr;
		_Bit_type _mask;
		bool* _dirty;

		handler(_Bit_type * __x, _Bit_type __y, bool* __dirty)
		: _ptr(__x), _mask(__y), _dirty(__dirty) { }

		handler() _GLIBCXX_NOEXCEPT : _ptr(0), _mask(0), _dirty(0) { }

		pointer base() { return _ptr; }

		operator bool() const _GLIBCXX_NOEXCEPT
		{ return !!(*_ptr & _mask); }

		handler& operator=(bool __x) _GLIBCXX_NOEXCEPT {
			if (__x) *_ptr |= _mask;
			else 		 *_ptr &= ~_mask;
			*_dirty = true;
			return *this;
		}

		handler& operator=(const handler& __x) _GLIBCXX_NOEXCEPT
		{ return *this = bool(__x); }

		bool operator==(const handler& __x) const
		{ return bool(*this) == bool(__x); }

		bool operator<(const handler& __x) const
		{ return !bool(*this) && bool(__x); }

		void flip() _GLIBCXX_NOEXCEPT {
			*_ptr ^= _mask;
			*_dirty = true;
		}
	};
} // namespace inaccel

namespace std _GLIBCXX_VISIBILITY(default)
{
#if __cplusplus >= 201103L
	template <typename _Tp>
	inline void swap(inaccel::handler<_Tp> __x, inaccel::handler<_Tp> __y) noexcept {
		bool __tmp = __x;
		__x = __y;
		__y = __tmp;
	}

	template <typename _Tp>
	inline void swap(inaccel::handler<_Tp> __x, _Tp& __y) noexcept {
		_Tp __tmp = __x;
		__x = __y;
		__y = __tmp;
	}

	template <typename _Tp>
	inline void swap(_Tp& __x, inaccel::handler<_Tp> __y) noexcept {
		_Tp __tmp = __x;
		__x = __y;
		__y = __tmp;
	}
#endif
} // namespace std

#endif // HANDLER_HPP_
