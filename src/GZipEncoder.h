/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#ifndef D_GZIP_ENCODER_H
#define D_GZIP_ENCODER_H
#include "common.h"

#include <string>

#include <zlib.h>

namespace aria2 {

class GZipEncoder {
private:
  z_stream* strm_;

  // Internal buffer for deflated data.
  std::string internalBuf_;

  std::string encode(const unsigned char* in, size_t length, int flush);
  // Not implemented
  GZipEncoder& operator<<(char c);

public:
  GZipEncoder();

  ~GZipEncoder();

  // Initializes deflater.
  void init();

  // Feeds NULL-terminated c-string s to deflater.  The deflated
  // result is kept in this class.
  GZipEncoder& operator<<(const char* s);

  // Feeds binary data in s to deflater.  The deflated result is kept
  // in this class.
  GZipEncoder& operator<<(const std::string& s);

  // Feeds binary data in s with size length to deflater.  The
  // deflated result is kept in this class.
  GZipEncoder& write(const char* s, size_t length);

  // Feeds integer to deflater. Before passed to deflater, i is
  // converted to std::string using util::itos().  The deflated result
  // is kept in this class.
  GZipEncoder& operator<<(int64_t i);

  // Feeds binary data pointed by in with size length to deflater and
  // returns compressed output available so far.  Don't use this
  // method with operator<< methods.
  std::string encode(const unsigned char* in, size_t length)
  {
    return encode(in, length, Z_NO_FLUSH);
  }

  // Releases allocated resources.
  void release();

  // Returns deflated result kept internally. After this function
  // call, further calls to operator<<() and encode() are not allowed.
  std::string str();
};

} // namespace aria2

#endif // D_GZIP_ENCODER_H
