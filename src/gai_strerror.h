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
#if defined(HAVE_GETADDRINFO) || defined(HAVE_GETNAMEINFO) || defined(HAVE_GAI_STRERROR)
#define gai_strerror my_gai_strerror
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
 * Functions.
 */
#ifdef __STDC__
const char *gai_strerror(int);
#else
const char *gai_strerror();
#endif

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* not _D_GETADDRINFO_H */
