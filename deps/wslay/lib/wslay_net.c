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
#include "wslay_net.h"

#ifndef WORDS_BIGENDIAN

static uint16_t byteswap16(uint16_t x)
{
  return ((x & 0xffu) << 8) | (x >> 8);;
}

#ifdef HAVE_NTOHL
#  define byteswap32(x) ntohl(x)
#else /* !HAVE_NTOHL */
static uint32_t byteswap32(uint32_t x)
{
  uint32_t u = byteswap16(x & 0xffffu);
  uint32_t l = byteswap16(x >> 16);
  return (u << 16) | l;
}
#endif /* !HAVE_NTOHL */

static uint64_t byteswap64(uint64_t x)
{
  uint64_t u = byteswap32(x & 0xffffffffllu);
  uint64_t l = byteswap32(x >> 32);
  return (u << 32) | l;
}

uint16_t wslay_byteswap16(uint16_t x)
{
  return byteswap16(x);
}

uint64_t wslay_byteswap64(uint64_t x)
{
  return byteswap64(x);
}

#endif /* !WORDS_BIGENDIAN */
