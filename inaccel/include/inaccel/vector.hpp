#ifndef VECTOR_HPP_
#define VECTOR_HPP_

#include "vector_base.hpp"

namespace inaccel {

	template <class _Tp>
	class vector: protected _Vector_base<_Tp> {
	protected:
		typedef _Vector_base<_Tp>															_Base;
		typedef typename _Base::_Alloc												_Alloc;
		typedef typename _Base::_Alloc_type								_Alloc_type;
		typedef __gnu_cxx::__alloc_traits<_Alloc_type>			_Alloc_traits;

	public:
		typedef _Tp																						value_type;
		typedef typename _Base::pointer												pointer;
		typedef typename _Base::const_pointer									const_pointer;
		typedef typename _Base::reference											reference;
		typedef typename _Base::const_reference								const_reference;
		typedef typename _Base::iterator											iterator;
		typedef typename _Base::const_iterator								const_iterator;
		typedef std::reverse_iterator<iterator>								reverse_iterator;
		typedef std::reverse_iterator<const_iterator>		const_reverse_iterator;
		typedef size_t																				size_type;
		typedef ptrdiff_t																			difference_type;
		typedef _Alloc																				allocator_type;
		typedef typename _Base::fundamental_type							fundamental_type;

		using _Base::get_allocator;
		using _Base::max_size;
		using _Base::size;
		using _Base::capacity;
		using _Base::begin;
		using _Base::end;
		using _Base::operator[];
		using _Base::data;
		using _Base::push_back;
		using _Base::emplace_back;
		using _Base::pop_back;
		using _Base::reserve;
		using _Base::insert;
#if __cplusplus >= 201103L
		using _Base::shrink_to_fit;
#endif

	protected:
		using _Base::_M_impl;
		using _Base::_M_range_check;
		using _Base::_M_default_initialize;
		using _Base::_M_default_append;
		using _Base::_M_fill_initialize;
		using _Base::_M_fill_insert;
		using _Base::_M_fill_assign;
		using _Base::_M_initialize_dispatch;
		using _Base::_M_range_initialize;
		using _Base::_M_range_insert;
		using _Base::_M_assign_aux;
		using _Base::_M_insert_rval;
		using _Base::_M_erase_at_end;
		using _Base::_M_erase;
		using _Base::_M_move_assign;
		using _Base::_M_copy_assign;

	public:

		/**
		 *	@brief	Creates a %vector with no elements.
		 */
		vector()
#if __cplusplus >= 201103L
		noexcept(std::is_nothrow_default_constructible<_Alloc>::value)
#endif
		: _Base() { set_dirty(); }

#if __cplusplus >= 201103L
		/**
		 *	@brief	Creates a %vector with default constructed elements.
		 *	@param	__n	The number of elements to initially create.
		 *
		 *	This constructor fills the %vector with @a __n default
		 *	constructed elements.
		 */
		explicit vector(size_type __n) : _Base(__n) {
			_M_default_initialize(__n);
			set_dirty();
		}

		/**
		 *	@brief	Creates a %vector with copies of an exemplar element.
		 *	@param	__n	The number of elements to initially create.
		 *	@param	__value	An element to copy.
		 *
		 *	This constructor fills the %vector with @a __n copies of @a __value.
		 */
		vector(size_type __n, const value_type& __value) : _Base(__n)
		{
			_M_fill_initialize(__n, __value);
			set_dirty();
		}
#else
		/**
		 *	@brief	Creates a %vector with copies of an exemplar element.
		 *	@param	__n	The number of elements to initially create.
		 *	@param	__value	An element to copy.
		 *
		 *	This constructor fills the %vector with @a __n copies of @a __value.
		 */
		explicit vector(size_type __n, const value_type& __value = value_type())
		: _Base(__n) {
			_M_fill_initialize(__n, __value);
			set_dirty();
		}
#endif

		/**
		 *	@brief	%Vector copy constructor.
		 *	@param	__x	A %vector of identical element types.
		 *
		 *	All the elements of @a __x are copied, but any unused capacity in
		 *	@a __x	will not be copied
		 *	(i.e. capacity() == size() in the new %vector).
		 */
		vector(const vector& __x) : _Base(__x.size()) {
			_M_range_initialize(__x.begin(), __x.end(),
													std::forward_iterator_tag());
			set_dirty();
		}

#if __cplusplus >= 201103L
		/**
		 *	@brief	%Vector move constructor.
		 *	@param	__x	A %vector of identical element types.
		 *
		 *	The newly-created %vector contains the exact contents of @a __x.
		 *	The contents of @a __x are a valid, but unspecified %vector.
		 */
		vector(vector&& __x) noexcept : _Base(std::move(__x)) { }

