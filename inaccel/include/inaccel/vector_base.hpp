#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifndef VECTOR_BASE_HPP_
#define VECTOR_BASE_HPP_

#include <vector>
#include "allocator.hpp"
#include "iterator.hpp"

namespace inaccel {

	template<typename _Tp, class Enable = void>
	class _Vector_base {
	protected:
		typedef inaccel::allocator<_Tp> _Alloc;
		typedef typename __gnu_cxx::__alloc_traits<_Alloc>::template
			rebind<_Tp>::other _Alloc_type;
		typedef typename __gnu_cxx::__alloc_traits<_Alloc_type>
			_Alloc_traits;

	public:
		typedef _Tp																				value_type;
		typedef typename _Alloc_traits::pointer 					_real_pointer;
		typedef typename _Alloc_traits::pointer						pointer;
		typedef typename _Alloc_traits::const_pointer			const_pointer;
		typedef typename _Alloc_traits::reference					reference;
		typedef typename _Alloc_traits::const_reference		const_reference;

		typedef typename inaccel::simple_iterator<_Tp>				iterator;
		typedef typename inaccel::const_simple_iterator<_Tp>	const_iterator;

		typedef std::false_type 															fundamental_type;

		struct _Vector_impl : public _Alloc_type {
			pointer _M_start;
			pointer _M_finish;
			pointer _M_end_of_storage;
			bool _M_dirty;

			_Vector_impl() : _Alloc_type(), _M_start(), _M_finish(),
				_M_end_of_storage(), _M_dirty() { }
		};

		typedef _Alloc allocator_type;

		_Alloc_type& _M_get_Tp_allocator() _GLIBCXX_NOEXCEPT
		{ return *static_cast<_Alloc_type*>(&_M_impl); }

		const _Alloc_type& _M_get_Tp_allocator() const _GLIBCXX_NOEXCEPT
		{ return *static_cast<const _Alloc_type*>(&_M_impl); }

		allocator_type get_allocator() const _GLIBCXX_NOEXCEPT
		{ return allocator_type(); }

		_Vector_base() : _M_impl() { }

		_Vector_base(size_t __n) : _M_impl() { _M_create_storage(__n); }

#if __cplusplus >= 201103L
		_Vector_base(_Vector_base&& __x) noexcept : _M_impl() {
			std::swap(_M_impl._M_start, __x._M_impl._M_start);
			std::swap(_M_impl._M_finish, __x._M_impl._M_finish);
			std::swap(_M_impl._M_end_of_storage, __x._M_impl._M_end_of_storage);
			std::swap(_M_impl._M_dirty, __x._M_impl._M_dirty);
		}
#endif

		~_Vector_base() {
			std::_Destroy(_M_impl._M_start, _M_impl._M_finish,
					_M_get_Tp_allocator());
			_M_deallocate();
		}

		size_t max_size() const
		{ return _Alloc_traits::max_size(_M_get_Tp_allocator()); }

		size_t size() const
		{ return size_t(end() - begin()); }

		size_t capacity() const
		{ return size_t(_M_impl._M_end_of_storage - _M_impl._M_start); }

		iterator begin() { return iterator(_M_impl._M_start); }

		const_iterator begin() const { return const_iterator(_M_impl._M_start); }

		iterator end() { return iterator(_M_impl._M_finish); }

		const_iterator end() const { return const_iterator(_M_impl._M_finish); }

		reference operator[](size_t __n) {
			__glibcxx_requires_subscript(__n);
			return *(_M_impl._M_start + __n);
		}

		const_reference operator[](size_t __n) const {
			__glibcxx_requires_subscript(__n);
			return *(_M_impl._M_start + __n);
		}

		_Tp* data() { return _M_data_ptr(_M_impl._M_start); }

		const _Tp* data() const { return _M_data_ptr(_M_impl._M_start); }

		void push_back(const _Tp& __x) {
			if (_M_impl._M_finish != _M_impl._M_end_of_storage)
			{
				_Alloc_traits::construct(_M_impl,
					_M_impl._M_finish, __x);
				++_M_impl._M_finish;
			}
			else
				_M_realloc_insert(end(), __x);
			if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
		}

#if __cplusplus >= 201103L
		void push_back(_Tp&& __x) {
			emplace_back(std::move(__x));
			if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
		}

		template<typename... _Args>
#if __cplusplus > 201402L
		typename reference
#else
		void
#endif
		emplace_back(_Args&&... __args) {
			if (_M_impl._M_finish != _M_impl._M_end_of_storage)
			{
				_Alloc_traits::construct(_M_impl, _M_impl._M_finish,
					std::forward<_Args>(__args)...);
				++_M_impl._M_finish;
			}
			else
				_M_realloc_insert(end(), std::forward<_Args>(__args)...);
#if __cplusplus > 201402L
			return back();
#endif
		}
#endif

		void pop_back() {
		 __glibcxx_requires_nonempty();
		 --_M_impl._M_finish;
		 _Alloc_traits::destroy(_M_impl, _M_impl._M_finish);
		}

		void reserve(size_t __n) {
			if (__n > max_size())
				std::__throw_length_error(__N("vector::reserve"));
			if (capacity() < __n) {
				_M_reallocate(__n);
				if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
			}
		}

#if __cplusplus >= 201103L
		iterator insert(const_iterator __position, const value_type& __x)
#else
		iterator insert(iterator __position, const value_type& __x)
#endif
		{
			const size_t __n = __position - begin();
			if (_M_impl._M_finish != _M_impl._M_end_of_storage)
				if (__position == end()) {
					_Alloc_traits::construct(_M_impl, _M_impl._M_finish,
						__x);
					++_M_impl._M_finish;
				}
				else {
#if __cplusplus >= 201103L
					const auto __pos = begin() + (__position - begin());
					_Temporary_value __x_copy(this, __x);
					_M_insert_aux(__pos, std::move(__x_copy._M_val()));
#else
					_M_insert_aux(__position, __x);
#endif
				}
			else
#if __cplusplus >= 201103L
				_M_realloc_insert(begin() + (__position - begin()), __x);
#else
				_M_realloc_insert(__position, __x);
#endif
			if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
			return iterator(_M_impl._M_start + __n);
		}

#if __cplusplus >= 201103L
		bool shrink_to_fit() {
			if (capacity() == size()) return false;
			__try {
				_M_reallocate(size());
				if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
				return true;
			}
			__catch(...)
			{ return false; }
		}
#endif

	protected:

#if __cplusplus >= 201103L
		struct _Temporary_value
		{
			template<typename... _Args>
			explicit _Temporary_value(_Vector_base* __vec, _Args&&... __args)
			: _M_this(__vec)
			{
				_Alloc_traits::construct(_M_this->_M_impl, _M_ptr(),
							 std::forward<_Args>(__args)...);
			}

			~_Temporary_value()
			{ _Alloc_traits::destroy(_M_this->_M_impl, _M_ptr()); }

			_Tp& _M_val() { return *reinterpret_cast<_Tp*>(&__buf); }

		private:
			pointer _M_ptr()
			{ return std::pointer_traits<pointer>::pointer_to(_M_val()); }

			_Vector_base* _M_this;
			typename std::aligned_storage<sizeof(_Tp), alignof(_Tp)>::type __buf;
		};
#endif

		_Vector_impl _M_impl;

		pointer _M_allocate(size_t __n)
		{ return __n != 0 ? _Alloc_traits::allocate(_M_impl, __n) : pointer(); }

