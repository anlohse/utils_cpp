/*
 * socket_iostream.hpp
 *
 *  Created on: 14/04/2011
 *      Author: Alan
 */

#ifndef SOCKET_IOSTREAM_HPP_
#define SOCKET_IOSTREAM_HPP_

#include <utils/utils_defs.hpp>
#include <utils/exception.hpp>
#include <utils/net/sock_streambuf.hpp>
#include <string>
#include <iostream>

namespace utils {

namespace net {

class socket_iostream : public std::basic_iostream<char> {
protected:
	typedef DEFAULT_ALLOCATOR<sock_streambuf> streambuf_allocator_type;
	// allocator::destroy was removed in C++20.
	typedef std::allocator_traits<streambuf_allocator_type> streambuf_alloc_traits;

	streambuf_allocator_type streambuf_allocator;
	sock_streambuf* get_streambuf() const {
		return dynamic_cast<sock_streambuf*>(rdbuf());
	}
public:
	socket_iostream(): std::basic_iostream<char>(0), streambuf_allocator() {
	}

	socket_iostream(sock_streambuf* buf, DEFAULT_ALLOCATOR<sock_streambuf> _allocator): std::basic_iostream<char>(buf->add_ref()), streambuf_allocator(_allocator) {
	}

	virtual ~socket_iostream() {
		if (rdbuf() && get_streambuf()->rem_ref() <= 0) {
			streambuf_alloc_traits::destroy(streambuf_allocator, (sock_streambuf*)rdbuf());
			streambuf_allocator.deallocate((sock_streambuf*)rdbuf(),1);
		}
	}

	socket_iostream(const socket_iostream& sios): std::iostream(sios.get_streambuf()->add_ref()) {
	}

	socket_iostream& operator = (const socket_iostream &rhs) {
		if (rdbuf() && get_streambuf()->rem_ref() <= 0) {
			streambuf_alloc_traits::destroy(streambuf_allocator, (sock_streambuf*)rdbuf());
			streambuf_allocator.deallocate((sock_streambuf*)rdbuf(),1);
		}
		streambuf_allocator = rhs.streambuf_allocator;
		this->init(rhs.get_streambuf()->add_ref());
		return *this;
	}

};

}

}

#endif /* SOCKET_IOSTREAM_HPP_ */