		/**
		 *	@brief	Builds a %vector from an initializer list.
		 *	@param	__l	An initializer_list.
		 *
		 *	Create a %vector consisting of copies of the elements in the
		 *	initializer_list @a __l.
		 *
		 *	This will call the element type's copy constructor N times
		 *	(where N is @a __l.size()) and do no memory reallocation.
		 */
		vector(std::initializer_list<value_type> __l) : _Base() {
			_M_range_initialize(__l.begin(), __l.end(),
													std::random_access_iterator_tag());
			set_dirty();
		}
#endif

		/**
		 *	@brief	Builds a %vector from a range.
		 *	@param	__first	An input iterator.
		 *	@param	__last	An input iterator.
		 *
		 *	Create a %vector consisting of copies of the elements from
		 *	[first,last).
		 *
		 *	If the iterators are forward, bidirectional, or
		 *	random-access, then this will call the elements' copy
		 *	constructor N times (where N is distance(first,last)) and do
		 *	no memory reallocation.	But if only input iterators are
		 *	used, then this will do at most 2N calls to the copy
		 *	constructor, and logN memory reallocations.
		 */
#if __cplusplus >= 201103L
		template<typename _InputIterator,
						 typename = std::_RequireInputIter<_InputIterator>>
		vector(_InputIterator __first, _InputIterator __last) : _Base() {
				_M_initialize_dispatch(__first, __last, std::__false_type());
				set_dirty();
			}
#else
		template<typename _InputIterator>
		vector(_InputIterator __first, _InputIterator __last) : _Base() {
			// Check whether it's an integral type.	If so, it's not an iterator.
			typedef typename std::__is_integer<_InputIterator>::__type _Integral;
			_M_initialize_dispatch(__first, __last, _Integral());
			set_dirty();
		}
#endif

		/**
		 *	The dtor only erases the elements, and note that if the
		 *	elements themselves are pointers, the pointed-to memory is
		 *	not touched in any way.	Managing the pointer is the user's
		 *	responsibility.
		 */
		~vector() _GLIBCXX_NOEXCEPT { }

		bool is_dirty() {
			return _M_impl._M_dirty;
		}

		bool is_dirty() const {
			return _M_impl._M_dirty;
		}

		vector& set_dirty() {
			if(!_M_impl._M_dirty)
				_M_impl._M_dirty = true;
			return *this;
		}

		const vector& set_dirty() const {
			if(!_M_impl._M_dirty)
				*(bool*)&_M_impl._M_dirty = true;
			return *this;
		}

		vector& unset_dirty() {
			if(_M_impl._M_dirty)
				_M_impl._M_dirty = false;
			return *this;
		}

		const vector& unset_dirty() const {
			if(_M_impl._M_dirty)
				*(bool*)&_M_impl._M_dirty = false;
			return *this;
		}

		bool is_fundamental() {
			return fundamental_type::value;
		}

		/**
		 *	@brief	%Vector assignment operator.
		 *	@param	__x	A %vector of identical element types.
		 *
		 *	All the elements of @a __x are copied, but any unused capacity in
		 *	@a __x will not be copied.
		 */
		vector& operator=(const vector& __x) {
			_M_copy_assign(__x);
			set_dirty();
			return *this;
		}

#if __cplusplus >= 201103L
		/**
		 *	@brief	%Vector move assignment operator.
		 *	@param	__x	A %vector of identical element types.
		 *
		 *	The contents of @a __x are moved into this %vector (without copying,
		 *	if the allocators permit it).
		 *	Afterwards @a __x is a valid, but unspecified %vector.
		 */
		vector& operator=(vector&& __x) noexcept(_Alloc_traits::_S_nothrow_move())
		{
			_M_move_assign(std::move(__x));
			return *this;
		}

		/**
		 *	@brief	%Vector list assignment operator.
		 *	@param	__l	An initializer_list.
		 *
		 *	This function fills a %vector with copies of the elements in the
		 *	initializer list @a __l.
		 *
		 *	Note that the assignment completely changes the %vector and
		 *	that the resulting %vector's size is the same as the number
		 *	of elements assigned.
		 */
		vector& operator=(std::initializer_list<value_type> __l) {
			_M_assign_aux(__l.begin(), __l.end(),
				std::random_access_iterator_tag());
			set_dirty();
			return *this;
		}
#endif

