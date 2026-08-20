/*
 * xml_parser_util.h
 *
 *  Created on: 28 de jun de 2018
 *      Author: alan.lohse
 */

#ifndef XML_PARSER_UTIL_H_
#define XML_PARSER_UTIL_H_

#include <utils/xml/xml_parser.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <regex>
#include <iostream>
#include <sstream>
#include <memory>

#define XC_EQUALS 0
#define XC_RETURN 1
#define XC_LT 2
#define XC_GT 3
#define XC_SLASH 4
#define XC_QUOTE 5
#define XC_COLON 6
#define XC_AMP 7
#define XC_SEMICOLON 8
#define XC_BEGIN_CDATA 9
#define XC_END_CDATA 10
#define XC_BEGIN_DOCTYPE 11
#define XC_END_DOCTYPE 12
#define XC_BEGIN_COMMENT 13
#define XC_END_COMMENT 14
#define XC_BEGIN_XML_DECL 15
#define XC_END_XML_DECL 16
#define XC_VERSION 17
#define XC_ENCODING 18
#define XC_STANDALONE 19
#define XC_ELEMENT 20
#define XC_PCDATA 21
#define XC_CDATA 22
#define XC_EMPTY 23
#define XC_ATTLIST 24
#define XC_ENTITY 25
#define XC_REQUIRED 26
#define XC_IMPLIED 27
#define XC_OPEN_PAR 28
#define XC_CLOSE_PAR 29
#define XC_PIPE 30
#define XC_PLUS 31
#define XC_QUESTION 32
#define XC_FIXED 33
#define XC_ASTERIX 34
#define XC_ANY 35
#define XC_OPEN_BRAC 36
#define XC_CLOSE_BRAC 37
#define XC_CLOSE_LEAF 38

extern std::regex IS_TAG;
extern std::wregex IS_TAGW;
extern std::regex CLOSE_TAG;
extern std::wregex CLOSE_TAGW;
extern std::regex ATTRS;
extern std::wregex ATTRSW;
extern std::regex ENT_NUM;
extern std::regex ENT_HEX;
extern std::wregex ENT_NUMW;
extern std::wregex ENT_HEXW;


extern const char* _char_xml_constants[];

extern const wchar_t* _wchar_xml_constants[];

using namespace utils::xml;

template <typename _CharT, class _Alloc>
class string_builder {
public:
	typedef _CharT char_type;
	// std::allocator<T>::rebind was removed in C++20.
	typedef typename std::allocator_traits<_Alloc>::template rebind_alloc<char_type> allocator_type;
	typedef size_t size_type;
private:
	char_type* _M_begin;
	char_type* _M_cur;
	char_type* _M_end;
	allocator_type _allocator;
	void _resize(size_type n) {
		if (_M_begin == NULL) n = 8;
		if (n == 0) return;
		char_type* old_begin = _M_begin;
		char_type* old_cur = _M_cur;
		_M_begin = _allocator.allocate(n);
		_M_cur = _M_begin;
		_M_end = _M_begin + n;
		*_M_cur = 0;
		if (old_begin) {
			for (char_type* cur = old_begin; cur < old_cur; cur++, _M_cur++)
				*_M_cur = *cur;
			*_M_cur = 0;
		}
	}
public:
	string_builder(size_type n = 0) : _M_begin(nullptr),
		_M_cur(nullptr),
		_M_end(nullptr),
		_allocator() {
		_resize(n);
	}
	~string_builder() {
		if (_M_begin)
			_allocator.deallocate(_M_begin, _M_end - _M_begin);
	}
	size_type capacity() const {
		return _M_end - _M_begin;
	}
	size_type length() const {
		return _M_cur - _M_begin;
	}
	string_builder& length(size_type _new_length) {
		if (_new_length < length()) {
			_M_cur = _M_begin + _new_length;
			*_M_cur = 0;
		}
		return *this;
	}
	string_builder& push(char_type ch) {
		if (_M_cur == NULL || _M_cur == _M_end - 1) {
			_resize(length() * 2);
		}
		*(_M_cur++) = ch;
		*_M_cur = 0;
		return *this;
	}
	string_builder& append(const char_type* str) {
		while (*str != 0) {
			push(*(str++));
		}
		return *this;
	}
	template <class _Iterator>
	string_builder& append(_Iterator _begin, _Iterator _end) {
		_Iterator it = _begin;
		for (; it != _end; it++)
			push(*it);
		return *this;
	}
	string_builder& clear() {
		_M_cur = _M_begin;
		*_M_cur = 0;
		return *this;
	}
	char_type* c_str() const {
		return _M_begin;
	}
	char_type* begin() const {
		return _M_begin;
	}
	char_type* end() const {
		return _M_cur;
	}
	const char_type* cbegin() const {
		return _M_begin;
	}
	const char_type* cend() const {
		return _M_cur;
	}
	bool equals(const char_type* str) const;
	bool endswith(const char_type* str) const;
};

