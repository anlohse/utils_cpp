/*
 * xml_parser.cpp
 *
 *  Created on: 15 de jun de 2018
 *      Author: alan.lohse
 */

#include "xml_parser_util.h"



template<typename _CharT>
struct attributes {
	std::vector<attribute<_CharT> > attrs;
	void operator()(const std::basic_string<_CharT>& ns,
			const std::basic_string<_CharT>& name,
			const std::basic_string<_CharT>& value) {
		attribute<_CharT> attr;
		attr.prefix = ns;
		attr.name = name;
		attr.value = value;
		attrs.push_back(attr);
	}
};

template<typename _CharT>
std::vector<attribute<_CharT> > parse_attrs(const _CharT* str, tag_handler<_CharT>* _handler) {
	attributes<_CharT> attrs;
	xml_defs<_CharT>::find_attr(str,attrs, _handler);
	return attrs.attrs;
}

template<typename _CharT>
struct tag_founder {
	std::basic_string<_CharT> prefix;
	std::basic_string<_CharT> name;
	void operator()(const std::basic_string<_CharT>& _prefix,
			const std::basic_string<_CharT>& _name) {
		prefix = _prefix;
		name = _name;
	}
};

template<typename _CharT>
bool parse_tag(const _CharT* str,
		tag_handler<_CharT>* _handler) {
	tag_founder<_CharT> _found_func;
	if (xml_defs<_CharT>::find_tag(str,_found_func)) {
		str += _found_func.prefix.length() + (_found_func.prefix.length() != 0 ? 1 : 0) + _found_func.name.length() + 1;
		std::vector<attribute<_CharT>> attrs = parse_attrs(str, _handler);
		_handler->start_element(_found_func.name.c_str(), _found_func.prefix.c_str(), attrs);
		if (xml_defs<_CharT>::endswith(str, xml_defs<_CharT>::text(XC_CLOSE_LEAF))) {
			_handler->end_element(_found_func.name.c_str(), _found_func.prefix.c_str());
		}
		return true;
	} else if (xml_defs<_CharT>::find_closetag(str,_found_func)) {
		_handler->end_element(_found_func.name.c_str(), _found_func.prefix.c_str());
		return true;
	}
	return false;
}

template<typename _CharT>
bool parse_doctype(const _CharT* str,
		tag_handler<_CharT>* _handler) {

	return true;
}

template<typename _CharT>
bool parse_xml_decl(const _CharT* str,
		tag_handler<_CharT>* _handler) {
	std::vector<attribute<_CharT> > attrs = parse_attrs(str, _handler);
	const _CharT* version = NULL;
	const _CharT* encoding = NULL;
	const _CharT* standalone = NULL;
	typename std::vector<attribute<_CharT> >::iterator it = attrs.begin();
	typename std::vector<attribute<_CharT> >::iterator end = attrs.end();
	for (; it != end; ++it) {
		if (it->name == xml_defs<_CharT>::text(XC_VERSION))
			version = it->value.c_str();
		if (it->name == xml_defs<_CharT>::text(XC_ENCODING))
			encoding = it->value.c_str();
		if (it->name == xml_defs<_CharT>::text(XC_STANDALONE))
			standalone = it->value.c_str();
	}
	_handler->xml_decl(version,encoding,standalone);
	return true;
}

