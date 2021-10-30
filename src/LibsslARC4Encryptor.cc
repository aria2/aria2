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
#include "LibsslARC4Encryptor.h"

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#  include <cassert>

#  include <openssl/core_names.h>

#  include "DlAbortEx.h"
#endif // OPENSSL_VERSION_NUMBER >= 0x30000000L

namespace aria2 {

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
ARC4Encryptor::ARC4Encryptor() : ctx_{nullptr} {}

ARC4Encryptor::~ARC4Encryptor()
{
  if (ctx_) {
    EVP_CIPHER_CTX_free(ctx_);
  }
}
#else  // !(OPENSSL_VERSION_NUMBER >= 0x30000000L)
ARC4Encryptor::ARC4Encryptor() {}

ARC4Encryptor::~ARC4Encryptor() = default;
#endif // !(OPENSSL_VERSION_NUMBER >= 0x30000000L)

void ARC4Encryptor::init(const unsigned char* key, size_t keyLength)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
  if (ctx_) {
    EVP_CIPHER_CTX_free(ctx_);
  }

  ctx_ = EVP_CIPHER_CTX_new();

  OSSL_PARAM params[] = {
      OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_KEYLEN, &keyLength),
      OSSL_PARAM_construct_end(),
  };

  if (EVP_EncryptInit_ex2(ctx_, EVP_rc4(), nullptr, nullptr, params) != 1) {
    throw DL_ABORT_EX("Failed to initialize RC4 cipher");
  }

  if (EVP_EncryptInit_ex2(ctx_, nullptr, key, nullptr, nullptr) != 1) {
    throw DL_ABORT_EX("Failed to set key to RC4 cipher");
  }
#else  // !(OPENSSL_VERSION_NUMBER >= 0x30000000L)
  RC4_set_key(&key_, keyLength, key);
#endif // !(OPENSSL_VERSION_NUMBER >= 0x30000000L)
}

void ARC4Encryptor::encrypt(size_t len, unsigned char* out,
                            const unsigned char* in)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
  int outlen;

  if (EVP_EncryptUpdate(ctx_, out, &outlen, in, len) != 1) {
    throw DL_ABORT_EX("Failed to encrypt data with RC4 cipher");
  }

  assert(static_cast<size_t>(outlen) == len);
#else  // !(OPENSSL_VERSION_NUMBER >= 0x30000000L)
  RC4(&key_, len, in, out);
#endif // !(OPENSSL_VERSION_NUMBER >= 0x30000000L)
}

} // namespace aria2