template<typename _CharT>
struct xml_defs {
	typedef _CharT char_type;
	static const char_type* text(int index) { return _char_xml_constants[index]; }
	static const bool is_space(char_type ch) {
		return ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ';
	}
	static int strcmp(const char_type* s1, const char_type* s2) {
		return ::strcmp(s1,s2);
	}
	static int strlen(const char_type* s) {
		return ::strlen(s);
	}
	static int strncmp(const char_type* s1, const char_type* s2, size_t n) {
		return ::strncmp(s1,s2,n);
	}
	/** @return true when s1 ends with s2. */
	static bool endswith(const char_type* s1, const char_type* s2) {
		size_t ls1 = ::strlen(s1);
		size_t ls2 = ::strlen(s2);
		// A shorter string cannot end with a longer one. The old else-branch
		// asked the reverse question -- whether s1 was a suffix of s2 -- so
		// endswith(">", "/>") was true and every open tag looked self-closing.
		if (ls1 < ls2)
			return false;
		return ::strcmp(s1 + ls1 - ls2, s2) == 0;
	}
	static bool is_tag(const char_type* str) {
		return std::regex_match (str, IS_TAG) || std::regex_match (str, CLOSE_TAG);
	}
	static bool is_ident(char_type ch) {
		return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')
				|| ch == '_' || ch == '#';
	}
	static bool default_entities(const char_type* name, string_builder<char_type,std::allocator<char_type>> &result) {
		const char_type* value = NULL;
		char_type buf[2]= {0,0};
		// strcmp returns 0 on equality: these tests were inverted, so every
		// entity resolved to the value of the first name it did NOT match
		// (&amp; decoded to ">").
		if (strcmp(name,"amp") == 0)
			value = "&";
		else if (strcmp(name,"gt") == 0)
			value = ">";
		else if (strcmp(name,"lt") == 0)
			value = "<";
		else if (strcmp(name,"quot") == 0)
			value = "\"";
		else if (strcmp(name,"apos") == 0)
			value = "'";
		else if (std::regex_match(name, name+strlen(name),ENT_NUM)) {
			char_type* endp;
			int val = std::strtol(name, &endp,10);
			buf[0] = (char_type) val;
			value = buf;
		} else if (std::regex_match(name, name+strlen(name),ENT_HEX)) {
			char_type* endp;
			int val = std::strtol(name, &endp,16);
			buf[0] = (char_type) val;
			value = buf;
		}
		if (value) result.append(value);
		return value != NULL;
	}
	static std::basic_string<char_type> parse_characters(const char_type* str, tag_handler<char_type>* _handler) {
		const char_type* send = str + strlen(str);
		string_builder<char_type,std::allocator<char_type>> result;
		string_builder<char_type,std::allocator<char_type>> entity;
		char_type amp = *text(XC_AMP);
		char_type sc = *text(XC_SEMICOLON);
		bool in_entity = false;
		for (; str < send; str++) {
			if (in_entity) {
				if (*str == sc) {
					if (!default_entities(entity.c_str(), result)) {
						result.append(_handler->entity(entity.c_str()));
					}
					entity.clear();
					in_entity = false;
				} else if (is_ident(*str)) {
					entity.push(*str);
				} else {
					throw xml_parser_exception("Invalid entity");
				}
			} else {
				if (*str == amp) {
					in_entity = true;
				} else
					result.push(*str);
			}
		}
		return result.c_str();
	}
	static std::basic_string<char_type> remove_colon(const std::basic_string<char_type>& str) {
		if (str.length() != 0 && str.back() == ':') {
			return str.substr(0, str.length() - 1);
		} else
			return str;
	}
	template<typename _Found_proc>
	static bool find_attr(const char_type* str, _Found_proc& found_proc, tag_handler<char_type>* _handler) {
		const char_type* send = str + strlen(str);
		while (str < send) {
			std::cmatch match;
			if (!std::regex_search(str, send, match, ATTRS))
				return false;
			found_proc(remove_colon(match[1]), match[2], parse_characters(match[3].str().c_str(), _handler));
			// Advance to the end of the match, not by its length: regex_search
			// may match at an offset. Guard against a zero-width match, which
			// would otherwise spin here forever.
			const char_type* next = match[0].second;
			if (next <= str)
				return false;
			str = next;
		}
		return true;
	}
	template<typename _Found_proc>
	static bool find_tag(const char_type* str, _Found_proc& found_proc) {
		const char_type* send = str + strlen(str);
		std::cmatch match;
		std::regex_search(str, send, match, IS_TAG);
		if (match.size() != 0) {
			found_proc(remove_colon(match[1]), match[2]);
			return true;
		} else
			return false;
	}
	template<typename _Found_proc>
	static bool find_closetag(const char_type* str, _Found_proc& found_proc) {
		const char_type* send = str + strlen(str);
		std::cmatch match;
		std::regex_search(str, send, match, CLOSE_TAG);
		if (match.size() != 0) {
			found_proc(remove_colon(match[1]), match[2]);
			return true;
		} else
			return false;
	}
};

