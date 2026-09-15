/*
 * ref.hpp
 *
 * An owning handle for the intrusively reference-counted UI objects.
 */

#ifndef UI_REF_HPP_
#define UI_REF_HPP_

#include <concepts>
#include <cstddef>
#include <utility>

namespace utils {
namespace ui {

/**
 * Anything carrying its own reference count, which is every UIObject.
 *
 * Stated as a concept rather than a UIObject base-class constraint so Ref
 * stays usable with any intrusively counted type and the error message names
 * the missing operation rather than a failed conversion.
 */
template <typename T>
concept RefCounted = requires(T* p) {
	{ p->add_reference() };
	{ p->rem_reference() };
	{ p->get_references() } -> std::convertible_to<int>;
};

/**
 * Owns one reference to an intrusively counted object, releasing it on
 * destruction and destroying the object when the last reference goes.
 *
 * This replaces hand-written add_reference/rem_reference pairs and the
 * SAFE_DELETE macro. Those made ownership a convention upheld at every call
 * site, where a single omission was a leak or a use-after-free rather than a
 * compile error -- and in practice produced both: a Stroke leaked on every
 * paint, a second setFillColor dereferenced freed memory, and the two
 * Graphics backends disagreed about who owned what createStroke() returned.
 *
 * The count is intrusive, so wrapping the same raw pointer in two separate
 * Refs is safe: each holds its own reference and the object dies once, when
 * the second one goes. That is not true of shared_ptr, which is why this
 * exists rather than a standard handle.
 *
 * Construction retains. A freshly created object starts at zero references,
 * so `Ref<Foo> r(new Foo)` leaves it at one and `r` owns it.
 */
template <RefCounted T>
class Ref {
private:
	T* _ptr;

	void retain() const {
		if (_ptr != nullptr)
			_ptr->add_reference();
	}

	void release() {
		if (_ptr != nullptr && _ptr->rem_reference()->get_references() < 1)
			delete _ptr;
		_ptr = nullptr;
	}

public:
	typedef T element_type;

	// Deliberately no std::nullptr_t overloads: NULL is the literal 0, which
	// converts equally well to T* and to std::nullptr_t, so having both made
	// Ref<T> r(NULL) ambiguous everywhere in this codebase. T* alone accepts
	// NULL, 0 and nullptr.
	Ref() noexcept : _ptr(nullptr) { }

	/** Takes a reference to p; p keeps whatever count it already had. */
	explicit Ref(T* p) : _ptr(p) { retain(); }

	Ref(const Ref& other) : _ptr(other._ptr) { retain(); }

	Ref(Ref&& other) noexcept : _ptr(other._ptr) { other._ptr = nullptr; }

	/** Converting copy, so a Ref<Derived> can initialise a Ref<Base>. */
	template <RefCounted U>
		requires std::convertible_to<U*, T*>
	Ref(const Ref<U>& other) : _ptr(other.get()) { retain(); }

	~Ref() { release(); }

	Ref& operator = (const Ref& other) {
		// Retain before releasing: self-assignment, and assigning an alias of
		// the same object, must not drop the last reference mid-way.
		T* incoming = other._ptr;
		if (incoming != nullptr)
			incoming->add_reference();
		release();
		_ptr = incoming;
		return *this;
	}

	Ref& operator = (Ref&& other) noexcept {
		if (this != &other) {
			release();
			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		return *this;
	}

	Ref& operator = (T* p) {
		if (p != nullptr)
			p->add_reference();
		release();
		_ptr = p;
		return *this;
	}

	T* get() const noexcept { return _ptr; }
	T* operator -> () const noexcept { return _ptr; }
	T& operator * () const noexcept { return *_ptr; }

	explicit operator bool() const noexcept { return _ptr != nullptr; }

	/** Implicit decay to the raw pointer, for the many APIs still taking one. */
	operator T* () const noexcept { return _ptr; }

	void reset(T* p = nullptr) { *this = p; }

	/** Hands the pointer back without releasing; the caller owns the reference. */
	T* detach() noexcept {
		T* p = _ptr;
		_ptr = nullptr;
		return p;
	}

	void swap(Ref& other) noexcept {
		T* tmp = _ptr;
		_ptr = other._ptr;
		other._ptr = tmp;
	}

	int use_count() const noexcept { return _ptr != nullptr ? _ptr->get_references() : 0; }
};

template <RefCounted T, RefCounted U>
inline bool operator == (const Ref<T>& a, const Ref<U>& b) noexcept {
	return a.get() == b.get();
}

template <RefCounted T>
inline bool operator == (const Ref<T>& a, std::nullptr_t) noexcept {
	return a.get() == nullptr;
}

template <RefCounted T>
inline void swap(Ref<T>& a, Ref<T>& b) noexcept {
	a.swap(b);
}

/**
 * Wraps a newly created object.
 *
 * `adopt(new Foo(...))` reads better than `Ref<Foo>(new Foo(...))` and keeps
 * the type from being written twice.
 */
template <RefCounted T>
inline Ref<T> adopt(T* p) {
	return Ref<T>(p);
}

} // ui
} // utils

#endif /* UI_REF_HPP_ */
