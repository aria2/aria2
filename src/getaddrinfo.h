/*
 * Copyright (c) 2001, 02  Motoyuki Kasahara
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _D_GETADDRINFO_H
#define _D_GETADDRINFO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif // HAVE_CONFIG_H

#ifdef __MINGW32__
# ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x501
# endif // _WIN32_WINNT
# include <winsock2.h>
# undef ERROR
# include <ws2tcpip.h>
#endif // __MINGW32__

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif // HAVE_SYS_SOCKET_H
#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif // HAVE_NETDB_H

#include <sys/types.h>

/********************************************************************/
/*
 * Undefine all the macros.
 * <netdb.h> might defines some of them.
 */
#ifdef EAI_ADDRFAMILY
#undef EAI_ADDRFAMILY
#endif
#ifdef EAI_AGAIN
#undef EAI_AGAIN
#endif
#ifdef EAI_BADFLAGS
#undef EAI_BADFLAGS
#endif
#ifdef EAI_FAIL
#undef EAI_FAIL
#endif
#ifdef EAI_FAMILY
#undef EAI_FAMILY
#endif
#ifdef EAI_MEMORY
#undef EAI_MEMORY
#endif
#ifdef EAI_NONAME
#undef EAI_NONAME
#endif
#ifdef EAI_OVERFLOW
#undef EAI_OVERFLOW
#endif
#ifdef EAI_SERVICE
#undef EAI_SERVICE
#endif
#ifdef EAI_SOCKTYPE
#undef EAI_SOCKTYPE
#endif
#ifdef EAI_SYSTEM
#undef EAI_SYSTEM
#endif

#ifdef AI_PASSIVE
#undef AI_PASSIVE
#endif
#ifdef AI_CANONNAME
#undef AI_CANONNAME
#endif
#ifdef AI_NUMERICHOST
#undef AI_NUMERICHOST
#endif
#ifdef AI_NUMERICSERV
#undef AI_NUMERICSERV
#endif
#ifdef AI_V4MAPPED
#undef AI_V4MAPPED
#endif
#ifdef AI_ALL
#undef AI_ALL
#endif
#ifdef AI_ADDRCONFIG
#undef AI_ADDRCONFIG
#endif
#ifdef AI_DEFAULT
#undef AI_DEFAULT
#endif

#ifdef NI_NOFQDN
#undef NI_NOFQDN
#endif
#ifdef NI_NUMERICHOST
#undef NI_NUMERICHOST
#endif
#ifdef NI_NAMEREQD
#undef NI_NAMEREQD
#endif
#ifdef NI_NUMERICSERV
#undef NI_NUMERICSERV
#endif
#ifdef NI_NUMERICSCOPE
#undef NI_NUMERICSCOPE
#endif

#ifdef NI_DGRAM
#undef NI_DGRAM
#endif
#ifdef NI_MAXHOST
#undef NI_MAXHOST
#endif
#ifdef NI_MAXSERV
#undef NI_MAXSERV
#endif

/*
 * Fake struct and function names.
 * <netdb.h> might declares all or some of them.
 */
#if defined(HAVE_GETADDRINFO) || defined(HAVE_GETNAMEINFO)
#define addrinfo my_addrinfo
#define gai_strerror my_gai_strerror
#define freeaddrinfo my_freeaddrinfo
#define getaddrinfo my_getaddrinfo
#define getnameinfo my_getnameinfo
#endif

/********************************************************************/
/*
 * Error codes.
 */
#define EAI_ADDRFAMILY	1
#define EAI_AGAIN	2
#define EAI_BADFLAGS	3
#define EAI_FAIL	4
#define EAI_FAMILY	5
#define EAI_MEMORY	6
#define EAI_NONAME	7
#define EAI_OVERFLOW	8
#define EAI_SERVICE	9
#define EAI_SOCKTYPE	10
#define EAI_SYSTEM	11

/*
 * Flags for getaddrinfo().
 */
#define AI_ADDRCONFIG	0x0001
#define AI_ALL		0x0002
#define AI_CANONNAME	0x0004
#define AI_NUMERICHOST	0x0008
#define AI_NUMERICSERV	0x0010
#define AI_PASSIVE	0x0020
#define AI_V4MAPPED	0x0040
#define AI_DEFAULT	(AI_V4MAPPED | AI_ADDRCONFIG)

/*
 * Flags for getnameinfo().
 */
#define NI_DGRAM	0x0001
#define NI_NAMEREQD	0x0002
#define NI_NOFQDN	0x0004
#define NI_NUMERICHOST	0x0008
#define NI_NUMERICSCOPE	0x0010
#define NI_NUMERICSERV	0x0020

/*
 * Maximum length of FQDN and servie name for getnameinfo().
 */
#define NI_MAXHOST	1025
#define NI_MAXSERV	32

/*
 * Address families and Protocol families.
 */
#ifndef AF_UNSPEC
#define AF_UNSPEC AF_INET
#endif
#ifndef PF_UNSPEC
#define PF_UNSPEC PF_INET
#endif

#ifndef __MINGW32__

/*
 * struct addrinfo.
 */
struct addrinfo {
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    socklen_t ai_addrlen;
    char *ai_canonname;
    struct sockaddr *ai_addr;
    struct addrinfo *ai_next;
};

#endif // __MINGW32__

/*
 * Functions.
 */
#ifdef __STDC__
const char *gai_strerror(int);
void freeaddrinfo(struct addrinfo *);
int getaddrinfo(const char *, const char *, const struct addrinfo *,
    struct addrinfo **);
int getnameinfo(const struct sockaddr *, socklen_t, char *,
    socklen_t, char *, socklen_t, int);
#else
const char *gai_strerror();
void freeaddrinfo();
int getaddrinfo();
int getnameinfo();
#endif

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* not _D_GETADDRINFO_H */