		/**
		 *	@brief	Assigns a given value to a %vector.
		 *	@param	__n	Number of elements to be assigned.
		 *	@param	__val	Value to be assigned.
		 *
		 *	This function fills a %vector with @a __n copies of the given
		 *	value.	Note that the assignment completely changes the
		 *	%vector and that the resulting %vector's size is the same as
		 *	the number of elements assigned.
		 */
		void assign(size_type __n, const value_type& __val) {
			_M_fill_assign(__n, __val);
			set_dirty();
		}

		/**
		 *	@brief	Assigns a range to a %vector.
		 *	@param	__first	An input iterator.
		 *	@param	__last	 An input iterator.
		 *
		 *	This function fills a %vector with copies of the elements in the
		 *	range [__first,__last).
		 *
		 *	Note that the assignment completely changes the %vector and
		 *	that the resulting %vector's size is the same as the number
		 *	of elements assigned.
		 */
#if __cplusplus >= 201103L
		template<typename _InputIterator,
						 typename = std::_RequireInputIter<_InputIterator>>
		void assign(_InputIterator __first, _InputIterator __last) {
			_M_assign_dispatch(__first, __last, std::__false_type());
			set_dirty();
		}
#else
		template<typename _InputIterator>
		void assign(_InputIterator __first, _InputIterator __last) {
			// Check whether it's an integral type.	If so, it's not an iterator.
			typedef typename std::__is_integer<_InputIterator>::__type _Integral;
			_M_assign_dispatch(__first, __last, _Integral());
			set_dirty();
		}
#endif

#if __cplusplus >= 201103L
		/**
		 *	@brief	Assigns an initializer list to a %vector.
		 *	@param	__l	An initializer_list.
		 *
		 *	This function fills a %vector with copies of the elements in the
		 *	initializer list @a __l.
		 *
		 *	Note that the assignment completely changes the %vector and
		 *	that the resulting %vector's size is the same as the number
		 *	of elements assigned.
		 */
		void assign(std::initializer_list<value_type> __l) {
			_M_assign_aux(__l.begin(), __l.end(), std::random_access_iterator_tag());
			set_dirty();
		}
#endif

		/**
		 *	Returns a read/write reverse iterator that points to the
		 *	last element in the %vector.	Iteration is done in reverse
		 *	element order.
		 */
		reverse_iterator rbegin() _GLIBCXX_NOEXCEPT
		{ return reverse_iterator(end()); }

		/**
		 *	Returns a read-only (constant) reverse iterator that points
		 *	to the last element in the %vector.	Iteration is done in
		 *	reverse element order.
		 */
		const_reverse_iterator rbegin() const _GLIBCXX_NOEXCEPT
		{ return const_reverse_iterator(end()); }

		/**
		 *	Returns a read/write reverse iterator that points to one
		 *	before the first element in the %vector.	Iteration is done
		 *	in reverse element order.
		 */
		reverse_iterator rend() _GLIBCXX_NOEXCEPT
		{ return reverse_iterator(begin()); }

		/**
		 *	Returns a read-only (constant) reverse iterator that points
		 *	to one before the first element in the %vector.	Iteration
		 *	is done in reverse element order.
		 */
		const_reverse_iterator rend() const _GLIBCXX_NOEXCEPT
		{ return const_reverse_iterator(begin()); }

#if __cplusplus >= 201103L
		/**
		 *	Returns a read-only (constant) iterator that points to the
		 *	first element in the %vector.	Iteration is done in ordinary
		 *	element order.
		 */
		const_iterator cbegin() const noexcept
		{ return const_iterator(begin()); }

		/**
		 *	Returns a read-only (constant) iterator that points one past
		 *	the last element in the %vector.	Iteration is done in
		 *	ordinary element order.
		 */
		const_iterator cend() const noexcept
		{ return const_iterator(end()); }

		/**
		 *	Returns a read-only (constant) reverse iterator that points
		 *	to the last element in the %vector.	Iteration is done in
		 *	reverse element order.
		 */
		const_reverse_iterator crbegin() const noexcept
		{ return const_reverse_iterator(end()); }