		void _M_deallocate()
		{ _Alloc_traits::deallocate(_M_impl, _M_impl._M_start,
				_M_impl._M_end_of_storage - _M_impl._M_start); }

		void _M_create_storage(size_t __n) {
			_M_impl._M_start = _M_allocate(__n);
			_M_impl._M_finish = _M_impl._M_start;
			_M_impl._M_end_of_storage = _M_impl._M_start + __n;
		}

		size_t _M_check_len(size_t __n, const char* __s) const {
			if (max_size() - size() < __n)
				std::__throw_length_error(__N(__s));

			const size_t __len = size() + std::max(size(), __n);
			return (__len < size() || __len > max_size()) ? max_size() : __len;
		}

		void _M_range_check(size_t __n) const {
			if (__n >= size())
				std::__throw_out_of_range(
					__N("vector::_M_range_check: __n  >= size() "));
		}

		template<typename _Up>
		_Up* _M_data_ptr(_Up* __ptr) const _GLIBCXX_NOEXCEPT
		{ return __ptr; }

#if __cplusplus >= 201103L
		template<typename _Ptr>
		typename std::pointer_traits<_Ptr>::element_type*
		_M_data_ptr(_Ptr __ptr) const
		{ return (begin() == end()) ? nullptr : std::__addressof(*__ptr); }
#else
		template<typename _Up>
		_Up* _M_data_ptr(_Up* __ptr) _GLIBCXX_NOEXCEPT
		{ return __ptr; }

		template<typename _Ptr>
		_Tp* _M_data_ptr(_Ptr __ptr)
		{ return __ptr.operator->(); }

		template<typename _Ptr>
		const _Tp* _M_data_ptr(_Ptr __ptr) const
		{ return __ptr.operator->(); }
#endif

#if __cplusplus >= 201103L
		void _M_default_initialize(size_t __n) {
			std::__uninitialized_default_n_a(_M_impl._M_start, __n,
									_M_get_Tp_allocator());
		_M_impl._M_finish = _M_impl._M_end_of_storage;
		}

		void _M_default_append(size_t __n) {
			if (__n != 0) {
				if (size_t(_M_impl._M_end_of_storage
					- _M_impl._M_finish) >= __n) {
						std::__uninitialized_default_n_a(_M_impl._M_finish, __n,
												_M_get_Tp_allocator());
					_M_impl._M_finish = _M_impl._M_end_of_storage;
				}
				else {
					const size_t __len = _M_check_len(__n, "vector::_M_default_append");
					const size_t __size = size();
					pointer __new_start(_M_allocate(__len));
					pointer __destroy_from = pointer();
					__try {
						std::__uninitialized_default_n_a(__new_start + __size,
							__n, _M_get_Tp_allocator());
						__destroy_from = __new_start + __size;
						std::__uninitialized_move_if_noexcept_a(
							_M_impl._M_start, _M_impl._M_finish,
							__new_start, _M_get_Tp_allocator());
					}
					__catch(...) {
						if (__destroy_from)
							std::_Destroy(__destroy_from, __destroy_from + __n,
								_M_get_Tp_allocator());
						_Alloc_traits::deallocate(_M_impl, __new_start, __len);
						__throw_exception_again;
					}
					std::_Destroy(_M_impl._M_start, _M_impl._M_finish,
						_M_get_Tp_allocator());
					_M_deallocate();
					_M_impl._M_start = __new_start;
					_M_impl._M_finish = __new_start + __size + __n;
					_M_impl._M_end_of_storage = __new_start + __len;
				}
			}
		}
#endif

		void _M_fill_initialize(size_t __n, const _Tp& __value) {
			_M_impl._M_finish =
				std::__uninitialized_fill_n_a(_M_impl._M_start, __n, __value,
							_M_get_Tp_allocator());
		}

		void _M_fill_insert(iterator __position, size_t __n, const _Tp& __x) {
			if (__n != 0) {
				if (size_t(_M_impl._M_end_of_storage
					- _M_impl._M_finish) >= __n) {
#if __cplusplus < 201103L
					_Tp __x_copy = __x;
#else
					_Temporary_value __tmp(this, __x);
					_Tp& __x_copy = __tmp._M_val();
#endif
					const size_t __elems_after = end() - __position;
					pointer __old_finish(_M_impl._M_finish);
					if (__elems_after > __n) {
						std::__uninitialized_move_a(_M_impl._M_finish - __n,
							_M_impl._M_finish, _M_impl._M_finish,
							_M_get_Tp_allocator());
						_M_impl._M_finish += __n;
						_GLIBCXX_MOVE_BACKWARD3(__position.base(),
							__old_finish - __n, __old_finish);
						std::fill(__position.base(), __position.base() + __n, __x_copy);
					}
					else {
						_M_impl._M_finish =
							std::__uninitialized_fill_n_a(_M_impl._M_finish,
								__n - __elems_after, __x_copy, _M_get_Tp_allocator());
						std::__uninitialized_move_a(__position.base(), __old_finish,
							_M_impl._M_finish, _M_get_Tp_allocator());
						_M_impl._M_finish += __elems_after;
						std::fill(__position.base(), __old_finish, __x_copy);
					}
				}
				else {
					const size_t __len = _M_check_len(__n, "vector::_M_fill_insert");
					const size_t __elems_before = __position - begin();
					pointer __new_start(_M_allocate(__len));
					pointer __new_finish(__new_start);
					__try {
						std::__uninitialized_fill_n_a(__new_start + __elems_before,
							__n, __x, _M_get_Tp_allocator());
						__new_finish = pointer();
						__new_finish = std::__uninitialized_move_if_noexcept_a
							(_M_impl._M_start, __position.base(),
								__new_start, _M_get_Tp_allocator());
						__new_finish += __n;
						__new_finish = std::__uninitialized_move_if_noexcept_a
							(__position.base(), _M_impl._M_finish,
								__new_finish, _M_get_Tp_allocator());
					}
					__catch(...) {
						if (!__new_finish)
							std::_Destroy(__new_start + __elems_before,
								__new_start + __elems_before + __n, _M_get_Tp_allocator());
						else
							std::_Destroy(__new_start, __new_finish, _M_get_Tp_allocator());
						_Alloc_traits::deallocate(_M_impl, __new_start, __len);
						__throw_exception_again;
					}
					std::_Destroy(_M_impl._M_start, _M_impl._M_finish,
						_M_get_Tp_allocator());
					_M_deallocate();
					_M_impl._M_start = __new_start;
					_M_impl._M_finish = __new_finish;
					_M_impl._M_end_of_storage = __new_start + __len;
				}
			}
		}

		void _M_fill_assign(size_t __n, const _Tp& __val) {
			if (__n > capacity()) {
				_M_deallocate();
				_M_create_storage(__n);
				_M_fill_initialize(__n, __val);
			}
			else if (__n > size()) {
				std::fill(begin(), end(), __val);
				_M_impl._M_finish =
					std::__uninitialized_fill_n_a(_M_impl._M_finish,
						__n - size(), __val, _M_get_Tp_allocator());
			}
			else
				_M_erase_at_end(std::fill_n(_M_impl._M_start, __n, __val));
		}

