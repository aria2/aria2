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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#ifndef _D_A2NETCOMPAT_H_
#define _D_A2NETCOMPAT_H_

#include "a2io.h"

#ifndef __CYGWIN__
# ifdef HAVE_WINSOCK2_H
#  ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x501
#  endif // _WIN32_WINNT
#  include <winsock2.h>
#  undef ERROR
# endif // HAVE_WINSOCK2_H
# ifdef HAVE_WS2TCPIP_H
#  include <ws2tcpip.h>
# endif // HAVE_WS2TCPIP_H
#endif // !__CYGWIN__

#ifdef __MINGW32__
# define SOCKOPT_T const char
# define HAVE_GETADDRINFO
# undef HAVE_GAI_STRERROR
# undef gai_strerror
#else
# define SOCKOPT_T socklen_t
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

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif // HAVE_ARPA_INET_H

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif // HAVE_NETINET_IN_H

#ifndef HAVE_INET_ATON
# include "inet_aton.h"
#endif // HAVE_INET_ATON

#ifndef HAVE_GETADDRINFO
# include "getaddrinfo.h"
# define HAVE_GAI_STRERROR
#endif // HAVE_GETADDRINFO

#ifndef HAVE_GAI_STRERROR
# include "gai_strerror.h"
#endif // HAVE_GAI_STRERROR

#endif // _D_A2NETCOMPAT_H_