		/**
		 *	Returns a read-only (constant) reverse iterator that points
		 *	to one before the first element in the %vector.	Iteration
		 *	is done in reverse element order.
		 */
		const_reverse_iterator crend() const noexcept
		{ return const_reverse_iterator(begin()); }
#endif

#if __cplusplus >= 201103L
		/**
		 *	@brief	Resizes the %vector to the specified number of elements.
		 *	@param	__new_size	Number of elements the %vector should contain.
		 *
		 *	This function will %resize the %vector to the specified
		 *	number of elements.	If the number is smaller than the
		 *	%vector's current size the %vector is truncated, otherwise
		 *	default constructed elements are appended.
		 */
		void resize(size_type __new_size) {
			if (__new_size > size())
				_M_default_append(__new_size - size());
			else if (__new_size < size())
				_M_erase_at_end(_M_impl._M_start + __new_size);
			set_dirty();
		}

		/**
		 *	@brief	Resizes the %vector to the specified number of elements.
		 *	@param	__new_size	Number of elements the %vector should contain.
		 *	@param	__x	Data with which new elements should be populated.
		 *
		 *	This function will %resize the %vector to the specified
		 *	number of elements.	If the number is smaller than the
		 *	%vector's current size the %vector is truncated, otherwise
		 *	the %vector is extended and new elements are populated with
		 *	given data.
		 */
		void resize(size_type __new_size, const value_type& __x) {
			if (__new_size > size())
				_M_fill_insert(end(), __new_size - size(), __x);
			else if (__new_size < size())
				_M_erase_at_end(_M_impl._M_start + __new_size);
			set_dirty();
		}
#else
		/**
		 *	@brief	Resizes the %vector to the specified number of elements.
		 *	@param	__new_size	Number of elements the %vector should contain.
		 *	@param	__x	Data with which new elements should be populated.
		 *
		 *	This function will %resize the %vector to the specified
		 *	number of elements.	If the number is smaller than the
		 *	%vector's current size the %vector is truncated, otherwise
		 *	the %vector is extended and new elements are populated with
		 *	given data.
		 */
		void resize(size_type __new_size, value_type __x = value_type()) {
			if (__new_size > size())
				_M_fill_insert(end(), __new_size - size(), __x);
			else if (__new_size < size())
				_M_erase_at_end(_M_impl._M_start + __new_size);
			set_dirty();
		}
#endif

		/**
		 *	Returns true if the %vector is empty.	(Thus begin() would
		 *	equal end().)
		 */
		bool empty() const _GLIBCXX_NOEXCEPT
		{ return begin() == end(); }

		/**
		 *	@brief	Provides access to the data contained in the %vector.
		 *	@param __n The index of the element for which data should be
		 *	accessed.
		 *	@return	Read/write reference to data.
		 *	@throw	std::out_of_range	If @a __n is an invalid index.
		 *
		 *	This function provides for safer data access.	The parameter
		 *	is first checked that it is in the range of the vector.	The
		 *	function throws out_of_range if the check fails.
		 */
		reference at(size_type __n) {
			_M_range_check(__n);
			return (*this)[__n];
		}

		/**
		 *	@brief	Provides access to the data contained in the %vector.
		 *	@param __n The index of the element for which data should be
		 *	accessed.
		 *	@return	Read-only (constant) reference to data.
		 *	@throw	std::out_of_range	If @a __n is an invalid index.
		 *
		 *	This function provides for safer data access.	The parameter
		 *	is first checked that it is in the range of the vector.	The
		 *	function throws out_of_range if the check fails.
		 */
		const_reference at(size_type __n) const {
			_M_range_check(__n);
			return (*this)[__n];
		}

		/**
		 *	Returns a read/write reference to the data at the first
		 *	element of the %vector.
		 */
		reference front() _GLIBCXX_NOEXCEPT {
			__glibcxx_requires_nonempty();
			return *begin();
		}

		/**
		 *	Returns a read-only (constant) reference to the data at the first
		 *	element of the %vector.
		 */
		const_reference front() const _GLIBCXX_NOEXCEPT {
			__glibcxx_requires_nonempty();
			return *begin();
		}

		/**
		 *	Returns a read/write reference to the data at the last
		 *	element of the %vector.
		 */
		reference back() _GLIBCXX_NOEXCEPT {
			__glibcxx_requires_nonempty();
			return *(end() - 1);
		}