		void _M_reallocate(size_t __n) {
			const size_t __old_size = size();
			pointer __tmp = _M_allocate(__n);
			__try {
				std::__uninitialized_copy_a(
					_GLIBCXX_MAKE_MOVE_IF_NOEXCEPT_ITERATOR(_M_impl._M_start),
					_GLIBCXX_MAKE_MOVE_IF_NOEXCEPT_ITERATOR(_M_impl._M_finish),
					__tmp, _M_get_Tp_allocator());
			}
			__catch(...) {
				_Alloc_traits::deallocate(_M_impl, __tmp, __n);
				__throw_exception_again;
			}
			std::_Destroy(_M_impl._M_start, _M_impl._M_finish,
				_M_get_Tp_allocator());
			_M_deallocate();
			_M_impl._M_start = __tmp;
			_M_impl._M_finish = __tmp + __old_size;
			_M_impl._M_end_of_storage = _M_impl._M_start + __n;
		}

#if __cplusplus >= 201103L
		template<typename... _Args>
		void _M_realloc_insert(iterator __position, _Args&&... __args)
#else
		void _M_realloc_insert(iterator __position, const _Tp& __x)
#endif
		{
			const size_t __len =
				_M_check_len(size_t(1), "vector::_M_realloc_insert");
			const size_t __elems_before = __position - begin();
			pointer __new_start(_M_allocate(__len));
			pointer __new_finish(__new_start);
			__try {
				_Alloc_traits::construct(_M_impl,
					__new_start + __elems_before,
#if __cplusplus >= 201103L
					std::forward<_Args>(__args)...);
#else
					__x);
#endif
				__new_finish = pointer();

				__new_finish = std::__uninitialized_move_if_noexcept_a
						(_M_impl._M_start, __position.base(),
							__new_start, _M_get_Tp_allocator());

				++__new_finish;

				__new_finish = std::__uninitialized_move_if_noexcept_a
						(__position.base(), _M_impl._M_finish,
							__new_finish, _M_get_Tp_allocator());
			}
			__catch(...) {
				if (!__new_finish)
					_Alloc_traits::destroy(_M_impl,
						__new_start + __elems_before);
				else
					std::_Destroy(__new_start, __new_finish, _M_get_Tp_allocator());
				_Alloc_traits::deallocate(_M_impl, __new_start, __len);
				__throw_exception_again;
			}
			std::_Destroy(_M_impl._M_start, _M_impl._M_finish,
				_M_get_Tp_allocator());
			_M_deallocate();
			_M_impl._M_start = __new_start;
			_M_impl._M_finish = __new_finish;
			_M_impl._M_end_of_storage = __new_start + __len;
		}

		template<typename _Integer>
		void _M_initialize_dispatch(_Integer __n, _Integer __value,
			std::__true_type) {
			_M_create_storage(static_cast<size_t>(__n));
			_M_fill_initialize(static_cast<size_t>(__n), __value);
		}

		template<typename _InputIterator>
		void _M_initialize_dispatch(_InputIterator __first, _InputIterator __last,
								std::__false_type) {
			typedef typename std::iterator_traits<_InputIterator>::
				iterator_category _IterCategory;
			_M_range_initialize(__first, __last, _IterCategory());
		}

		template<typename _InputIterator>
		void _M_range_initialize(_InputIterator __first, _InputIterator __last,
						std::input_iterator_tag) {
			__try {
				for (; __first != __last; ++__first)
#if __cplusplus >= 201103L
					emplace_back(*__first);
#else
					push_back(*__first);
#endif
			} __catch(...) {
				_M_erase_at_end(begin());
				__throw_exception_again;
			}
		}

		template<typename _ForwardIterator>
		void _M_range_initialize(_ForwardIterator __first, _ForwardIterator __last,
						std::forward_iterator_tag) {
			const size_t __n = std::distance(__first, __last);
			_M_create_storage(static_cast<size_t>(__n));
			_M_impl._M_finish =
				std::__uninitialized_copy_a(__first, __last,
						_M_impl._M_start,
						_M_get_Tp_allocator());
		}

		template<typename _InputIterator>
		void _M_range_insert(iterator __position, _InputIterator __first,
				_InputIterator __last, std::input_iterator_tag) {
			for (; __first != __last; ++__first) {
				__position = insert(__position, *__first);
				++__position;
			}
		}

		template<typename _ForwardIterator>
		void _M_range_insert(iterator __position, _ForwardIterator __first,
				_ForwardIterator __last, std::forward_iterator_tag) {
			if (__first != __last) {
				const size_t __n = std::distance(__first, __last);
				if (size_t(_M_impl._M_end_of_storage
					- _M_impl._M_finish) >= __n) {
					const size_t __elems_after = end() - __position;
					pointer __old_finish(_M_impl._M_finish);
					if (__elems_after > __n) {
						std::__uninitialized_move_a(_M_impl._M_finish - __n,
							_M_impl._M_finish, _M_impl._M_finish,
							_M_get_Tp_allocator());
						_M_impl._M_finish += __n;
						_GLIBCXX_MOVE_BACKWARD3(__position.base(),
							__old_finish - __n, __old_finish);
						std::copy(__first, __last, __position);
					}
					else {
						_ForwardIterator __mid = __first;
						std::advance(__mid, __elems_after);
						std::__uninitialized_copy_a(__mid, __last, _M_impl._M_finish,
							_M_get_Tp_allocator());
						_M_impl._M_finish += __n - __elems_after;
						std::__uninitialized_move_a(__position.base(), __old_finish,
							_M_impl._M_finish, _M_get_Tp_allocator());
						_M_impl._M_finish += __elems_after;
						std::copy(__first, __mid, __position);
					}
				}
				else {
					const size_t __len = _M_check_len(__n, "vector::_M_range_insert");
					pointer __new_start(_M_allocate(__len));
					pointer __new_finish(__new_start);
					__try {
						__new_finish = std::__uninitialized_move_if_noexcept_a
							(_M_impl._M_start, __position.base(),
							__new_start, _M_get_Tp_allocator());
						__new_finish = std::__uninitialized_copy_a(__first, __last,
							__new_finish, _M_get_Tp_allocator());
						__new_finish = std::__uninitialized_move_if_noexcept_a
							(__position.base(), _M_impl._M_finish,
							__new_finish, _M_get_Tp_allocator());
					}
					__catch(...) {
						std::_Destroy(__new_start, __new_finish, _M_get_Tp_allocator());
						_Alloc_traits::deallocate(_M_impl, __new_start, __len);
						__throw_exception_again;
					}
					std::_Destroy(_M_impl._M_start, _M_impl._M_finish,
						_M_get_Tp_allocator());
					_M_deallocate();
					_M_impl._M_start = __new_start;
					_M_impl._M_finish = __new_finish;
					_M_impl._M_end_of_storage = __new_start + __len;
				}
			}
		}

		template<typename _InputIterator>
		void _M_assign_aux(_InputIterator __first, _InputIterator __last,
						std::input_iterator_tag) {
			pointer __cur(_M_impl._M_start);
			for (; __first != __last && __cur != _M_impl._M_finish;
				++__cur, ++__first)
				*__cur = *__first;
			if (__first == __last)
				_M_erase_at_end(__cur);
			else
				_M_range_insert(end(), __first, __last,
					std::__iterator_category(__first));
		}

