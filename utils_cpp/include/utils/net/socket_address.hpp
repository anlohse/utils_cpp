/*
 * socket_address.hpp
 *
 *  Created on: 24/04/2011
 *      Author: Alan
 */

#ifndef SOCKET_ADDRESS_HPP_
#define SOCKET_ADDRESS_HPP_

#include <utils/utils_defs.hpp>
#include <cstring>
#include <string>

namespace utils {

namespace net {

namespace __socket_impl {
class socket_base;
}  // namespace __socket_impl

/**
 * Size of the opaque sockaddr storage, large enough for sockaddr_in6.
 *
 * A constant rather than a #define: ADDRSIZE was previously a macro leaking
 * into every translation unit that included this header.
 */
inline constexpr size_t ADDRSIZE = 32;

class socket_address {
protected:
	friend class __socket_impl::socket_base;
	char buff[ADDRSIZE];
	socket_address() {
		::memset(buff,0,ADDRSIZE);
	}
public:
	// t_word (uint16_t), not the BSD alias u_short: a port is 16 bits by
	// definition, and u_short is not portable.
	socket_address(const std::string& hostname, t_word port);
	~socket_address() {
	}

	std::string get_host_name() const;

	int get_length() const;

	const void* get_sockaddr() const;

	std::string get_ip_address() const;

	t_word get_port() const;
};

}

}

#endif /* SOCKET_ADDRESS_HPP_ */