		/**
		 *	Returns a read-only (constant) reference to the data at the
		 *	last element of the %vector.
		 */
		const_reference back() const _GLIBCXX_NOEXCEPT {
			__glibcxx_requires_nonempty();
			return *(end() - 1);
		}

#if __cplusplus >= 201103L
		/**
		 *	@brief	Inserts an object in %vector before specified iterator.
		 *	@param	__position	A const_iterator into the %vector.
		 *	@param	__args	Arguments.
		 *	@return	An iterator that points to the inserted data.
		 *
		 *	This function will insert an object of type T constructed
		 *	with T(std::forward<Args>(args)...) before the specified location.
		 *	Note that this kind of operation could be expensive for a %vector
		 *	and if it is frequently used the user should consider using
		 *	std::list.
		 */
		template<typename... _Args>
		iterator emplace(const_iterator __position, _Args&&... __args)
		{ return insert(__position, _Tp(__args...)); }
#endif

#if __cplusplus >= 201103L
		/**
		 *	@brief	Inserts given rvalue into %vector before specified iterator.
		 *	@param	__position	A const_iterator into the %vector.
		 *	@param	__x	Data to be inserted.
		 *	@return	An iterator that points to the inserted data.
		 *
		 *	This function will insert a copy of the given rvalue before
		 *	the specified location.	Note that this kind of operation
		 *	could be expensive for a %vector and if it is frequently
		 *	used the user should consider using std::list.
		 */
		iterator insert(const_iterator __position, value_type&& __x) {
			set_dirty();
			return _M_insert_rval(__position, std::move(__x));
		}

		/**
		 *	@brief	Inserts an initializer_list into the %vector.
		 *	@param	__position	An iterator into the %vector.
		 *	@param	__l	An initializer_list.
		 *
		 *	This function will insert copies of the data in the
		 *	initializer_list @a l into the %vector before the location
		 *	specified by @a position.
		 *
		 *	Note that this kind of operation could be expensive for a
		 *	%vector and if it is frequently used the user should
		 *	consider using std::list.
		 */
		iterator insert(const_iterator __position, std::initializer_list<value_type> __l)
		{ return insert(__position, __l.begin(), __l.end()); }
#endif

#if __cplusplus >= 201103L
		/**
		 *	@brief	Inserts a number of copies of given data into the %vector.
		 *	@param	__position	A const_iterator into the %vector.
		 *	@param	__n	Number of elements to be inserted.
		 *	@param	__x	Data to be inserted.
		 *	@return	An iterator that points to the inserted data.
		 *
		 *	This function will insert a specified number of copies of
		 *	the given data before the location specified by @a position.
		 *
		 *	Note that this kind of operation could be expensive for a
		 *	%vector and if it is frequently used the user should
		 *	consider using std::list.
		 */
		iterator insert(const_iterator __position, size_type __n, const value_type& __x)
		{
			difference_type __offset = __position - cbegin();
			_M_fill_insert(begin() + __offset, __n, __x);
			set_dirty();
			return begin() + __offset;
		}
#else
		/**
		 *	@brief	Inserts a number of copies of given data into the %vector.
		 *	@param	__position	An iterator into the %vector.
		 *	@param	__n	Number of elements to be inserted.
		 *	@param	__x	Data to be inserted.
		 *
		 *	This function will insert a specified number of copies of
		 *	the given data before the location specified by @a position.
		 *
		 *	Note that this kind of operation could be expensive for a
		 *	%vector and if it is frequently used the user should
		 *	consider using std::list.
		 */
		void insert(iterator __position, size_type __n, const value_type& __x) {
			_M_fill_insert(__position, __n, __x);
			set_dirty();
		}
#endif

#if __cplusplus >= 201103L
		/**
		 *	@brief	Inserts a range into the %vector.
		 *	@param	__position	A const_iterator into the %vector.
		 *	@param	__first	An input iterator.
		 *	@param	__last	 An input iterator.
		 *	@return	An iterator that points to the inserted data.
		 *
		 *	This function will insert copies of the data in the range
		 *	[__first,__last) into the %vector before the location specified
		 *	by @a pos.
		 *
		 *	Note that this kind of operation could be expensive for a
		 *	%vector and if it is frequently used the user should
		 *	consider using std::list.
		 */
		template<typename _InputIterator,
						 typename = std::_RequireInputIter<_InputIterator>>
		iterator insert(const_iterator __position, _InputIterator __first,
										_InputIterator __last)
		{
			difference_type __offset = __position - cbegin();
			_M_insert_dispatch(begin() + __offset,
						 __first, __last, std::__false_type());
			set_dirty();
			return begin() + __offset;
		}
#else
		/**
		 *	@brief	Inserts a range into the %vector.
		 *	@param	__position	An iterator into the %vector.
		 *	@param	__first	An input iterator.
		 *	@param	__last	 An input iterator.
		 *
		 *	This function will insert copies of the data in the range
		 *	[__first,__last) into the %vector before the location specified
		 *	by @a pos.
		 *
		 *	Note that this kind of operation could be expensive for a
		 *	%vector and if it is frequently used the user should
		 *	consider using std::list.
		 */
		template<typename _InputIterator>
		void insert(iterator __position, _InputIterator __first,
								_InputIterator __last)
		{
			// Check whether it's an integral type.	If so, it's not an iterator.
			typedef typename std::__is_integer<_InputIterator>::__type _Integral;
			_M_insert_dispatch(__position, __first, __last, _Integral());
			set_dirty();
		}
#endif

