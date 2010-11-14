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
#include "LibsslARC4Context.h"

#include <openssl/err.h>

#include "DlAbortEx.h"
#include "StringFormat.h"

namespace aria2 {

namespace {
void handleError()
{
  throw DL_ABORT_EX
    (StringFormat("Exception in libssl routine(ARC4Context class): %s",
                  ERR_error_string(ERR_get_error(), 0)).str());
}
} // namespace

LibsslARC4Context::LibsslARC4Context():cipherCtx_(0) {}

LibsslARC4Context::~LibsslARC4Context()
{
  if(cipherCtx_) {
    EVP_CIPHER_CTX_cleanup(cipherCtx_);
  }
  delete cipherCtx_;
}

EVP_CIPHER_CTX* LibsslARC4Context::getCipherContext() const
{
  return cipherCtx_;
}

// enc == 1: encryption
// enc == 0: decryption
void LibsslARC4Context::init
(const unsigned char* key, size_t keyLength, int enc)
{
  if(cipherCtx_) {
    EVP_CIPHER_CTX_cleanup(cipherCtx_);
  }
  delete cipherCtx_;
  cipherCtx_ = new EVP_CIPHER_CTX;
  EVP_CIPHER_CTX_init(cipherCtx_);

  if(!EVP_CipherInit_ex(cipherCtx_, EVP_rc4(), 0, 0, 0, enc)) {
    handleError();
  }
  if(!EVP_CIPHER_CTX_set_key_length(cipherCtx_, keyLength)) {
    handleError();
  }
  if(!EVP_CipherInit_ex(cipherCtx_, 0, 0, key, 0, -1)) {
    handleError();
  }
}

} // namespace aria2
