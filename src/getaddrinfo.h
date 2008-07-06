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

#ifdef __MINGW32__
# undef SIZE_MAX
#endif // __MINGW32__

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

/* <from linux's netdb.h> */
/* Possible values for `ai_flags' field in `addrinfo' structure.  */
# define AI_PASSIVE     0x0001  /* Socket address is intended for `bind'.  */
# define AI_CANONNAME   0x0002  /* Request for canonical name.  */
# define AI_NUMERICHOST 0x0004  /* Don't use name resolution.  */
# define AI_V4MAPPED    0x0008  /* IPv4 mapped addresses are acceptable.  */
# define AI_ALL         0x0010  /* Return IPv4 mapped and IPv6 addresses.  */
# define AI_ADDRCONFIG  0x0020  /* Use configuration of this host to choose
                                   returned address type..  */
# ifdef __USE_GNU
#  define AI_IDN        0x0040  /* IDN encode input (assuming it is encoded
                                   in the current locale's character set)
                                   before looking it up. */
#  define AI_CANONIDN   0x0080  /* Translate canonical name from IDN format. */
#  define AI_IDN_ALLOW_UNASSIGNED 0x0100 /* Don't reject unassigned Unicode
                                            code points.  */
#  define AI_IDN_USE_STD3_ASCII_RULES 0x0200 /* Validate strings according to
                                                STD3 rules.  */
# endif
# define AI_NUMERICSERV 0x0400  /* Don't use name resolution.  */

/* Error values for `getaddrinfo' function.  */
# define EAI_BADFLAGS     -1    /* Invalid value for `ai_flags' field.  */
# define EAI_NONAME       -2    /* NAME or SERVICE is unknown.  */
# define EAI_AGAIN        -3    /* Temporary failure in name resolution.  */
# define EAI_FAIL         -4    /* Non-recoverable failure in name res.  */
# define EAI_NODATA       -5    /* No address associated with NAME.  */
# define EAI_FAMILY       -6    /* `ai_family' not supported.  */
# define EAI_SOCKTYPE     -7    /* `ai_socktype' not supported.  */
# define EAI_SERVICE      -8    /* SERVICE not supported for `ai_socktype'.  */
# define EAI_ADDRFAMILY   -9    /* Address family for NAME not supported.  */
# define EAI_MEMORY       -10   /* Memory allocation failure.  */
# define EAI_SYSTEM       -11   /* System error returned in `errno'.  */
# define EAI_OVERFLOW     -12   /* Argument buffer overflow.  */
# ifdef __USE_GNU
#  define EAI_INPROGRESS  -100  /* Processing request in progress.  */
#  define EAI_CANCELED    -101  /* Request canceled.  */
#  define EAI_NOTCANCELED -102  /* Request not canceled.  */
#  define EAI_ALLDONE     -103  /* All requests done.  */
#  define EAI_INTR        -104  /* Interrupted by a signal.  */
#  define EAI_IDN_ENCODE  -105  /* IDN encoding failed.  */
# endif

#define NI_MAXHOST	1025
#define NI_MAXSERV	32

# define NI_NUMERICHOST 1       /* Don't try to look up hostname.  */
# define NI_NUMERICSERV 2       /* Don't convert port number to name.  */
# define NI_NOFQDN      4       /* Only return nodename portion.  */
# define NI_NAMEREQD    8       /* Don't return numeric addresses.  */
# define NI_DGRAM       16      /* Look up UDP service rather than TCP.  */
# ifdef __USE_GNU
#  define NI_IDN        32      /* Convert name from IDN format.  */
#  define NI_IDN_ALLOW_UNASSIGNED 64 /* Don't reject unassigned Unicode
                                        code points.  */
#  define NI_IDN_USE_STD3_ASCII_RULES 128 /* Validate strings according to
                                             STD3 rules.  */
# endif
/* </from linux's netdb.h> */

#define AI_DEFAULT	(AI_V4MAPPED | AI_ADDRCONFIG)

/*
 * Address families and Protocol families.
 */
#ifndef AF_UNSPEC
#define AF_UNSPEC AF_INET
#endif
#ifndef PF_UNSPEC
#define PF_UNSPEC PF_INET
#endif

/* Nexenta OS(GNU/Solaris OS) defines `struct addrinfo' in netdb.h */
#if !defined( __MINGW32__ ) && !defined( __sun )

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

#endif // !__MINGW32__ && !__sun

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