		/**
		 *	@brief	Remove element at given position.
		 *	@param	__position	Iterator pointing to element to be erased.
		 *	@return	An iterator pointing to the next element (or end()).
		 *
		 *	This function will erase the element at the given position and thus
		 *	shorten the %vector by one.
		 *
		 *	Note This operation could be expensive and if it is
		 *	frequently used the user should consider using std::list.
		 *	The user is also cautioned that this function only erases
		 *	the element, and that if the element is itself a pointer,
		 *	the pointed-to memory is not touched in any way.	Managing
		 *	the pointer is the user's responsibility.
		 */
		iterator
#if __cplusplus >= 201103L
		erase(const_iterator __position)
#else
		erase(iterator __position)
#endif
		{
			set_dirty();
			return _M_erase(__position._M_const_cast());
		}

		/**
		 *	@brief	Remove a range of elements.
		 *	@param	__first	Iterator pointing to the first element to be erased.
		 *	@param	__last	Iterator pointing to one past the last element to be
		 *									erased.
		 *	@return	An iterator pointing to the element pointed to by @a __last
		 *					 prior to erasing (or end()).
		 *
		 *	This function will erase the elements in the range
		 *	[__first,__last) and shorten the %vector accordingly.
		 *
		 *	Note This operation could be expensive and if it is
		 *	frequently used the user should consider using std::list.
		 *	The user is also cautioned that this function only erases
		 *	the elements, and that if the elements themselves are
		 *	pointers, the pointed-to memory is not touched in any way.
		 *	Managing the pointer is the user's responsibility.
		 */
		iterator
#if __cplusplus >= 201103L
		erase(const_iterator __first, const_iterator __last)
#else
		erase(iterator __first, iterator __last)
#endif
		{
			set_dirty();
			return _M_erase(__first._M_const_cast(), __last._M_const_cast());
		}

		/**
		 *	@brief	Swaps data with another %vector.
		 *	@param	__x	A %vector of the same element types.
		 *
		 *	This exchanges the elements between two vectors in constant time.
		 *	(Three pointers, so it should be quite fast.)
		 *	Note that the global std::swap() function is specialized such that
		 *	std::swap(v1,v2) will feed to this function.
		 */
		void swap(vector& __x) _GLIBCXX_NOEXCEPT {
			std::swap(_M_impl._M_start, __x._M_impl._M_start);
			std::swap(_M_impl._M_finish, __x._M_impl._M_finish);
			std::swap(_M_impl._M_end_of_storage, __x._M_impl._M_end_of_storage);
			std::swap(_M_impl._M_dirty, __x._M_impl._M_dirty);
		}

// 		/**
// 		 *	Erases all the elements.	Note that this function only erases the
// 		 *	elements, and that if the elements themselves are pointers, the
// 		 *	pointed-to memory is not touched in any way.	Managing the pointer is
// 		 *	the user's responsibility.
// 		 */
		void clear() _GLIBCXX_NOEXCEPT {
			_M_erase_at_end(begin());
			set_dirty();
		}

	protected:

		template<typename _Integer>
		void _M_assign_dispatch(_Integer __n, _Integer __val, std::__true_type)
		{ _M_fill_assign(__n, __val); }

		template<typename _InputIterator>
		void _M_assign_dispatch(_InputIterator __first, _InputIterator __last,
					 std::__false_type)
		{ _M_assign_aux(__first, __last, std::__iterator_category(__first)); }

		template<typename _Integer>
		void _M_insert_dispatch(iterator __pos, _Integer __n, _Integer __val,
					 std::__true_type)
		{ _M_fill_insert(__pos, __n, __val); }

		template<typename _InputIterator>
		void _M_insert_dispatch(iterator __pos, _InputIterator __first,
					 _InputIterator __last, std::__false_type)
		{
			_M_range_insert(__pos, __first, __last,
					std::__iterator_category(__first));
		}
	};

} // namespace inaccel

#endif // VECTOR_HPP_
