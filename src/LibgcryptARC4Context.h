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
#ifndef _D_LIBGCRYPT_ARC4_CONTEXT_H_
#define _D_LIBGCRYPT_ARC4_CONTEXT_H_

#include "common.h"

#include <gcrypt.h>

#include "DlAbortEx.h"
#include "StringFormat.h"

namespace aria2 {

class LibgcryptARC4Context {
private:
  gcry_cipher_hd_t cipherCtx_;

  void handleError(gcry_error_t err) const
  {
    throw DL_ABORT_EX
      (StringFormat("Exception in libgcrypt routine(ARC4Context class): %s",
                    gcry_strerror(err)).str());
  }
public:
  LibgcryptARC4Context():cipherCtx_(0) {}

  ~LibgcryptARC4Context()
  {
    gcry_cipher_close(cipherCtx_);
  }

  gcry_cipher_hd_t getCipherContext() const
  {
    return cipherCtx_;
  }

  void init(const unsigned char* key, size_t keyLength)
  {
    gcry_cipher_close(cipherCtx_);

    int algo = GCRY_CIPHER_ARCFOUR;
    int mode = GCRY_CIPHER_MODE_STREAM;
    unsigned int flags = 0;
    {
      gcry_error_t r = gcry_cipher_open(&cipherCtx_, algo, mode, flags);
      if(r) {
        handleError(r);
      }
    }
    {
      gcry_error_t r = gcry_cipher_setkey(cipherCtx_, key, keyLength);
      if(r) {
        handleError(r);
      }
    }
    {
      gcry_error_t r = gcry_cipher_setiv(cipherCtx_, 0, 0);
      if(r) {
        handleError(r);
      }
    }
  }
};

} // namespace aria2

#endif // _D_LIBGCRYPT_ARC4_CONTEXT_H_
