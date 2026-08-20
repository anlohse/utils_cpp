/*
 * ui_allocator.cpp
 *
 *  Created on: 05/08/2014
 *      Author: Alan
 */

#include <utils/ui/ui_defs.hpp>
#include <utils/ui/ui_allocator.hpp>
#include <utils/ui/image.hpp>
#include <cstdlib>

namespace utils {
namespace ui {

struct DefaultUIAllocator : public UIAllocator {
	DefaultUIAllocator() { }
	virtual ~DefaultUIAllocator() { }
	virtual void* malloc(size_t size)  {
		return ::malloc(size);
	}
	virtual void free(void* mem, size_t size) {
		return ::free(mem);
	}
};

/**
 * The installed allocator.
 *
 * This used to be a namespace-scope `UIAllocator* = new DefaultUIAllocator()`,
 * which is dynamically initialised. UIObject::operator new reads it, and any
 * UIObject constructed by another translation unit's static initialiser could
 * therefore run before this pointer was assigned -- a static initialisation
 * order fiasco that dereferenced null. Wrapping it in a function makes the
 * default allocator a function-local static, initialised on first use.
 */
static UIAllocator*& current_allocator() {
	static DefaultUIAllocator default_allocator;
	static UIAllocator* allocator = &default_allocator;
	return allocator;
}

UIAllocator* getUIAllocator() {
	return current_allocator();
}
UIAllocator* setUIAllocator(UIAllocator* new_alloc) {
	UIAllocator*& slot = current_allocator();
	UIAllocator* old = slot;
	// Refuse null so the invariant "there is always an allocator" holds.
	if (new_alloc != NULL)
		slot = new_alloc;
	return old;
}

void* UIObject::operator new(size_t nbytes) {
	return current_allocator()->malloc(nbytes);
}
void UIObject::operator delete(void* p, size_t nbytes) {
	current_allocator()->free(p,nbytes);
}

}
}

