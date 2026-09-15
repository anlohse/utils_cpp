/*
 * ui_allocator.hpp
 *
 *  Created on: 04/08/2014
 *      Author: Alan
 */

#ifndef UI_ALLOCATOR_HPP_
#define UI_ALLOCATOR_HPP_

namespace utils {
namespace ui {

struct UIAllocator {
	virtual ~UIAllocator() { }
	virtual void* malloc(size_t size) = 0;
	virtual void free(void* mem, size_t size) = 0;
};

UIAllocator* getUIAllocator();
UIAllocator* setUIAllocator(UIAllocator* new_alloc);

class ui_allocator_base {
public:
	virtual ~ui_allocator_base() { }

	virtual void* allocate_objs(size_t count) = 0;

	virtual void deallocate_objs(void* mem, size_t count) = 0;

};

template<typename _Tp>
class ui_allocator : public ui_allocator_base {
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef _Tp* pointer;
	typedef const _Tp* const_pointer;
	typedef _Tp& reference;
	typedef const _Tp& const_reference;
	typedef _Tp value_type;

	template<typename _Tp1>
	struct rebind {
		typedef ui_allocator<_Tp1> other;
	};

	ui_allocator() throw () {
	}

	ui_allocator(const ui_allocator& other) throw () {
	}

	template<typename _Tp1>
	ui_allocator(const ui_allocator<_Tp1>&) throw () {
	}

	~ui_allocator() throw () {
	}

	pointer address(reference __x) const {
		return &__x;
	}

	const_pointer address(const_reference __x) const {
		return &__x;
	}

	pointer allocate(size_type __n, const void* = 0) {
		void* mem = allocate_objs(__n);
		return static_cast<_Tp*> (mem);
	}

	void deallocate(pointer __p, size_type __n) {
		deallocate_objs((void*)__p, __n);
	}

	size_type max_size() const throw () {
		return size_t(-1) / sizeof(_Tp);
	}

	void construct(pointer __p, const _Tp& __val) {
		::new (__p) _Tp(__val);
	}

	void destroy(pointer __p) {
		__p->~_Tp();
	}

	pointer allocate(const _Tp& __val) {
		pointer __p = allocate(1);
		construct(__p,__val);
		return __p;
	}

	virtual void* allocate_objs(size_t count) {
		return getUIAllocator()->malloc(count * sizeof(_Tp));
	}

	virtual void deallocate_objs(void* mem, size_t count) {
		getUIAllocator()->free(mem,count * sizeof(_Tp));
	}

};

template<typename _Tp>
inline bool operator==(const ui_allocator<_Tp>&,
		const ui_allocator<_Tp>&) {
	return true;
}

template<typename _Tp>
inline bool operator!=(const ui_allocator<_Tp>&,
		const ui_allocator<_Tp>&) {
	return false;
}

// SAFE_DELETE lived here. It has no users left: every owning member in the UI
// now holds a Ref<T> (utils/ui/ref.hpp), which releases on destruction and on
// reassignment. The macro made ownership a convention upheld by hand at every
// call site, and the bugs it produced -- a Stroke leaked on every paint, a
// use-after-free on the second setFillColor, two backends disagreeing about
// who owned a createStroke() result -- are exactly what a handle prevents.


} // ui
} // utils

#endif /* UI_ALLOCATOR_HPP_ */
