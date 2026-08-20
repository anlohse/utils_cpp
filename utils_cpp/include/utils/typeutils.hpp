/*
 * typeutils.hpp
 *
 *  Created on: 26/03/2010
 *      Author: alan.lohse
 */

#ifndef TYPEUTILS_HPP_
#define TYPEUTILS_HPP_

#include <cstdlib>
#include <string>
#include <typeinfo>

#if defined(__GNUC__) || defined(__clang__)
#  include <cxxabi.h>
#  define UTILS_HAS_CXA_DEMANGLE 1
#endif

namespace utils {

/**
 * Helpers for inspecting types at runtime.
 *
 * The Itanium ABI name mangling is decoded by abi::__cxa_demangle rather than
 * by hand. The previous hand-rolled parser wrote into fixed 13- and 128-byte
 * stack buffers using lengths taken straight from the mangled string with no
 * clamping, so any sufficiently long or deeply nested type name overflowed the
 * stack. MSVC needs no demangling: typeid().name() is already readable there.
 */
class typeutils {
public:

	/** Decodes a mangled type name, returning it unchanged if undecodable. */
	static std::string demangle(const char* mangled) {
		if (mangled == nullptr)
			return std::string();
#ifdef UTILS_HAS_CXA_DEMANGLE
		int status = 0;
		char* decoded = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
		if (decoded != nullptr) {
			std::string result(status == 0 ? decoded : mangled);
			// __cxa_demangle allocates with malloc, so it must be freed, not deleted.
			std::free(decoded);
			return result;
		}
#endif
		return std::string(mangled);
	}

	/**
	 * @return the name of the most-derived type of *a, or "nullptr" when a is null.
	 *
	 * For a polymorphic A this is the dynamic type; otherwise it is the static
	 * type, exactly as typeid dictates.
	 */
	template<typename A>
	static std::string type_name(const A* a) {
		if (a == nullptr)
			return std::string("nullptr");
		return demangle(typeid(*a).name());
	}

	/** @return the name of the most-derived type of a. */
	template<typename A>
	static std::string type_name(const A& a) {
		return demangle(typeid(a).name());
	}

	/** @return the name of the static type A. */
	template<typename A>
	static std::string type_name() {
		return demangle(typeid(A).name());
	}

	/** @return true when a points to an A. False for a null pointer. */
	template<class A, class B>
	static bool is_instance_of(const B* a) {
		return dynamic_cast<const A*>(a) != nullptr;
	}

	/**
	 * @return true when a refers to an A.
	 *
	 * Takes the address rather than casting the reference: dynamic_cast to a
	 * reference type throws std::bad_cast on failure instead of yielding null,
	 * so the old `dynamic_cast<A>(a) != NULL` form could not work.
	 */
	template<class A, class B>
	static bool is_instance_of(const B& a) {
		return dynamic_cast<const A*>(&a) != nullptr;
	}

};

}

#endif /* TYPEUTILS_HPP_ */
