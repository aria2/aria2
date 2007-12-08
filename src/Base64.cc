/* <!-- copyright */
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
#include "Base64.h"

static const char CHAR_TABLE[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/',
};

static const int INDEX_TABLE[] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, 
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, 
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

void Base64::encode(unsigned char*& result, size_t& rlength,
		    const unsigned char* src, size_t slength)
{
  if(slength == 0) {
    rlength = 0;
    return;
  }
  size_t trituple = (slength+2)/3;
  int r = slength%3;
  rlength = trituple*4;
  result = new unsigned char[rlength];

  unsigned char* p = result;
  const unsigned char* s = src;
  const unsigned char* smax = s+slength-r;
  while(s != smax) {
    int n = *s++ << 16;
    n += *s++ << 8;
    n += *s++;
    *p++ = CHAR_TABLE[n >> 18];
    *p++ = CHAR_TABLE[n >> 12&0x3f];
    *p++ = CHAR_TABLE[n >> 6&0x3f];
    *p++ = CHAR_TABLE[n&0x3f];
  }
  if(r == 2) {
    int n = *s++ << 16;
    n += *s++ << 8;
    *p++ = CHAR_TABLE[n >> 18];
    *p++ = CHAR_TABLE[n >> 12&0x3f];
    *p++ = CHAR_TABLE[n >> 6&0x3f];
    *p++ = '=';
  } else if(r == 1) {
    int n = *s++ << 16;
    *p++ = CHAR_TABLE[n >> 18];
    *p++ = CHAR_TABLE[n >> 12&0x3f];
    *p++ = '=';
    *p++ = '=';
  }
}

void Base64::removeNonBase64Chars(unsigned char*& nsrc,
				  size_t& nlength,
				  const unsigned char* src,
				  size_t slength)
{
  unsigned char* temp = new unsigned char[slength];
  const unsigned char* end = src+slength;
  size_t n = 0;
  for(const unsigned char*s = src; s != end; ++s) {
    if(INDEX_TABLE[*s] != -1 || *s == '=') {
      *(temp+n++) = *s;
    }
  }
  nlength = n;
  nsrc = temp;
}

void Base64::decode(unsigned char*& result, size_t& rlength,
		    const unsigned char* src, size_t slength)
{
  if(slength == 0) {
    rlength = 0;
    return;
  }
  unsigned char* nsrc;
  size_t nlength;
  removeNonBase64Chars(nsrc, nlength, src, slength);

  if(nlength%4 != 0) {
    delete [] nsrc;
    rlength = 0;
    return;
  }

  size_t quadtuple = nlength/4;
  size_t len;
  if(nsrc[nlength-1] == '=') {
    if(nsrc[nlength-2] == '=') {
      len = nlength-2;
    } else {
      len = nlength-1;
    }
  } else {
    len = nlength;
  }
  rlength = len-quadtuple;
  result = new unsigned char[rlength];
  int r = len%4;

  unsigned char* p = result;
  const unsigned char* s = nsrc;
  const unsigned char* smax = s+len-r;
  while(s != smax) {
    int n = INDEX_TABLE[*s++] << 18;
    n += INDEX_TABLE[*s++] << 12;
    n += INDEX_TABLE[*s++] << 6;
    n += INDEX_TABLE[*s++];
    *p++ = n >> 16;
    *p++ = n >> 8&0xff;
    *p++ = n&0xff;
  }
  if(r == 2) {
    int n = INDEX_TABLE[*s++] << 18;
    n += INDEX_TABLE[*s++] << 12;
    *p++ = n >> 16;
  } else if(r == 3) {
    int n = INDEX_TABLE[*s++] << 18;
    n += INDEX_TABLE[*s++] << 12;
    n += INDEX_TABLE[*s++] << 6;
    *p++ = n >> 16;
    *p++ = n >> 8&0xff;
  }
  delete [] nsrc;
}

string Base64::encode(const string& s)
{
  unsigned char* buf = 0;
  size_t len;
  encode(buf, len, s.c_str(), s.size());
  string r(&buf[0], &buf[len]);
  delete [] buf;
  return r;
}

string Base64::decode(const string& s)
{
  unsigned char* buf = 0;
  size_t len;
  decode(buf, len, s.c_str(), s.size());
  string r(&buf[0], &buf[len]);
  delete [] buf;
  return r;
}