		template<typename _ForwardIterator>
		void _M_assign_aux(_ForwardIterator __first, _ForwardIterator __last,
			std::forward_iterator_tag) {
			const size_t __len = std::distance(__first, __last);
			if (__len < size())
				_M_erase_at_end(std::copy(__first, __last, begin()));
			else {
				_ForwardIterator __mid = __first;
				std::advance(__mid, size());
				std::copy(__first, __mid, begin());
				_M_range_insert(end(), __mid, __last,
					std::__iterator_category(__mid));
			}
		}

#if __cplusplus >= 201103L
		template<typename _Arg>
		void _M_insert_aux(iterator __position, _Arg&& __arg)
#else
		void _M_insert_aux(iterator __position, const _Tp& __x)
#endif
		{
			_Alloc_traits::construct(_M_impl, _M_impl._M_finish,
				_GLIBCXX_MOVE(*(_M_impl._M_finish - 1)));
			++_M_impl._M_finish;
#if __cplusplus < 201103L
			_Tp __x_copy = __x;
#endif
			_GLIBCXX_MOVE_BACKWARD3(__position.base(), _M_impl._M_finish - 2,
				_M_impl._M_finish - 1);
#if __cplusplus < 201103L
			*__position = __x_copy;
#else
			*__position = std::forward<_Arg>(__arg);
#endif
		}

#if __cplusplus >= 201103L
		iterator _M_insert_rval(const_iterator __position, value_type&& __v) {
			const auto __n = __position - begin();
			if (_M_impl._M_finish != _M_impl._M_end_of_storage)
				if (__position == end()) {
					_Alloc_traits::construct(_M_impl, _M_impl._M_finish,
						std::move(__v));
					++_M_impl._M_finish;
				}
				else
					_M_insert_aux(begin() + __n, std::move(__v));
			else
				_M_realloc_insert(begin() + __n, std::move(__v));
			return iterator(_M_impl._M_start + __n);
		}
#endif

		void _M_erase_at_end(iterator __pos) {
			std::_Destroy(__pos._M_p, _M_impl._M_finish, _M_get_Tp_allocator());
			_M_impl._M_finish = __pos._M_p;
		}

		iterator _M_erase(iterator __position) {
			if (__position + 1 != end())
				_GLIBCXX_MOVE3(__position + 1, end(), __position);
			--_M_impl._M_finish;
			_Alloc_traits::destroy(_M_impl, _M_impl._M_finish);
			return __position;
		}

		iterator _M_erase(iterator __first, iterator __last) {
			if (__first != __last) {
				if (__last != end())
					_GLIBCXX_MOVE3(__last, end(), __first);
				_M_erase_at_end(__first.base() + (end() - __last));
			}
			return __first;
		}

		void _M_move_assign(_Vector_base&& __x) noexcept {
			std::swap(_M_impl._M_start, __x._M_impl._M_start);
			std::swap(_M_impl._M_finish, __x._M_impl._M_finish);
			std::swap(_M_impl._M_end_of_storage, __x._M_impl._M_end_of_storage);
			std::swap(_M_impl._M_dirty, __x._M_impl._M_dirty);
		}

