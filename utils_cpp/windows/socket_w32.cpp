/*
 * socket_w32.cpp
 *
 *  Created on: 17/04/2011
 *      Author: Alan
 */

#include <utils/net/impl/socket_base.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <cstdio>
#include <cstdlib>


std::string get_socket_error() {
	return utils::get_error_message(WSAGetLastError());
}

void socket_timeout(int fd, int timeout, bool reading) {
	timeval tv;
	tv.tv_sec = timeout/1000;
	tv.tv_usec = 1000*(timeout%1000);
	timeval* lptv = &tv;
	fd_set* lpfds_r = NULL;
	fd_set* lpfds_w = NULL;
	fd_set tbl;
	FD_ZERO(&tbl);
	FD_SET((u_int)fd,&tbl);
	if (reading)
		lpfds_r = &tbl;
	else
		lpfds_w = &tbl;
	if (timeout <= 0) lptv = NULL;
	if(select(0, lpfds_r, lpfds_w, NULL, lptv) <= 0) {
	   throw utils::net::socket_exception("Socket timeout.");
	}
}

namespace utils {

namespace net {

std::streamsize sock_streambuf::socket_read(char_type* __s, std::streamsize __n) {
	 socket_timeout(_M_fd,_M_timeout,true);
     int n_bytes_recv;
     if((n_bytes_recv = recv(_M_fd, __s, __n, 0)) == SOCKET_ERROR) {
            throw socket_exception(WSAGetLastError());
     }
     return n_bytes_recv;

}

std::streamsize sock_streambuf::socket_write(const char_type* __s, std::streamsize __n) {
	socket_timeout(_M_fd,_M_timeout,false);
    int n_bytes_sent;
    if((n_bytes_sent = send(_M_fd, __s, __n, 0)) == SOCKET_ERROR) {
           throw socket_exception(WSAGetLastError());
    }
    return n_bytes_sent;
}

std::streamsize sock_streambuf::showmanyc() {
	u_long av = 0;
	if (ioctlsocket(_M_fd,FIONREAD,&av) != 0)
        throw socket_exception(WSAGetLastError());
	return av;
}

namespace __socket_impl {

socket_base::socket_startup::socket_startup(){
	WSADATA wsaData;
	error = WSAStartup(MAKEWORD(2,2), &wsaData);
}
socket_base::socket_startup::~socket_startup() {
	WSACleanup();
}

void socket_base::create_socket(socket_domain domain, socket_type type, socket_protocol protocol) {
	if (_startup().error != NO_ERROR)
    	throw socket_exception(WSAGetLastError());
	int fd = socket((int)domain, (int)type, (int)protocol);
    if (fd < 0)
    	throw socket_exception(WSAGetLastError());
	_M_data = allocator.allocate(1);
	socket_base::data_alloc_traits::construct(allocator, _M_data, socket_data());
	_M_data->_M_fd = fd;
	_M_data->_M_domain = domain;
	_M_data->_M_type = type;
	_M_data->_M_protocol = protocol;
}

socket_base::socket_data* socket_base::_accept() {
	int acc_fd = ::accept(_M_data->_M_fd, NULL, NULL);
	if ((SOCKET)acc_fd == INVALID_SOCKET)
		throw socket_exception(WSAGetLastError());
	socket_data* sock_data = allocator.allocate(1);
	socket_base::data_alloc_traits::construct(allocator, sock_data, socket_data());
	sock_data->_M_fd = acc_fd;
	sock_data->_M_domain = _M_data->_M_domain;
	sock_data->_M_type = _M_data->_M_type;
	sock_data->_M_protocol = _M_data->_M_protocol;
	return sock_data;
}

void socket_base::connect(const socket_address& address) {
	u_long iMode = 0;
	int r = ioctlsocket(_M_data->_M_fd,FIONBIO,&iMode);
	if (r < 0)
		throw socket_exception(WSAGetLastError());
	r = ::connect(_M_data->_M_fd,(const sockaddr*)address.get_sockaddr(),address.get_length());
	if (r < 0)
		throw socket_exception(WSAGetLastError());
}

void socket_base::bind(const socket_address& service) {
	if (::bind(_M_data->_M_fd,(const sockaddr*)service.get_sockaddr(),service.get_length()) == SOCKET_ERROR)
           throw socket_exception(WSAGetLastError());
}

void socket_base::listen(int max_queued) {
	if (::listen(_M_data->_M_fd, max_queued) == SOCKET_ERROR)
           throw socket_exception(WSAGetLastError());
}

socket_address socket_base::get_socket_address() {
	socket_address sa;
	int len = ADDRSIZE;
	if (getsockname(_M_data->_M_fd,
				(sockaddr*)sa.buff,
				&len) != 0)
           throw socket_exception(WSAGetLastError());
	return sa;
}

socket_iostream socket_base::get_iostream() {
	if (_M_data->_M_type != STREAM)
           throw socket_exception("Socket is not of stream type.");
	DEFAULT_ALLOCATOR<sock_streambuf> _stb_alloc;
	sock_streambuf* stbf = _stb_alloc.allocate(1);
	std::allocator_traits<DEFAULT_ALLOCATOR<sock_streambuf> >::construct(_stb_alloc, stbf, sock_streambuf(_M_data->_M_fd, _M_data->_M_timeout));
	socket_iostream str = socket_iostream(stbf,_stb_alloc);
	return str;
}

void socket_base::sendto(const socket_address& to_addr, char* buff, size_t buf_len) {
	if (::sendto(_M_data->_M_fd,buff, buf_len, 0, (sockaddr *) to_addr.get_sockaddr(), to_addr.get_length()) != 0)
           throw socket_exception(WSAGetLastError());
}

socket_address socket_base::recvfrom(char* buff, size_t buf_len, bool just_peek) {
	socket_address sa;
	int ad_len = ADDRSIZE;
	if (::recvfrom(_M_data->_M_fd,buff, buf_len, just_peek?MSG_PEEK:0, (sockaddr *) sa.buff, &ad_len) != 0)
           throw socket_exception(WSAGetLastError());
	return sa;
}

void socket_base::set_timeout(int timeout) {
	_M_data->_M_timeout=timeout;
	::setsockopt(_M_data->_M_fd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
}

void socket_base::close() {
	if (_M_data->_M_fd > 0) {
		::closesocket(_M_data->_M_fd);
	}
}


}  // namespace __socket_impl

}  // namespace net

}  // namespace utils
