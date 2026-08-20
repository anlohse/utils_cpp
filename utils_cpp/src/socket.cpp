/*
 * socket.cpp
 *
 *  Created on: 22/04/2011
 *      Author: Alan
 */

#include <utils/net/impl/socket_base.hpp>
#if defined(UTILS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#elif defined(UTILS_LINUX)
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <cstdio>
#include <cstdlib>

#define SB_BUFF_SIZE 1024


std::string get_socket_error();

namespace utils {

namespace net {
//---------------------------------------------------------------------------------------------------------------------------
// socket_address

socket_address::socket_address(const std::string& hostname, t_word port) {
    addrinfo hints, *result, *ptr = NULL, *found=NULL;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;
	int r = getaddrinfo(hostname.c_str(), "domain", &hints, &result);
	if ( r != 0 )
		throw socket_exception(get_socket_error());
	for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
		if (ptr->ai_family == AF_INET) {
			found = ptr;
			if (ptr->ai_canonname)
				break;
		} else if (ptr->ai_family == AF_INET6) {
			found = ptr;
		}
	}
	memset(buff,0,ADDRSIZE);
	if (found==NULL) {
		freeaddrinfo(result);
		throw socket_exception("Address not found or not supported.");
	}
	if (found->ai_family == AF_INET) {
		sockaddr_in* addr = (sockaddr_in*)buff;
		addr->sin_family = found->ai_family;
		addr->sin_port = htons(port);
		addr->sin_addr = ((sockaddr_in*)found->ai_addr)->sin_addr;
	} else {
		sockaddr_in6* addr = (sockaddr_in6*)buff;
		addr->sin6_family = found->ai_family;
		addr->sin6_port = htons(port);
		addr->sin6_addr = ((sockaddr_in6*)found->ai_addr)->sin6_addr;
	}
	freeaddrinfo(result);
}

std::string socket_address::get_host_name() const {
	char hostname[NI_MAXHOST];
	int r = getnameinfo((const sockaddr*)buff,get_length(),
				hostname,NI_MAXHOST, NULL, 0, NI_NUMERICSERV);
	if (r != 0)
		throw socket_exception(get_socket_error());
	return hostname;
}

int socket_address::get_length() const {
	sockaddr* addr = (sockaddr*)buff;
	return addr->sa_family == AF_INET ? sizeof(sockaddr_in) : (
			addr->sa_family == AF_INET6 ? sizeof(sockaddr_in6):ADDRSIZE);
}

const void* socket_address::get_sockaddr() const {
	return (const void*)buff;
}

std::string socket_address::get_ip_address() const {
	if (((sockaddr*)buff)->sa_family == AF_INET)
		return inet_ntoa(((sockaddr_in*)buff)->sin_addr);
	return "";
}

t_word socket_address::get_port() const {
	if (((sockaddr*)buff)->sa_family == AF_INET)
		return htons(((sockaddr_in*)buff)->sin_port);
	else
		return htons(((sockaddr_in6*)buff)->sin6_port);
}

//---------------------------------------------------------------------------------------------------------------------------
// sock_streambuf

sock_streambuf::~sock_streambuf() {
	if (eback()) {
		_M_allocator.deallocate(eback(),2*SB_BUFF_SIZE);
	}
}
int sock_streambuf::underflow() {
	if (gptr() < egptr())
		return *(unsigned char*) gptr();

	if (eback() == 0 && doallocate() == 0)
		return EOF;

	int sz_r = socket_read(eback(), SB_BUFF_SIZE);
	if (sz_r <= 0)
		return EOF;
	setg(eback(), eback(), eback() + sz_r);
	return *(unsigned char*) gptr();
}
int sock_streambuf::overflow(int c) {
	if (c == EOF)
		return do_flush();

	if (pbase() == 0 && doallocate() == 0)
		return EOF;

	if (pptr() >= epptr() && do_flush() == EOF)
		return EOF;

	(*pptr()) = c;
	pbump(1);
	if (pptr() >= epptr()
			&& do_flush() == EOF)
		return EOF;
	return c;
}
int	sock_streambuf::doallocate () {
	if (!eback()) {
		char* buf = _M_allocator.allocate(2 * SB_BUFF_SIZE);
		setg(buf, buf+SB_BUFF_SIZE, buf+SB_BUFF_SIZE);
		buf += SB_BUFF_SIZE;
		setp(buf, buf + SB_BUFF_SIZE);
		return 1;
	}
	return 0;
}
int	sock_streambuf::sync() {
	return do_flush();
}
int sock_streambuf::do_flush() {
	if (pptr() <= pbase())
		return 0;
	int sz_tow = pptr() - pbase();
	int sz_w = socket_write(pbase(), sz_tow);
	int status = (sz_w == sz_tow) ? 0 : EOF;
	setp(pbase(), pbase() + SB_BUFF_SIZE);
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------
// socket_base::socket_data

namespace __socket_impl {

socket_base::socket_data::~socket_data() {
}

}

//---------------------------------------------------------------------------------------------------------------------------

}

}
