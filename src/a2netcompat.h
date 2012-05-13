/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#ifndef D_A2NETCOMPAT_H
#define D_A2NETCOMPAT_H

#include "a2io.h"

#ifdef __MINGW32__
# ifndef WINVER
#  define WINVER 0x501u
# endif // !WINVER
# ifdef HAVE_WINSOCK2_H
#  ifndef FD_SETSIZE
#    define FD_SETSIZE 1024
#  endif // !FD_SETSIZE
#  include <winsock2.h>
#  undef ERROR
# endif // HAVE_WINSOCK2_H
# ifdef HAVE_WS2TCPIP_H
#  include <ws2tcpip.h>
# endif // HAVE_WS2TCPIP_H
#endif // __MINGW32__

#ifdef __MINGW32__
# define a2_sockopt_t char *
# ifndef HAVE_GETADDRINFO
#   define HAVE_GETADDRINFO
# endif // !HAVE_GETADDRINFO
# undef HAVE_GAI_STRERROR
# undef gai_strerror
#else
# define a2_sockopt_t void *
#endif // __MINGW32__

#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif // HAVE_NETDB_H

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif // HAVE_SYS_SOCKET_H

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif // HAVE_NETINET_IN_H

#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif // HAVE_NETINET_TCP_H

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif // HAVE_ARPA_INET_H

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif // HAVE_NETINET_IN_H

#ifndef HAVE_GETADDRINFO
# include "getaddrinfo.h"
# define HAVE_GAI_STRERROR
#endif // HAVE_GETADDRINFO

#ifndef HAVE_GAI_STRERROR
# include "gai_strerror.h"
#endif // HAVE_GAI_STRERROR

#ifdef HAVE_WINSOCK2_H
# define sock_t SOCKET
#else
# define sock_t int
#endif

#ifndef AI_ADDRCONFIG
# define AI_ADDRCONFIG 0
#endif // !AI_ADDRCONFIG

#define DEFAULT_AI_FLAGS AI_ADDRCONFIG

#ifdef __MINGW32__
# ifndef SHUT_WR
#  define SHUT_WR SD_SEND
# endif // !SHUT_WR
#endif // __MINGW32__

#ifdef __MINGW32__
template<typename T>
class wsaapi_auto_delete {
private:
  T obj_;
  void (WSAAPI*deleter_)(T);
public:
  wsaapi_auto_delete(T obj, void (WSAAPI*deleter)(T)):
    obj_(obj), deleter_(deleter) {}

  ~wsaapi_auto_delete()
  {
    deleter_(obj_);
  }
};
# define WSAAPI_AUTO_DELETE wsaapi_auto_delete
#else // !__MINGW32__
# define WSAAPI_AUTO_DELETE auto_delete
#endif // !__MINGW32__

union sockaddr_union {
  sockaddr sa;
  sockaddr_storage storage;
  sockaddr_in6 in6;
  sockaddr_in in;
};

#endif // D_A2NETCOMPAT_H
