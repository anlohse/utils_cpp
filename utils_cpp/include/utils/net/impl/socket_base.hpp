/*
 * socket_base.hpp
 *
 *  Created on: 12/04/2011
 *      Author: Alan
 */

#ifndef SOCKET_BASE_HPP_
#define SOCKET_BASE_HPP_
#include <utils/utils_defs.hpp>
#include <utils/net/socket_iostream.hpp>
#include <utils/net/socket_address.hpp>
#include <utils/net/socket_exception.hpp>
#include <utils/net/url.hpp>
#include <utils/funcs.hpp>
#include <memory>

namespace utils {

namespace net {

namespace __socket_impl {

void socket_timeout(int fd, int timeout);

class socket_base {
public:
	enum socket_option {
		so_debug = 1,
		so_accept_conn = 2,
		so_reuse_addr = 4,
		so_keep_alive = 8,
		so_dont_route = 16,
		so_broat_cast = 32,
		so_use_loop_back = 64,
		so_linger = 128,
		so_oob_inline = 256,
		so_dont_linger = 0xFFFFFF7F,
		so_exclusive_addr_use = 0xFFFFFFFB,
		so_send_buf = 0x1001,
		so_recv_buf = 0x1002,
		so_send_low_wm = 0x1003,
		so_recv_low_wm = 0x1004,
		so_send_timeout = 0x1005,
		so_recv_timeout = 0x1006,
		so_error = 0x1007,
		so_type = 0x1008
	};
	enum socket_domain {
		UNSPEC_DOMAIN=0,
		INET=2,
		INET6=23
	};
	enum socket_type {
		STREAM=1,
		DATAGRAM=2,
		RAW=3,
		RDM=4,
		SEQ_PACKET=5
	};
	enum socket_protocol {
		UNSPEC_PROTO=0,
		TCP=6,
		UDP=17
	};
protected:
	struct socket_data {
		int _M_fd;
		int _M_timeout;
		int _M_ref;
		socket_domain _M_domain;
		socket_type _M_type;
		socket_protocol _M_protocol;
		socket_data(): _M_fd(0),
			_M_timeout(0),
			_M_ref(0),
			_M_domain(UNSPEC_DOMAIN),
			_M_type(STREAM),
			_M_protocol(UNSPEC_PROTO) {
		}
		socket_data(int fd,
			int timeout,
			socket_domain domain,
			socket_type type,
			socket_protocol protocol): _M_fd(fd),
			_M_timeout(timeout),
			_M_ref(0),
			_M_domain(domain),
			_M_type(type),
			_M_protocol(protocol) {
		}
		~socket_data();
		socket_data* add_ref() {
			++_M_ref;
			return this;
		}
		int rem_ref() {
			return --_M_ref;
		}
		bool is_leak() {
			return _M_ref <= 0;
		}
	};

	typedef DEFAULT_ALLOCATOR<socket_data> data_allocator_type;
	// allocator::construct/destroy were removed in C++20; go through
	// allocator_traits, which works for every conforming allocator.
	typedef std::allocator_traits<data_allocator_type> data_alloc_traits;

	data_allocator_type allocator;
	socket_data* _M_data;

private:

	class socket_startup {
	public:
		int error;
		socket_startup();
		~socket_startup();
	};

	static socket_startup& _startup() {
		static socket_startup _startup;
		return _startup;
	}

	void create_socket(socket_domain domain, socket_type type, socket_protocol protocol);

protected:
	socket_base(const socket_base& other):allocator(other.allocator), _M_data(other._M_data->add_ref()) {

	}

	socket_base& operator = (const socket_base& other) {
		if (_M_data->rem_ref() <= 0) {
			data_alloc_traits::destroy(allocator, _M_data);
			allocator.deallocate(_M_data,1);
		}
		allocator = other.allocator;
		_M_data = other._M_data->add_ref();
		return *this;
	}

	socket_base(const DEFAULT_ALLOCATOR<socket_data>& _alloc, socket_data* _sd):allocator(_alloc), _M_data(_sd->add_ref()) {

	}

	virtual socket_data* _accept();

public:
	socket_base():allocator(),_M_data(NULL) {
		// creates a IPv4 of type stream using protocol unspecified by default
		create_socket(INET,STREAM,UNSPEC_PROTO);
	}

	socket_base(socket_domain domain, socket_type type, socket_protocol protocol):allocator(),_M_data(NULL) {
		create_socket(domain,type,protocol);
	}

	virtual ~socket_base() {
		if (_M_data->rem_ref() <= 0) {
			close();
			data_alloc_traits::destroy(allocator, _M_data);
			allocator.deallocate(_M_data,1);
		}
	}

	virtual void connect(const socket_address& address);

	virtual void bind(const socket_address& service);

	virtual void listen(int max_queued);

	virtual socket_address get_socket_address();

	virtual socket_iostream get_iostream();

	virtual void sendto(const socket_address& to_addr, char* buff, size_t buf_len);

	virtual socket_address recvfrom(char* buff, size_t buf_len, bool just_peek);

	virtual void set_timeout(int timeout);

	virtual void close();

};



}  // namespace __socket_impl

}  // namespace net

}  // namespace utils

#endif /* SOCKET_BASE_HPP_ */
