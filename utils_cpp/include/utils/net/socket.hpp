/*
 * socket.hpp
 *
 *  Created on: 12/04/2011
 *      Author: Alan
 */

#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include <utils/utils_defs.hpp>

#include <utils/net/impl/socket_base.hpp>

namespace utils {

namespace net {

//#if defined(UTILS_WINDOWS) || defined(UTILS_LINUX)
using __socket_impl::socket_base;
//#endif

class socket : public socket_base {
protected:
	socket(const DEFAULT_ALLOCATOR<socket_data>& _alloc, socket_data* _sd): socket_base(_alloc,_sd) {
	}
public:

	socket(): socket_base() {
	}

	socket(socket_domain domain, socket_type type, socket_protocol protocol): socket_base(domain, type, protocol) {
	}

	socket(const socket& other):socket_base(other) {

	}

	socket& operator = (socket& other) {
		if (_M_data->rem_ref() <= 0) {
			data_alloc_traits::destroy(allocator, _M_data);
			allocator.deallocate(_M_data,1);
		}
		allocator = other.allocator;
		_M_data = other._M_data->add_ref();
		return *this;
	}

	virtual ~socket() {

	}

	virtual socket accept() {
		socket_data* sd = _accept();
		socket result(allocator,sd);
		return result;
	}

};

}

}

#endif /* SOCKET_HPP_ */
