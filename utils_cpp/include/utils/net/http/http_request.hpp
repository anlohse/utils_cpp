/*
 * http_request.hpp
 *
 *  Created on: 26/04/2011
 *      Author: Alan
 */

#ifndef HTTP_REQUEST_HPP_
#define HTTP_REQUEST_HPP_

#include <utils/net/http/http.hpp>
#include <utils/net/url.hpp>
#include <utils/net/socket.hpp>
#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace utils {

namespace net {

namespace http {

struct http_request_data {
	typedef std::vector<std::string, DEFAULT_ALLOCATOR<std::string> > string_list;
	// hash_nocase, not hash<const std::string&>: parameter and header lookup is
	// case-insensitive, so the hash has to be too or differing-case keys never
	// resolve to the same bucket.
	typedef std::unordered_map<std::string, string_list, hash_nocase, equal_to_nocase<const std::string&> > parameter_map;
	typedef std::unordered_map<std::string, std::string, hash_nocase, equal_to_nocase<const std::string&> > headers_map;

	parameter_map _M_parameters;
	headers_map _M_headers;
	multipart_handler* _M_multipart_handler;
	socket _M_socket;
	int _M_refs;

	http_request_data* add_ref() {
		++_M_refs;
		return this;
	}
	int rem_ref() {
		return --_M_refs;
	}
	bool is_leak() const {
		return _M_refs <= 0;
	}
};

class http_request {
protected:

public:

};

}

}

}

#endif /* HTTP_REQUEST_HPP_ */