template<>
struct xml_defs<wchar_t> {
	typedef wchar_t char_type;
	static const char_type* text(int index) { return _wchar_xml_constants[index]; }
	static const bool is_space(char_type ch) {
		return ch == L'\n' || ch == L'\r' || ch == L'\t' || ch == L' ';
	}
	static int strlen(const char_type* s) {
		return ::wcslen(s);
	}
	static int strcmp(const wchar_t* s1, const wchar_t* s2) {
		return ::wcscmp(s1,s2);
	}
	static int strncmp(const wchar_t* s1, const wchar_t* s2, size_t n) {
		return ::wcsncmp(s1,s2,n);
	}
	/** @return true when s1 ends with s2. See the char specialisation. */
	static bool endswith(const char_type* s1, const char_type* s2) {
		size_t ls1 = ::wcslen(s1);
		size_t ls2 = ::wcslen(s2);
		if (ls1 < ls2)
			return false;
		return ::wcscmp(s1 + ls1 - ls2, s2) == 0;
	}
	static bool is_ident(char_type ch) {
		return (ch >= L'A' && ch <= L'Z') || (ch >= L'a' && ch <= L'z') || (ch >= L'0' && ch <= L'9')
				|| ch == L'_' || ch == L'#';
	}
	static bool is_tag(const char_type* str) {
		return std::regex_match (str, IS_TAGW) || std::regex_match (str, CLOSE_TAGW);
	}
	static bool default_entities(const char_type* name, string_builder<char_type,std::allocator<char_type>> &result) {
		const char_type* value = NULL;
		char_type buf[2]= {0,0};
		// See the char specialisation: these comparisons were inverted.
		if (strcmp(name,L"amp") == 0)
			value = L"&";
		else if (strcmp(name,L"gt") == 0)
			value = L">";
		else if (strcmp(name,L"lt") == 0)
			value = L"<";
		else if (strcmp(name,L"quot") == 0)
			value = L"\"";
		else if (strcmp(name,L"apos") == 0)
			value = L"'";
		else if (std::regex_match(name, name+strlen(name),ENT_NUMW)) {
			char_type* endp;
			int val = std::wcstol(name, &endp,10);
			buf[0] = (char_type) val;
			value = buf;
		} else if (std::regex_match(name, name+strlen(name),ENT_HEXW)) {
			char_type* endp;
			int val = std::wcstol(name, &endp,16);
			buf[0] = (char_type) val;
			value = buf;
		}
		if (value) result.append(value);
		return value != NULL;
	}
	static std::basic_string<char_type> parse_characters(const char_type* str, tag_handler<char_type>* _handler) {
		const char_type* send = str + strlen(str);
		string_builder<char_type,std::allocator<char_type>> result;
		string_builder<char_type,std::allocator<char_type>> entity;
		char_type amp = *text(XC_AMP);
		char_type sc = *text(XC_SEMICOLON);
		bool in_entity = false;
		for (; str < send; str++) {
			if (in_entity) {
				if (*str == sc) {
					if (!default_entities(entity.c_str(), result)) {
						result.append(_handler->entity(entity.c_str()));
					}
					entity.clear();
					in_entity = false;
				} else if (is_ident(*str)) {
					entity.push(*str);
				} else {
					throw xml_parser_exception("Invalid entity");
				}
			} else {
				if (*str == amp) {
					in_entity = true;
				} else
					result.push(*str);
			}
		}
		return result.c_str();
	}
	static std::basic_string<char_type> remove_colon(const std::basic_string<char_type>& str) {
		if (str.length() != 0 && str.back() == L':') {
			return str.substr(0, str.length() - 1);
		} else
			return str;
	}
	template<typename _Found_proc>
	static bool find_attr(const char_type* str, _Found_proc& found_proc, tag_handler<char_type>* _handler) {
		const char_type* send = str + strlen(str);
		while (str < send) {
			std::wcmatch match;
			if (!std::regex_search(str, send, match, ATTRSW))
				return false;
			found_proc(remove_colon(match[1]), match[2], parse_characters(match[3].str().c_str(), _handler));
			// See the char specialisation.
			const char_type* next = match[0].second;
			if (next <= str)
				return false;
			str = next;
		}
		return true;
	}
	template<typename _Found_proc>
	static bool find_tag(const char_type* str, _Found_proc& found_proc) {
		const char_type* send = str + strlen(str);
		std::wcmatch match;
		std::regex_search(str, send, match, IS_TAGW);
		if (match.size() != 0) {
			found_proc(remove_colon(match[1]), match[2]);
			return true;
		} else
			return false;
	}
	template<typename _Found_proc>
	static bool find_closetag(const char_type* str, _Found_proc& found_proc) {
		const char_type* send = str + strlen(str);
		std::wcmatch match;
		std::regex_search(str, send, match, CLOSE_TAGW);
		if (match.size() != 0) {
			found_proc(remove_colon(match[1]), match[2]);
			return true;
		} else
			return false;
	}
};

template <typename _CharT, class _Alloc>
inline bool string_builder<_CharT, _Alloc>::equals(const char_type* str) const {
	return xml_defs<char_type>::strcmp(_M_begin, str) == 0;
}
template <typename _CharT, class _Alloc>
inline bool string_builder<_CharT, _Alloc>::endswith(const char_type* str) const {
	size_t ls2 = xml_defs<char_type>::strlen(str);
	if (length() < ls2) return false;
	return xml_defs<char_type>::strcmp(_M_cur - ls2, str) == 0;
}

#endif /* XML_PARSER_UTIL_H_ */