template<typename _CharT>
bool read_tag(string_builder<_CharT,std::allocator<_CharT>> &builder,
		std::basic_istream<_CharT>& is,
		tag_handler<_CharT>* _handler) {
	bool in_tag = false;
	bool in_comment = false;
	bool in_cdata = false;
	bool in_doctype = false;
	bool in_xml = false;
	int tags_count = 0;
	while(is) {
		// get() returns int_type; casting straight to _CharT turns EOF into
		// 0xFF and appends it to the builder before the loop notices.
		typename std::basic_istream<_CharT>::int_type c = is.get();
		if (c == std::char_traits<_CharT>::eof())
			break;
		_CharT ch = static_cast<_CharT>(c);
		builder.push(ch);
		if (in_tag) {
			if (ch == *(xml_defs<_CharT>::text(XC_GT))) {
				bool result = parse_tag(builder.c_str(), _handler);
				builder.clear();
				return result;
			}
		} else if (in_doctype) {
			if (ch == *(xml_defs<_CharT>::text(XC_LT)))
				tags_count++;
			if (ch == *(xml_defs<_CharT>::text(XC_GT))) {
				if (tags_count == 0) {
					bool result = parse_doctype(builder.c_str(), _handler);
					builder.clear();
					return result;
				} else
					tags_count--;
			}
		} else if (in_comment) {
			const _CharT* begin_comments = xml_defs<_CharT>::text(XC_BEGIN_COMMENT);
			const _CharT* end_comments = xml_defs<_CharT>::text(XC_END_COMMENT);
			if (builder.endswith(end_comments)) {
				builder.length(builder.length() - xml_defs<_CharT>::strlen(end_comments));
				_handler->comments(builder.c_str() + xml_defs<_CharT>::strlen(begin_comments),
						builder.cend());
				builder.clear();
				return true;
			}
		} else if (in_xml) {
			const _CharT* begin_xml_decl = xml_defs<_CharT>::text(XC_BEGIN_XML_DECL);
			const _CharT* end_xml_decl = xml_defs<_CharT>::text(XC_END_XML_DECL);
			if (builder.endswith(end_xml_decl)) {
				builder.length(builder.length() - xml_defs<_CharT>::strlen(end_xml_decl));
				bool result = parse_xml_decl(builder.c_str() + xml_defs<_CharT>::strlen(begin_xml_decl), _handler);
				builder.clear();
				return result;
			}
		} else if (in_cdata) {
			const _CharT* begin_cdata = xml_defs<_CharT>::text(XC_BEGIN_CDATA);
			const _CharT* end_cdata = xml_defs<_CharT>::text(XC_END_CDATA);
			if (builder.endswith(end_cdata)) {
				builder.length(builder.length() - xml_defs<_CharT>::strlen(end_cdata));
				_handler->characters(builder.c_str() + xml_defs<_CharT>::strlen(begin_cdata),
						builder.cend());
				builder.clear();
				return true;
			}
		} else {
			if (builder.equals(xml_defs<_CharT>::text(XC_BEGIN_DOCTYPE)))
				in_doctype = true;
			else if (builder.equals(xml_defs<_CharT>::text(XC_BEGIN_CDATA)))
				in_cdata = true;
			else if (builder.equals(xml_defs<_CharT>::text(XC_BEGIN_COMMENT)))
				in_comment = true;
			else if (builder.equals(xml_defs<_CharT>::text(XC_BEGIN_XML_DECL)))
				in_xml = true;
			else if (xml_defs<_CharT>::is_tag(builder.c_str())) {
				if (ch == *(xml_defs<_CharT>::text(XC_GT))) {
					bool result = parse_tag(builder.c_str(), _handler);
					builder.clear();
					return result;
				} else
					in_tag = true;
			}
		}
	}
	_handler->fatal_error(xml_parser_exception("Unexpected end of file."));
	return false;
}

template<typename _CharT>
void parse_xml(std::basic_istream<_CharT>& is, tag_handler<_CharT>* _handler) {
	string_builder<_CharT,std::allocator<_CharT>> builder;
	_handler->start_document();
	while(is) {
		typename std::basic_istream<_CharT>::int_type c = is.get();
		if (c == std::char_traits<_CharT>::eof())
			break;
		_CharT ch = static_cast<_CharT>(c);
		if (ch == *(xml_defs<_CharT>::text(XC_LT))) {
			if (builder.length() != 0) {
				std::basic_string<_CharT> chars = xml_defs<_CharT>::parse_characters(builder.c_str(), _handler);
				// data(), not &(*begin()) / &(*cend()): dereferencing a string's
				// end iterator is undefined, and MSVC's debug iterators trap it
				// with a modal assertion -- which is what hung this test.
				_handler->characters(chars.data(), chars.data() + chars.size());
			}
			builder.clear();
			builder.push(ch);
			if (!read_tag(builder, is, _handler))
				break;
		} else
			builder.push(ch);
	}
	_handler->end_document();
}

void basic_xml_parser<char>::parse(std::basic_istream<char_type>& _input_stream, tag_handler<char_type>* _handler) {
	parse_xml(_input_stream, _handler);
}

void basic_xml_parser<wchar_t>::parse(std::basic_istream<char_type>& _input_stream, tag_handler<char_type>* _handler) {
	parse_xml(_input_stream, _handler);
}

