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

/*
 * This program provides getaddrinfo() and getnameinfo() described in
 * RFC2133, 2553 and 3493.  These functions are mainly used for IPv6
 * application to resolve hostname or address.
 *
 * This program is designed to be working on traditional IPv4 systems
 * which don't have those functions.  Therefore, this implementation
 * supports IPv4 only.
 *
 * This program is useful for application which should support both IPv6
 * and traditional IPv4 systems.  Use genuine getaddrinfo() and getnameinfo()
 * provided by system if the system supports IPv6.  Otherwise, use this
 * implementation.
 *
 * This program is intended to be used in combination with GNU Autoconf.
 *
 * This program also provides freeaddrinfo() and gai_strerror().
 *
 * To use this program in your application, insert the following lines to
 * C source files after including `sys/types.h', `sys/socket.h' and
 * `netdb.h'.  `getaddrinfo.h' defines `struct addrinfo' and AI_, NI_,
 * EAI_ macros.
 *
 *    #ifndef HAVE_GETADDRINFO
 *    #include "getaddrinfo.h"
 *    #endif
 *
 * Restriction:
 *   getaddrinfo() and getnameinfo() of this program are NOT thread
 *   safe, unless the cpp macro ENABLE_PTHREAD is defined.
 */

/*
 * Add the following code to your configure.ac (or configure.in).
 *   AC_C_CONST
 *   AC_HEADER_STDC
 *   AC_CHECK_HEADERS(string.h memory.h stdlib.h)
 *   AC_CHECK_FUNCS(memcpy)
 *   AC_REPLACE_FUNCS(memset)
 *   AC_TYPE_SOCKLEN_T
 *   AC_TYPE_IN_PORT_T
 *   AC_DECL_H_ERRNO
 *
 *   AC_CHECK_FUNCS(getaddrinfo getnameinfo)
 *   if test "$ac_cv_func_getaddrinfo$ac_cv_func_getnameinfo" != yesyes ; then
 *       LIBOBJS="$LIBOBJS getaddrinfo.$ac_objext"
 *   fi
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef __MINGW32__
#  include <winsock2.h>
#  undef ERROR
#  include <ws2tcpip.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#  include <netdb.h>
#endif

#include <sys/types.h>
#include <stdio.h>

#if defined(STDC_HEADERS) || defined(HAVE_STRING_H)
#  include <string.h>
#  if !defined(STDC_HEADERS) && defined(HAVE_MEMORY_H)
#    include <memory.h>
#  endif /* not STDC_HEADERS and HAVE_MEMORY_H */
#else    /* not STDC_HEADERS and not HAVE_STRING_H */
#  include <strings.h>
#endif /* not STDC_HEADERS and not HAVE_STRING_H */

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#ifdef ENABLE_PTHREAD
#  include <pthread.h>
#endif

#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

#ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy((s), (d), (n))
#  ifdef __STDC__
void* memchr(const void*, int, size_t);
int memcmp(const void*, const void*, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
#  else  /* not __STDC__ */
char* memchr();
int memcmp();
char* memmove();
char* memset();
#  endif /* not __STDC__ */
#endif   /* not HAVE_MEMCPY */

#ifndef H_ERRNO_DECLARED
extern int h_errno;
#endif

#include "getaddrinfo.h"

#ifdef ENABLE_NLS
#  define _(string) gettext(string)
#  ifdef gettext_noop
#    define N_(string) gettext_noop(string)
#  else
#    define N_(string) (string)
#  endif
#else
#  define gettext(string) (string)
#  define _(string) (string)
#  define N_(string) (string)
#endif

/*
 * Error messages for gai_strerror().
 */
static char* eai_errlist[] = {
    N_("Success"),

    /* EAI_ADDRFAMILY */
    N_("Address family for hostname not supported"),

    /* EAI_AGAIN */
    N_("Temporary failure in name resolution"),

    /* EAI_BADFLAGS */
    N_("Invalid value for ai_flags"),

    /* EAI_FAIL */
    N_("Non-recoverable failure in name resolution"),

    /* EAI_FAMILY */
    N_("ai_family not supported"),

    /* EAI_MEMORY */
    N_("Memory allocation failure"),

    /* EAI_NONAME */
    N_("hostname nor servname provided, or not known"),

    /* EAI_OVERFLOW */
    N_("An argument buffer overflowed"),

    /* EAI_SERVICE */
    N_("servname not supported for ai_socktype"),

    /* EAI_SOCKTYPE */
    N_("ai_socktype not supported"),

    /* EAI_SYSTEM */
    N_("System error returned in errno")};

/*
 * Default hints for getaddrinfo().
 */
static struct addrinfo default_hints = {0, PF_UNSPEC, 0,    0,
                                        0, NULL,      NULL, NULL};

/*
 * Mutex.
 */
#ifdef ENABLE_PTHREAD
static pthread_mutex_t gai_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*
 * Declaration of static functions.
 */
#ifdef __STDC__
static int is_integer(const char*);
static int is_address(const char*);
static int itoa_length(int);
#else
static int is_integer();
static int is_address();
static int itoa_length();
#endif

/*
 * gai_strerror().
 */
const char* gai_strerror(ecode) int ecode;
{
  if (ecode < 0 || ecode > EAI_SYSTEM)
    return _("Unknown error");

  return gettext(eai_errlist[ecode]);
}

/*
 * freeaddrinfo().
 */
void freeaddrinfo(ai) struct addrinfo* ai;
{
  struct addrinfo* next_ai;

  while (ai != NULL) {
    if (ai->ai_canonname != NULL)
      free(ai->ai_canonname);
    if (ai->ai_addr != NULL)
      free(ai->ai_addr);
    next_ai = ai->ai_next;
    free(ai);
    ai = next_ai;
  }
}

/*
 * Return 1 if the string `s' represents an integer.
 */
static int is_integer(s) const char* s;
{
  if (*s == '-' || *s == '+')
    s++;
  if (*s < '0' || '9' < *s)
    return 0;

  s++;
  while ('0' <= *s && *s <= '9')
    s++;

  return (*s == '\0');
}

/*
 * Return 1 if the string `s' represents an IPv4 address.
 * Unlike inet_addr(), it doesn't permit malformed nortation such
 * as "192.168".
 */
static int is_address(s) const char* s;
{
  const static char delimiters[] = {'.', '.', '.', '\0'};
  int i, j;
  int octet;

  for (i = 0; i < 4; i++) {
    if (*s == '0' && *(s + 1) != delimiters[i])
      return 0;
    for (j = 0, octet = 0; '0' <= *s && *s <= '9' && j < 3; s++, j++)
      octet = octet * 10 + (*s - '0');
    if (j == 0 || octet > 255 || *s != delimiters[i])
      return 0;
    s++;
  }

  return 1;
}

/*
 * Calcurate length of the string `s', where `s' is set by
 * sprintf(s, "%d", n).
 */
static int itoa_length(n) int n;
{
  int result = 1;

  if (n < 0) {
    n = -n;
    result++;
  }

  while (n >= 10) {
    result++;
    n /= 10;
  }

  return result;
}

/*
 * getaddrinfo().
 */
int getaddrinfo(nodename, servname, hints, res) const char* nodename;
const char* servname;
const struct addrinfo* hints;
struct addrinfo** res;
{
  struct addrinfo* head_res = NULL;
  struct addrinfo* tail_res = NULL;
  struct addrinfo* new_res;
  struct sockaddr_in* sa_in;
  struct in_addr** addr_list;
  struct in_addr* addr_list_buf[2];
  struct in_addr addr_buf;
  struct in_addr** ap;
  struct servent* servent;
  struct hostent* hostent;
  const char* canonname = NULL;
  in_port_t port;
  int saved_h_errno;
  int result = 0;

#ifdef ENABLE_PTHREAD
  pthread_mutex_lock(&gai_mutex);
#endif

  saved_h_errno = h_errno;

  if (nodename == NULL && servname == NULL) {
    result = EAI_NONAME;
    goto end;
  }

  if (hints != NULL) {
    if (hints->ai_family != PF_INET && hints->ai_family != PF_UNSPEC) {
      result = EAI_FAMILY;
      goto end;
    }
    if (hints->ai_socktype != SOCK_DGRAM && hints->ai_socktype != SOCK_STREAM &&
        hints->ai_socktype != 0) {
      result = EAI_SOCKTYPE;
      goto end;
    }
  }
  else {
    hints = &default_hints;
  }

  if (servname != NULL) {
    if (is_integer(servname))
      port = htons(atoi(servname));
    else {
      if (hints->ai_flags & AI_NUMERICSERV) {
        result = EAI_NONAME;
        goto end;
      }

      if (hints->ai_socktype == SOCK_DGRAM)
        servent = getservbyname(servname, "udp");
      else if (hints->ai_socktype == SOCK_STREAM)
        servent = getservbyname(servname, "tcp");
      else if (hints->ai_socktype == 0)
        servent = getservbyname(servname, "tcp");
      else {
        result = EAI_SOCKTYPE;
        goto end;
      }

      if (servent == NULL) {
        result = EAI_SERVICE;
        goto end;
      }
      port = servent->s_port;
    }
  }
  else {
    port = htons(0);
  }

  if (nodename != NULL) {
    if (is_address(nodename)) {
      addr_buf.s_addr = inet_addr(nodename);
      addr_list_buf[0] = &addr_buf;
      addr_list_buf[1] = NULL;
      addr_list = addr_list_buf;

      if (hints->ai_flags & AI_CANONNAME &&
          !(hints->ai_flags & AI_NUMERICHOST)) {
        hostent =
            gethostbyaddr((char*)&addr_buf, sizeof(struct in_addr), AF_INET);
        if (hostent != NULL)
          canonname = hostent->h_name;
        else
          canonname = nodename;
      }
    }
    else {
      if (hints->ai_flags & AI_NUMERICHOST) {
        result = EAI_NONAME;
        goto end;
      }

      hostent = gethostbyname(nodename);
      if (hostent == NULL) {
        switch (h_errno) {
        case HOST_NOT_FOUND:
        case NO_DATA:
          result = EAI_NONAME;
          goto end;
        case TRY_AGAIN:
          result = EAI_AGAIN;
          goto end;
        default:
          result = EAI_FAIL;
          goto end;
        }
      }
      addr_list = (struct in_addr**)hostent->h_addr_list;

      if (hints->ai_flags & AI_CANONNAME)
        canonname = hostent->h_name;
    }
  }
  else {
    if (hints->ai_flags & AI_PASSIVE)
      addr_buf.s_addr = htonl(INADDR_ANY);
    else
      addr_buf.s_addr = htonl(0x7F000001);
    addr_list_buf[0] = &addr_buf;
    addr_list_buf[1] = NULL;
    addr_list = addr_list_buf;
  }

  for (ap = addr_list; *ap != NULL; ap++) {
    new_res = (struct addrinfo*)malloc(sizeof(struct addrinfo));
    if (new_res == NULL) {
      if (head_res != NULL)
        freeaddrinfo(head_res);
      result = EAI_MEMORY;
      goto end;
    }

    new_res->ai_family = PF_INET;
    new_res->ai_socktype = hints->ai_socktype;
    new_res->ai_protocol = hints->ai_protocol;
    new_res->ai_addr = NULL;
    new_res->ai_addrlen = sizeof(struct sockaddr_in);
    new_res->ai_canonname = NULL;
    new_res->ai_next = NULL;

    new_res->ai_addr = (struct sockaddr*)malloc(sizeof(struct sockaddr_in));
    if (new_res->ai_addr == NULL) {
      free(new_res);
      if (head_res != NULL)
        freeaddrinfo(head_res);
      result = EAI_MEMORY;
      goto end;
    }

    sa_in = (struct sockaddr_in*)new_res->ai_addr;
    memset(sa_in, 0, sizeof(struct sockaddr_in));
    sa_in->sin_family = PF_INET;
    sa_in->sin_port = port;
    memcpy(&sa_in->sin_addr, *ap, sizeof(struct in_addr));

    if (head_res == NULL)
      head_res = new_res;
    else
      tail_res->ai_next = new_res;
    tail_res = new_res;
  }

  if (canonname != NULL && head_res != NULL) {
    head_res->ai_canonname = (char*)malloc(strlen(canonname) + 1);
    if (head_res->ai_canonname != NULL)
      strcpy(head_res->ai_canonname, canonname);
  }

  *res = head_res;

end:
  h_errno = saved_h_errno;
#ifdef ENABLE_PTHREAD
  pthread_mutex_unlock(&gai_mutex);
#endif
  return result;
}

/*
 * getnameinfo().
 */
int getnameinfo(sa, salen, node, nodelen, serv, servlen, flags) const
    struct sockaddr* sa;
socklen_t salen;
char* node;
socklen_t nodelen;
char* serv;
socklen_t servlen;
int flags;
{
  const struct sockaddr_in* sa_in = (const struct sockaddr_in*)sa;
  struct hostent* hostent;
  struct servent* servent;
  char* ntoa_address;
  int saved_h_errno;
  int result = 0;

#ifdef ENABLE_PTHREAD
  pthread_mutex_lock(&gai_mutex);
#endif

  saved_h_errno = h_errno;

  if (sa_in->sin_family != PF_INET) {
    result = EAI_FAMILY;
    goto end;
  }
  else if (node == NULL && serv == NULL) {
    result = EAI_NONAME;
    goto end;
  }

  if (serv != NULL && servlen > 0) {
    if (flags & NI_NUMERICSERV)
      servent = NULL;
    else if (flags & NI_DGRAM)
      servent = getservbyport(sa_in->sin_port, "udp");
    else
      servent = getservbyport(sa_in->sin_port, "tcp");

    if (servent != NULL) {
      if (servlen <= strlen(servent->s_name)) {
        result = EAI_OVERFLOW;
        goto end;
      }
      strcpy(serv, servent->s_name);
    }
    else {
      if (servlen <= itoa_length(ntohs(sa_in->sin_port))) {
        result = EAI_OVERFLOW;
        goto end;
      }
      sprintf(serv, "%d", ntohs(sa_in->sin_port));
    }
  }

  if (node != NULL && nodelen > 0) {
    if (flags & NI_NUMERICHOST)
      hostent = NULL;
    else {
      hostent = gethostbyaddr((char*)&sa_in->sin_addr, sizeof(struct in_addr),
                              AF_INET);
    }
    if (hostent != NULL) {
      if (nodelen <= strlen(hostent->h_name)) {
        result = EAI_OVERFLOW;
        goto end;
      }
      strcpy(node, hostent->h_name);
    }
    else {
      if (flags & NI_NAMEREQD) {
        result = EAI_NONAME;
        goto end;
      }
      ntoa_address = inet_ntoa(sa_in->sin_addr);
      if (nodelen <= strlen(ntoa_address)) {
        result = EAI_OVERFLOW;
        goto end;
      }
      strcpy(node, ntoa_address);
    }
  }

end:
  h_errno = saved_h_errno;
#ifdef ENABLE_PTHREAD
  pthread_mutex_unlock(&gai_mutex);
#endif
  return result;
}
