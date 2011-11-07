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
#include "LibgcryptARC4Encryptor.h"

#include <gcrypt.h>

#include "DlAbortEx.h"
#include "fmt.h"

namespace aria2 {

namespace {
void handleError(gcry_error_t err)
{
  throw DL_ABORT_EX
    (fmt("Exception in libgcrypt routine(ARC4Encryptor class): %s",
         gcry_strerror(err)));
}
} // namespace

ARC4Encryptor::ARC4Encryptor()
  : hdl_(0)
{}

ARC4Encryptor::~ARC4Encryptor()
{
  gcry_cipher_close(hdl_);
}

void ARC4Encryptor::init(const unsigned char* key, size_t keyLength)
{
  int algo = GCRY_CIPHER_ARCFOUR;
  int mode = GCRY_CIPHER_MODE_STREAM;
  unsigned int flags = 0;
  gcry_error_t r;
  if((r = gcry_cipher_open(&hdl_, algo, mode, flags))) {
    handleError(r);
  }
  if((r = gcry_cipher_setkey(hdl_, key, keyLength))) {
    handleError(r);
  }
  if((r = gcry_cipher_setiv(hdl_, 0, 0))) {
    handleError(r);
  }
}

void ARC4Encryptor::encrypt
(size_t len,
 unsigned char* out,
 const unsigned char* in)
{
  size_t inlen;
  if(in == out) {
    out = const_cast<unsigned char*>(in);
    in = 0;
    inlen = 0;
  } else {
    inlen = len;
  }
  gcry_error_t r;
  if((r = gcry_cipher_encrypt(hdl_, out, len, in, inlen))) {
    handleError(r);
  }
}

} // namespace aria2
