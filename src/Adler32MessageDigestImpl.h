/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2014 Tatsuhiro Tsujikawa
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
#ifndef D_ADLER32_MESSAGE_DIGEST_H
#define D_ADLER32_MESSAGE_DIGEST_H

#include "MessageDigestImpl.h"

namespace aria2 {

#ifdef HAVE_ZLIB

#  define ADLER32_MESSAGE_DIGEST                                               \
    {"adler32", make_hi<Adler32MessageDigestImpl>()},

class Adler32MessageDigestImpl : public MessageDigestImpl {
public:
  Adler32MessageDigestImpl();
  virtual size_t getDigestLength() const CXX11_OVERRIDE;
  virtual void reset() CXX11_OVERRIDE;
  virtual void update(const void* data, size_t length) CXX11_OVERRIDE;
  virtual void digest(unsigned char* md) CXX11_OVERRIDE;
  static size_t length();

private:
  unsigned long adler_;
};

#else // !HAVE_ZLIB

#  define ADLER32_MESSAGE_DIGEST

#endif // !HAVE_ZLIB

} // namespace aria2

#endif // D_ADLER32_MESSAGE_DIGEST_H