		void _M_copy_assign(const _Vector_base& __x) {
			if (&__x != this) {
				const size_t __xlen = __x.size();
				if (__xlen > capacity()) {
					_M_deallocate();
					_M_range_initialize(__x.begin(), __x.end(),
															std::forward_iterator_tag());
				}
				else if (size() >= __xlen) {
					_M_erase_at_end(std::copy(__x.begin(), __x.end(), begin()));
				}
				else {
					std::copy(__x._M_impl._M_start, __x._M_impl._M_start + size(),
						_M_impl._M_start);
					std::__uninitialized_copy_a(__x._M_impl._M_start + size(),
						__x._M_impl._M_finish, _M_impl._M_finish,
						_M_get_Tp_allocator());
				}
				_M_impl._M_finish = _M_impl._M_start + __xlen;
			}
		}
	};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	template <class _Tp>
	class _Vector_base <_Tp,
		typename std::enable_if<std::is_fundamental<_Tp>::value>::type> {
	protected:
		typedef typename inaccel::allocator<_Tp>							_Alloc;
		typedef typename __gnu_cxx::__alloc_traits<_Alloc>::template
			rebind<_Tp>::other _Alloc_type;
		typedef typename __gnu_cxx::__alloc_traits<_Alloc_type>
			_Alloc_traits;
		typedef typename inaccel::handler<_Tp>								_handler;

	public:
		typedef _Tp																				value_type;
		typedef typename _Alloc_traits::pointer 					_real_pointer;
		typedef _handler*																	pointer;
		typedef typename _Alloc_traits::const_pointer			const_pointer;
		typedef _handler																	reference;
		typedef typename _Alloc_traits::const_reference		const_reference;

		typedef typename inaccel::handler_iterator<_Tp>				iterator;
		typedef typename inaccel::const_handler_iterator<_Tp>	const_iterator;

		typedef std::true_type 																fundamental_type;

		struct _Vector_impl : public _Alloc_type {
			iterator _M_start;
			iterator _M_finish;
			_real_pointer _M_end_of_storage;
			bool _M_dirty;

			_Vector_impl() : _Alloc_type(), _M_start(), _M_finish(),
				_M_end_of_storage(), _M_dirty() { }
		};

		typedef _Alloc allocator_type;

		_Alloc_type& _M_get_Tp_allocator() _GLIBCXX_NOEXCEPT
		{ return *static_cast<_Alloc_type*>(&_M_impl); }

		const _Alloc_type& _M_get_Tp_allocator() const _GLIBCXX_NOEXCEPT
		{ return *static_cast<const _Alloc_type*>(&_M_impl); }

		allocator_type get_allocator() const _GLIBCXX_NOEXCEPT
		{ return allocator_type(); }

		_Vector_base() : _M_impl() { }

		_Vector_base(size_t __n) : _M_impl() { _M_create_storage(__n); }

#if __cplusplus >= 201103L
		_Vector_base(_Vector_base&& __x) noexcept : _M_impl() {
			_M_impl._M_start = __x._M_impl._M_start;
			_M_impl._M_finish = __x._M_impl._M_finish;
			_M_impl._M_end_of_storage = __x._M_impl._M_end_of_storage;
			__x._M_impl._M_start = iterator();
			__x._M_impl._M_finish = iterator();
			__x._M_impl._M_end_of_storage = nullptr;
			std::swap(_M_impl._M_dirty, __x._M_impl._M_dirty);
		}
#endif

		~_Vector_base() { _M_deallocate(); }

		size_t max_size() const
		{ return _Alloc_traits::max_size(_M_get_Tp_allocator()); }

		size_t size() const
		{ return size_t(end() - begin()); }

		size_t capacity() const
		{ return size_t(_M_impl._M_end_of_storage - _M_impl._M_start._M_p); }

		iterator begin() { return _M_impl._M_start; }

		const_iterator begin() const { return const_iterator(_M_impl._M_start); }

		iterator end() { return _M_impl._M_finish; }

		const_iterator end() const { return const_iterator(_M_impl._M_finish); }

		reference operator[](size_t __n) {
			__glibcxx_requires_subscript(__n);
			return _handler(_M_impl._M_start._M_p + __n,
				&_M_impl._M_dirty);
		}

		const_reference operator[](size_t __n) const {
			__glibcxx_requires_subscript(__n);
			return *(_M_impl._M_start._M_p + __n);
		}

		_Tp* data() { return _M_data_ptr(_M_impl._M_start._M_p); }

		const _Tp* data() const { return _M_data_ptr(_M_impl._M_start._M_p); }

		void push_back(const _Tp& __x) {
			if (_M_impl._M_finish._M_p != _M_impl._M_end_of_storage)
				*_M_impl._M_finish++ = __x;
			else
				_M_realloc_insert(end(), __x);
			if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
		}

		#if __cplusplus >= 201103L
		template<typename... _Args>
		#if __cplusplus > 201402L
		reference
		#else
		void
		#endif
		emplace_back(_Args&&... __args) {
			push_back(_Tp(__args...));
		#if __cplusplus > 201402L
			return _M_back();
		#endif
		}
		#endif

		void pop_back() {
		 __glibcxx_requires_nonempty();
		 --_M_impl._M_finish;
		}

		void reserve(size_t __n) {
			if (__n > max_size())
				std::__throw_length_error(__N("vector<fundamental>::reserve"));
			if (capacity() < __n) {
				_M_reallocate(__n);
				if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
			}
		}

		iterator
#if __cplusplus >= 201103L
		insert(const_iterator __position, const _Tp& __x = _Tp())
#else
		insert(iterator __position, const _Tp& __x = _Tp())
#endif
		{
			const ptrdiff_t __n = __position - begin();
			if (_M_impl._M_finish._M_p != _M_impl._M_end_of_storage
				&& __position == end())
				*_M_impl._M_finish++ = __x;
			else
				_M_realloc_insert(__position._M_const_cast(), __x);
			if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
			return begin() + __n;
		}

#if __cplusplus >= 201103L
		bool shrink_to_fit() {
			if (capacity() == size()) return false;
			__try {
				_M_reallocate(size());
				if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
				return true;
			}
			__catch(...)
			{ return false; }
		}
#endif

	protected:
		_Vector_impl _M_impl;

		_real_pointer _M_allocate(size_t __n)
		{ return __n != 0 ? _Alloc_traits::allocate(_M_impl, __n)
												: _real_pointer(); }

		void _M_deallocate() {
			if (_M_impl._M_start._M_p) {
				const size_t __n = _M_impl._M_end_of_storage - _M_impl._M_start._M_p;
				_Alloc_traits::deallocate(_M_impl, _M_impl._M_start._M_p,
					_M_impl._M_end_of_storage - _M_impl._M_start._M_p);
				_M_impl._M_start = _M_impl._M_finish = iterator();
				_M_impl._M_end_of_storage = _real_pointer();
			}
		}

		void _M_create_storage(size_t __n) {
			if (__n) {
				_real_pointer __q = _M_allocate(__n);
				_M_impl._M_end_of_storage = __q + __n;
				_M_impl._M_start = iterator(std::__addressof(*__q),
																						&_M_impl._M_dirty);
			}
			else {
				_M_impl._M_end_of_storage = _real_pointer();
				_M_impl._M_start = iterator(0, 0);
			}
			_M_impl._M_finish = _M_impl._M_start + ptrdiff_t(__n);
		}

		size_t _M_check_len(size_t __n, const char* __s) const {
			if (max_size() - size() < __n)
				std::__throw_length_error(__N(__s));

			const size_t __len = size() + std::max(size(), __n);
			return (__len < size() || __len > max_size()) ? max_size() : __len;
		}

		void _M_range_check(size_t __n) const {
			if (__n >= size())
				std::__throw_out_of_range(__N(
					"vector<fundamental>::_M_range_check: __n  >= size() "));
		}

		template<typename _Up>
		_Up* _M_data_ptr(_Up* __ptr) const _GLIBCXX_NOEXCEPT
		{ return __ptr; }

#if __cplusplus >= 201103L
		template<typename _Ptr>
		typename std::pointer_traits<_Ptr>::element_type*
		_M_data_ptr(_Ptr __ptr) const
		{ return (begin() == end()) ? nullptr : std::__addressof(*__ptr); }
#else
		template<typename _Up>
		_Up* _M_data_ptr(_Up* __ptr) _GLIBCXX_NOEXCEPT
		{ return __ptr; }

		template<typename _Ptr>
		_Tp* _M_data_ptr(_Ptr __ptr)
		{ return __ptr.operator->(); }

		template<typename _Ptr>
		const _Tp* _M_data_ptr(_Ptr __ptr) const
		{ return __ptr.operator->(); }
#endif

#if __cplusplus >= 201103L
		void _M_default_initialize(size_t __n) { _M_create_storage(__n); }

		void _M_default_append(size_t __n) { _M_fill_insert(end(), __n, _Tp()); }
#endif

		void _M_fill_initialize(size_t __n, const _Tp& __value) {
			std::fill(_M_impl._M_start._M_p,
				_M_impl._M_end_of_storage, __value);
		}

		void _M_fill_insert(iterator __position, size_t __n, const _Tp& __x) {
			if (__n == 0) return;
			if (capacity() - size() >= __n) {
				std::copy_backward(__position, end(),
					_M_impl._M_finish + ptrdiff_t(__n));
				std::fill(__position, __position + ptrdiff_t(__n), __x);
				_M_impl._M_finish += ptrdiff_t(__n);
			}
			else {
				const size_t __len =
					_M_check_len(__n, "vector<fundamental>::_M_fill_insert");
				_real_pointer __q = _M_allocate(__len);
				iterator __start(std::__addressof(*__q), &_M_impl._M_dirty);
				iterator __i = std::copy(begin(), __position, __start);
				std::fill(__i, __i + ptrdiff_t(__n), __x);
				iterator __finish = std::copy(__position, end(),
					__i + ptrdiff_t(__n));
				_M_deallocate();
				_M_impl._M_end_of_storage = __q + __len;
				_M_impl._M_start = __start;
				_M_impl._M_finish = __finish;
			}
		}

		void _M_fill_assign(size_t __n, const _Tp& __x) {
			if (__n > size()) {
				std::fill(_M_impl._M_start._M_p,
					_M_impl._M_end_of_storage, __x);
#if __cplusplus >= 201103L
				_M_fill_insert(end()._M_const_cast(), __n - size(), __x);
#else
				_M_fill_insert(end(), __n - size(), __x);
#endif
			}
			else {
				_M_erase_at_end(begin() + __n);
				std::fill(_M_impl._M_start._M_p,
					_M_impl._M_end_of_storage, __x);
			}
		}

		void _M_reallocate(size_t __n) {
			_real_pointer __q = _M_allocate(__n);
			iterator __start(std::__addressof(*__q), &_M_impl._M_dirty);
			iterator __finish(std::copy(begin(), end(), __start));
			_M_deallocate();
			_M_impl._M_start = __start;
			_M_impl._M_finish = __finish;
			_M_impl._M_end_of_storage = __q + __n;
		}

		void _M_realloc_insert(iterator __position, const _Tp& __x) {
			if (_M_impl._M_finish._M_p != _M_impl._M_end_of_storage) {
				std::copy_backward(__position, _M_impl._M_finish,
					_M_impl._M_finish + 1);
				*__position = __x;
				++_M_impl._M_finish;
			}
			else {
				const size_t __len =
					_M_check_len(size_t(1), "vector<fundamental>::_M_realloc_insert");
				_real_pointer __q = _M_allocate(__len);
				iterator __start(std::__addressof(*__q), &_M_impl._M_dirty);
				iterator __i = std::copy(begin(), __position, __start);
				*__i++ = __x;
				iterator __finish = std::copy(__position, end(), __i);
				_M_deallocate();
				_M_impl._M_end_of_storage = __q + __len;
				_M_impl._M_start = __start;
				_M_impl._M_finish = __finish;
			}
		}

		template<typename _Integer>
		void _M_initialize_dispatch(_Integer __n, _Integer __x, std::__true_type) {
			_M_create_storage(static_cast<size_t>(__n));
			std::fill(_M_impl._M_start._M_p, _M_impl._M_end_addr(), __x);
		}

		template<typename _InputIterator>
		void _M_initialize_dispatch(_InputIterator __first, _InputIterator __last,
			std::__false_type) {
			_M_range_initialize(__first, __last,
				std::__iterator_category(__first));
		}

		template<typename _InputIterator>
		void _M_range_initialize(_InputIterator __first, _InputIterator __last,
			std::input_iterator_tag) {
			for (; __first != __last; ++__first)
				push_back(*__first);
		}

		template<typename _ForwardIterator>
		void _M_range_initialize(_ForwardIterator __first, _ForwardIterator __last,
			std::forward_iterator_tag) {
			const size_t __n = std::distance(__first, __last);
			_M_create_storage(__n);
			std::copy(__first, __last, _M_impl._M_start);
		}

		template<typename _InputIterator>
		void _M_range_insert(iterator __position, _InputIterator __first,
			_InputIterator __last, std::input_iterator_tag) {
			for (; __first != __last; ++__first) {
				const ptrdiff_t __n = __position - begin();
				if (_M_impl._M_finish._M_p != _M_impl._M_end_addr()
					&& __position == end())
					*_M_impl._M_finish++ = *__first;
				else
					_M_realloc_insert(__position._M_const_cast(), *__first);
				__position = begin() + __n;

				++__position;
			}
		}

		template<typename _ForwardIterator>
		void _M_range_insert(iterator __position, _ForwardIterator __first,
			_ForwardIterator __last, std::forward_iterator_tag) {
			if (__first != __last) {
				size_t __n = std::distance(__first, __last);
				if (capacity() - size() >= __n) {
					std::copy_backward(__position, end(),
						_M_impl._M_finish + ptrdiff_t(__n));
					std::copy(__first, __last, __position);
					_M_impl._M_finish += ptrdiff_t(__n);
				}
				else {
					const size_t __len =
						_M_check_len(__n, "vector<fundamental>::_M_range_insert");
					_real_pointer __q = _M_allocate(__len);
					iterator __start(std::__addressof(*__q), &_M_impl._M_dirty);
					iterator __i = std::copy(begin(), __position, __start);
					__i = std::copy(__first, __last, __i);
					iterator __finish = std::copy(__position, end(), __i);
					_M_deallocate();
					_M_impl._M_end_of_storage = __q + __len;
					_M_impl._M_start = __start;
					_M_impl._M_finish = __finish;
				}
			}
		}

		template<typename _InputIterator>
		void _M_assign_aux(_InputIterator __first, _InputIterator __last,
			std::input_iterator_tag) {
			iterator __cur = begin();
			for (; __first != __last && __cur != end(); ++__cur, ++__first)
				*__cur = *__first;
			if (__first == __last)
				_M_erase_at_end(__cur);
			else
				_M_range_insert(end(), __first, __last,
					std::__iterator_category(__first));
		}

		template<typename _ForwardIterator>
		void _M_assign_aux(_ForwardIterator __first, _ForwardIterator __last,
			std::forward_iterator_tag) {
			const size_t __len = std::distance(__first, __last);
			if (__len < size())
				_M_erase_at_end(std::copy(__first, __last, begin()));
			else {
				_ForwardIterator __mid = __first;
				std::advance(__mid, size());
				std::copy(__first, __mid, begin());
				_M_range_insert(end(), __mid, __last,
					std::__iterator_category(__mid));
			}
		}

		void _M_insert_aux(iterator __position, const _Tp& __x)
		{ _M_realloc_insert(__position, __x); }

#if __cplusplus >= 201103L
		iterator _M_insert_rval(const_iterator __position, value_type&& __v) {
			_M_realloc_insert(__position._M_const_cast(), __v);
			return __position._M_const_cast();
		}
#endif

		void _M_erase_at_end(iterator __pos) { _M_impl._M_finish = __pos; }

		iterator _M_erase(iterator __position) {
			if (__position + 1 != end())
				std::copy(__position + 1, end(), __position);
			--_M_impl._M_finish;
			return __position;
		}

		iterator _M_erase(iterator __first, iterator __last) {
			if (__first != __last)
				_M_erase_at_end(std::copy(__last, end(), __first));
			return __first;
		}

		void _M_move_assign(_Vector_base&& __x) noexcept {
			_M_impl._M_start = __x._M_impl._M_start;
			_M_impl._M_finish = __x._M_impl._M_finish;
			_M_impl._M_end_of_storage = __x._M_impl._M_end_of_storage;
			__x._M_impl._M_start = iterator();
			__x._M_impl._M_finish = iterator();
			__x._M_impl._M_end_of_storage = nullptr;
			std::swap(_M_impl._M_dirty, __x._M_impl._M_dirty);
		}

		void _M_copy_assign(const _Vector_base& __x) {
			if (&__x != this) {
				const size_t __xlen = __x.size();
				if (__xlen > capacity()) {
					_M_deallocate();
					_M_create_storage(__xlen);
				}
				this->_M_impl._M_finish = std::copy(__x.begin(), __x.end(), begin());
			}
		}
	};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	template<>
	class _Vector_base<bool, void> {
	protected:
		typedef inaccel::allocator<bool> _Alloc;
		typedef __gnu_cxx::__alloc_traits<_Alloc>::template
		rebind<_Bit_type>::other _Alloc_type;
		typedef typename __gnu_cxx::__alloc_traits<_Alloc_type>
			_Alloc_traits;
		typedef inaccel::handler<bool>											_handler;

	public:
		typedef bool																				value_type;
		typedef typename _Alloc_traits::pointer 						_real_pointer;
		typedef _handler*																		pointer;
		typedef typename _Alloc_traits::const_pointer				const_pointer;
		typedef _handler																		reference;
		typedef bool																				const_reference;

		typedef inaccel::handler_iterator<bool>							iterator;
		typedef inaccel::const_handler_iterator<bool>				const_iterator;

		typedef std::true_type 															fundamental_type;

		struct _Vector_impl : public _Alloc_type {
			iterator _M_start;
			iterator _M_finish;
			_real_pointer _M_end_of_storage;
			bool _M_dirty;

			_Vector_impl() : _Alloc_type(), _M_start(), _M_finish(),
				_M_end_of_storage(), _M_dirty() { }

			_Bit_type* _M_end_addr() const _GLIBCXX_NOEXCEPT {
				if (_M_end_of_storage)
					return std::__addressof(_M_end_of_storage[-1]) + 1;
				return 0;
			}
		};

		typedef _Alloc allocator_type;

		_Alloc_type& _M_get_Tp_allocator() _GLIBCXX_NOEXCEPT
		{ return *static_cast<_Alloc_type*>(&_M_impl); }

		const _Alloc_type& _M_get_Tp_allocator() const _GLIBCXX_NOEXCEPT
		{ return *static_cast<const _Alloc_type*>(&_M_impl); }

		allocator_type get_allocator() const _GLIBCXX_NOEXCEPT
		{ return allocator_type(); }

		_Vector_base() : _M_impl() { }

		_Vector_base(size_t __n) : _M_impl() { _M_create_storage(__n); }

#if __cplusplus >= 201103L
		_Vector_base(_Vector_base&& __x) noexcept : _M_impl() {
			_M_impl._M_start = __x._M_impl._M_start;
			_M_impl._M_finish = __x._M_impl._M_finish;
			_M_impl._M_end_of_storage = __x._M_impl._M_end_of_storage;
			__x._M_impl._M_start = iterator();
			__x._M_impl._M_finish = iterator();
			__x._M_impl._M_end_of_storage = nullptr;
			std::swap(_M_impl._M_dirty, __x._M_impl._M_dirty);
		}
#endif

		~_Vector_base() { _M_deallocate(); }

		size_t max_size() const {
			const size_t __isize =
				__gnu_cxx::__numeric_traits<ptrdiff_t>::__max
				- int(_S_word_bit) + 1;
			const size_t __asize =
				_Alloc_traits::max_size(_M_get_Tp_allocator());
			return (__asize <= __isize / int(_S_word_bit)
				? __asize * int(_S_word_bit) : __isize);
		}

		size_t size() const
		{ return size_t(end() - begin()); }

		size_t capacity() const
		{ return size_t(
				const_iterator(_M_impl._M_end_addr(), 0, (bool*)&_M_impl._M_dirty)
				- _M_impl._M_start); }

		iterator begin() { return _M_impl._M_start; }

		const_iterator begin() const  { return const_iterator(_M_impl._M_start); }

		iterator end() { return _M_impl._M_finish; }

		const_iterator end() const  { return const_iterator(_M_impl._M_finish); }

		reference operator[](size_t __n) {
			__glibcxx_requires_subscript(__n);
			return _handler(_M_impl._M_start._M_p + __n / int(_S_word_bit),
				1UL << (__n % int(_S_word_bit)), &_M_impl._M_dirty);
		}

		const_reference operator[](size_t __n) const {
			__glibcxx_requires_subscript(__n);
			return _handler(_M_impl._M_start._M_p + __n / int(_S_word_bit),
				1UL << (__n % int(_S_word_bit)), (bool*)&_M_impl._M_dirty);
		}

		void data() { }

		void push_back(const bool& __x) {
			if (_M_impl._M_finish._M_p != _M_impl._M_end_addr())
				*_M_impl._M_finish++ = __x;
			else
				_M_realloc_insert(end(), __x);
			if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
		}

#if __cplusplus >= 201103L
		template<typename... _Args>
#if __cplusplus > 201402L
		reference
#else
		void
#endif
		emplace_back(_Args&&... __args) {
			push_back(bool(__args...));
#if __cplusplus > 201402L
			return _M_back();
#endif
		}
#endif

		void pop_back() {
		 __glibcxx_requires_nonempty();
		 --_M_impl._M_finish;
		}

		void reserve(size_t __n) {
			if (__n > max_size())
				std::__throw_length_error(__N("vector<bool>::reserve"));
			if (capacity() < __n) {
				_M_reallocate(__n);
				if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
			}
		}

		iterator
#if __cplusplus >= 201103L
		insert(const_iterator __position, const bool& __x = bool())
#else
		insert(iterator __position, const bool& __x = bool())
#endif
		{
			const ptrdiff_t __n = __position - begin();
			if (_M_impl._M_finish._M_p != _M_impl._M_end_addr()
				&& __position == end())
				*_M_impl._M_finish++ = __x;
			else
				_M_realloc_insert(__position._M_const_cast(), __x);
			if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
			return begin() + __n;
		}

#if __cplusplus >= 201103L
		bool shrink_to_fit() {
			if (capacity() - size() < int(_S_word_bit)) return false;
			__try {
				_M_reallocate(size());
				if(!_M_impl._M_dirty) _M_impl._M_dirty = true;
				return true;
			}
			__catch(...)
			{ return false; }
		}
#endif

	protected:
		_Vector_impl _M_impl;

		static size_t _S_nword(size_t __n)
		{ return (__n + int(_S_word_bit) - 1) / int(_S_word_bit); }

		_real_pointer _M_allocate(size_t __n)
		{ return _Alloc_traits::allocate(_M_impl, _S_nword(__n)); }

		void _M_deallocate() {
			if (_M_impl._M_start._M_p) {
				const size_t __n = _M_impl._M_end_addr() - _M_impl._M_start._M_p;
				_Alloc_traits::deallocate(_M_impl,
					_M_impl._M_end_of_storage - __n, __n);
				_M_impl._M_start = _M_impl._M_finish = iterator();
				_M_impl._M_end_of_storage = _real_pointer();
			}
		}

		void _M_create_storage(size_t __n) {
			if (__n) {
				_real_pointer __q = _M_allocate(__n);
				_M_impl._M_end_of_storage = __q + _S_nword(__n);
				_M_impl._M_start = iterator(std::__addressof(*__q), 0,
																						&_M_impl._M_dirty);
			}
			else {
				_M_impl._M_end_of_storage = _real_pointer();
				_M_impl._M_start = iterator(0, 0, 0);
			}
			_M_impl._M_finish = _M_impl._M_start + ptrdiff_t(__n);
		}

		size_t _M_check_len(size_t __n, const char* __s) const {
			if (max_size() - size() < __n)
				std::__throw_length_error(__N(__s));
			const size_t __len = size() + std::max(size(), __n);
			return (__len < size() || __len > max_size()) ? max_size() : __len;
		}

		void _M_range_check(size_t __n) const {
			if (__n >= size())
				std::__throw_out_of_range(
					__N("vector<bool>::_M_range_check: __n  >= size() "));
		}

		iterator _M_copy_aligned(const_iterator __first, const_iterator __last,
			iterator __result) {
			_real_pointer __q = std::copy(__first._M_p, __last._M_p, __result._M_p);
			return std::copy(const_iterator(__last._M_p, 0,__last._M_dirty), __last,
				iterator(__q, 0, &_M_impl._M_dirty));
		}

#if __cplusplus >= 201103L
		void _M_default_initialize(size_t __n) { _M_create_storage(__n); }

		void _M_default_append(size_t __n) { _M_fill_insert(end(), __n, bool()); }
#endif

		void _M_fill_initialize(size_t __n, const bool& __value) {
			if(__value)
				std::fill(_M_impl._M_start._M_p, _M_impl._M_end_addr(), ~0);
		}

		void _M_fill_insert(iterator __position, size_t __n, bool __x) {
			if (__n == 0) return;
			if (capacity() - size() >= __n) {
				std::copy_backward(__position, end(),
					_M_impl._M_finish + ptrdiff_t(__n));
				std::fill(__position, __position + ptrdiff_t(__n), __x);
				_M_impl._M_finish += ptrdiff_t(__n);
			}
			else {
				const size_t __len =
					_M_check_len(__n, "vector<bool>::_M_fill_insert");
				_real_pointer __q = _M_allocate(__len);
				iterator __start(std::__addressof(*__q), 0, &_M_impl._M_dirty);
				iterator __i = _M_copy_aligned(begin(), __position, __start);
				std::fill(__i, __i + ptrdiff_t(__n), __x);
				iterator __finish = std::copy(__position, end(),
					__i + ptrdiff_t(__n));
				_M_deallocate();
				_M_impl._M_end_of_storage = __q + _S_nword(__len);
				_M_impl._M_start = __start;
				_M_impl._M_finish = __finish;
			}
		}

		void _M_fill_assign(size_t __n, bool __x) {
			if (__n > size()) {
				std::fill(_M_impl._M_start._M_p,
					_M_impl._M_end_addr(), __x ? ~0 : 0);
#if __cplusplus >= 201103L
				_M_fill_insert(end()._M_const_cast(), __n - size(), __x);
#else
				_M_fill_insert(end(), __n - size(), __x);
#endif
			}
			else {
				_M_erase_at_end(begin() + __n);
				std::fill(_M_impl._M_start._M_p,
					_M_impl._M_end_addr(), __x ? ~0 : 0);
			}
		}

		void _M_reallocate(size_t __n) {
			_real_pointer __q = _M_allocate(__n);
			iterator __start(std::__addressof(*__q), 0, &_M_impl._M_dirty);
			iterator __finish(_M_copy_aligned(begin(), end(), __start));
			_M_deallocate();
			_M_impl._M_start = __start;
			_M_impl._M_finish = __finish;
			_M_impl._M_end_of_storage = __q + _S_nword(__n);
		}

		void _M_realloc_insert(iterator __position, bool __x) {
			if (_M_impl._M_finish._M_p != _M_impl._M_end_addr()) {
				std::copy_backward(__position, _M_impl._M_finish,
					_M_impl._M_finish + 1);
				*__position = __x;
				++_M_impl._M_finish;
			}
			else {
				const size_t __len =
					_M_check_len(size_t(1), "vector<bool>::_M_realloc_insert");
				_real_pointer __q = _M_allocate(__len);
				iterator __start(std::__addressof(*__q), 0, &_M_impl._M_dirty);
				iterator __i = _M_copy_aligned(begin(), __position, __start);
				*__i++ = __x;
				iterator __finish = std::copy(__position, end(), __i);
				_M_deallocate();
				_M_impl._M_end_of_storage = __q + _S_nword(__len);
				_M_impl._M_start = __start;
				_M_impl._M_finish = __finish;
			}
		}

		template<typename _Integer>
		void _M_initialize_dispatch(_Integer __n, _Integer __x, std::__true_type) {
			_M_create_storage(static_cast<size_t>(__n));
			std::fill(_M_impl._M_start._M_p, _M_impl._M_end_addr(), __x ? ~0 : 0);
		}

		template<typename _InputIterator>
		void _M_initialize_dispatch(_InputIterator __first, _InputIterator __last,
			std::__false_type) {
			_M_range_initialize(__first, __last,
				std::__iterator_category(__first));
		}

		template<typename _InputIterator>
		void _M_range_initialize(_InputIterator __first, _InputIterator __last,
			std::input_iterator_tag) {
			for (; __first != __last; ++__first)
				push_back(*__first);
		}

		template<typename _ForwardIterator>
		void _M_range_initialize(_ForwardIterator __first, _ForwardIterator __last,
			std::forward_iterator_tag) {
			const size_t __n = std::distance(__first, __last);
			_M_create_storage(__n);
			std::copy(__first, __last, _M_impl._M_start);
		}

		template<typename _InputIterator>
		void _M_range_insert(iterator __position, _InputIterator __first,
			_InputIterator __last, std::input_iterator_tag) {
			for (; __first != __last; ++__first) {
				const ptrdiff_t __n = __position - begin();
				if (_M_impl._M_finish._M_p != _M_impl._M_end_addr()
					&& __position == end())
					*_M_impl._M_finish++ = *__first;
				else
					_M_realloc_insert(__position._M_const_cast(), *__first);
				__position = begin() + __n;

				++__position;
			}
		}

		template<typename _ForwardIterator>
		void _M_range_insert(iterator __position, _ForwardIterator __first,
			_ForwardIterator __last, std::forward_iterator_tag) {
			if (__first != __last) {
				size_t __n = std::distance(__first, __last);
				if (capacity() - size() >= __n) {
					std::copy_backward(__position, end(),
						_M_impl._M_finish + ptrdiff_t(__n));
					std::copy(__first, __last, __position);
					_M_impl._M_finish += ptrdiff_t(__n);
				}
				else {
					const size_t __len =
						_M_check_len(__n, "vector<bool>::_M_range_insert");
					_real_pointer __q = _M_allocate(__len);
					iterator __start(std::__addressof(*__q), 0,
								reinterpret_cast<bool*>(cube_getdirty(std::__addressof(*__q))));
					iterator __i = _M_copy_aligned(begin(), __position, __start);
					__i = std::copy(__first, __last, __i);
					iterator __finish = std::copy(__position, end(), __i);
					_M_deallocate();
					_M_impl._M_end_of_storage = __q + _S_nword(__len);
					_M_impl._M_start = __start;
					_M_impl._M_finish = __finish;
				}
			}
		}

		template<typename _InputIterator>
		void _M_assign_aux(_InputIterator __first, _InputIterator __last,
			std::input_iterator_tag) {
			iterator __cur = begin();
			for (; __first != __last && __cur != end(); ++__cur, ++__first)
				*__cur = *__first;
			if (__first == __last)
				_M_erase_at_end(__cur);
			else
				_M_range_insert(end(), __first, __last,
					std::__iterator_category(__first));
		}

		template<typename _ForwardIterator>
		void _M_assign_aux(_ForwardIterator __first, _ForwardIterator __last,
			std::forward_iterator_tag) {
			const size_t __len = std::distance(__first, __last);
			if (__len < size())
				_M_erase_at_end(std::copy(__first, __last, begin()));
			else {
				_ForwardIterator __mid = __first;
				std::advance(__mid, size());
				std::copy(__first, __mid, begin());
				_M_range_insert(end(), __mid, __last,
					std::__iterator_category(__mid));
			}
		}

		void _M_insert_aux(iterator __position, bool __x)
		{ _M_realloc_insert(__position, __x); }

#if __cplusplus >= 201103L
		iterator _M_insert_rval(const_iterator __position, bool __v) {
			_M_realloc_insert(__position._M_const_cast(), __v);
			return __position._M_const_cast();
		}
#endif

		void _M_erase_at_end(iterator __pos) { _M_impl._M_finish = __pos; }

		iterator _M_erase(iterator __position) {
			if (__position + 1 != end())
				std::copy(__position + 1, end(), __position);
			--_M_impl._M_finish;
			return __position;
		}

		iterator _M_erase(iterator __first, iterator __last) {
			if (__first != __last)
				_M_erase_at_end(std::copy(__last, end(), __first));
			return __first;
		}

		void _M_move_assign(_Vector_base&& __x) noexcept {
			_M_impl._M_start = __x._M_impl._M_start;
			_M_impl._M_finish = __x._M_impl._M_finish;
			_M_impl._M_end_of_storage = __x._M_impl._M_end_of_storage;
			__x._M_impl._M_start = iterator();
			__x._M_impl._M_finish = iterator();
			__x._M_impl._M_end_of_storage = nullptr;
			std::swap(_M_impl._M_dirty, __x._M_impl._M_dirty);
		}

		void _M_copy_assign(const _Vector_base& __x) {
			if (&__x != this) {
				const size_t __xlen = __x.size();
				if (__xlen > capacity()) {
					this->_M_deallocate();
					_M_create_storage(__xlen);
				}
				this->_M_impl._M_finish = _M_copy_aligned(__x.begin(),
					__x.end(), begin());
			}
		}
	};
} // namespace inaccel

#endif // VECTOR_BASE_HPP_

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
