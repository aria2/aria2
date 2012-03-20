/*
 * Wslay - The WebSocket Library
 *
 * Copyright (c) 2011, 2012 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef WSLAY_NET_H
#define WSLAY_NET_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <wslay/wslay.h>

#ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
#endif /* HAVE_ARPA_INET_H */
#ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
#endif /* HAVE_NETINET_IN_H */

#ifdef WORDS_BIGENDIAN
#  ifndef HAVE_HTONS
#    define htons(x) (x)
#  endif /* !HAVE_HTONS */
#  ifndef HAVE_NTOHS
#    define ntohs(x) (x)
#  endif /* !HAVE_NTOHS */
#  define ntoh64(x) (x)
#  define hton64(x) (x)
#else /* !WORDS_BIGENDIAN */
uint16_t wslay_byteswap16(uint16_t x);
uint64_t wslay_byteswap64(uint64_t x);
#  ifndef HAVE_HTONS
#    define htons(x) wslay_byteswap16(x)
#  endif /* !HAVE_HTONS */
#  ifndef HAVE_NTOHS
#    define ntohs(x) wslay_byteswap16(x)
#  endif /* !HAVE_NTOHS */
#  define ntoh64(x) wslay_byteswap64(x)
#  define hton64(x) wslay_byteswap64(x)
#endif /* !WORDS_BIGENDIAN */

#endif /* WSLAY_NET_H */
