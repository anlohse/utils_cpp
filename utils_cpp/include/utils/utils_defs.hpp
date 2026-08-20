/*
 * utils_defs.hpp
 *
 *  Created on: 08/07/2010
 *      Author: alan.lohse
 */

#ifndef UTILS_DEFS_HPP_
#define UTILS_DEFS_HPP_

#include <utils/config.hpp>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cwchar>

// The width-named types below are exactly that: fixed width on every platform.
//
// They used to be spelled with the built-in types, which made them lie. `t_int`
// and `t_dword` were `long int` / `unsigned long int` -- 32 bits on Windows but
// 64 on LP64 Linux -- so anything storing a 32-bit quantity in a t_dword (the
// giantint limb array, the packed ARGB in ui::Color) silently changed shape
// across platforms. Two further typedefs were guarded by `#ifdef __int64` and
// `#ifdef wchar_t`, which test keywords rather than macros and are therefore
// always false; t_wchar consequently resolved to `unsigned short` even where
// wchar_t is 32 bits wide.
typedef std::int8_t			t_small;
typedef std::uint8_t		t_byte;
typedef std::int16_t		t_short;
typedef std::int32_t		t_int;

typedef std::uint16_t		t_word;
typedef std::uint32_t		t_dword;
typedef std::uint32_t		t_uint;

typedef std::int64_t		t_bigint;
typedef std::uint64_t		t_qword;

typedef float				t_float;
typedef double 				t_double;

typedef void*				t_pointer;

typedef wchar_t				t_wchar;
typedef char				t_char;

typedef std::size_t			t_size;

// VALUES FOR t_result
#define R_OK 0
#define R_ERROR_UNKNOWN -1

typedef std::int32_t		t_result;

// allocator configurations
#include <memory>
#define DEFAULT_ALLOCATOR std::allocator

#define MAKE_ENUMERATION(name) private: \
	long value; \
	name() : \
			value(0) { \
	} \
	name(long _value) : \
			value(_value) { \
	} \
public: \
	name(const name& o) : \
			value(o.value) { \
	} \
	~name() { \
	} \
	name& operator = (const name& o) { \
		value = o.value; \
		return *this; \
	} \
	bool operator == (const name& o) const { \
		return value == o.value; \
	} \
	bool operator != (const name& o) const { \
		return value != o.value; \
	} \
	operator long () const { \
		return value; \
	} \
	long ordinal () const { \
		return value; \
	}

#define MAKE_CONSTANT(type,name,init_value) static constexpr type name = init_value
#define MAKE_ENUMERATION2(name) MAKE_ENUMERATION(name) \
	static const name


#define DECLARE_CONSTANT(type,name) static const type name

#define INIT_CONSTANT(type,name,init_value) const type type::name = init_value

#endif /* UTILS_DEFS_HPP_ */
