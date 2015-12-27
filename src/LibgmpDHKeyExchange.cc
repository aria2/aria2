/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
#include "LibgmpDHKeyExchange.h"

#include <cstring>

#include "DlAbortEx.h"
#include "fmt.h"
#include "a2gmp.h"
#include "util.h"

namespace aria2 {

namespace {
void handleError(int err)
{
  throw DL_ABORT_EX(
      fmt("Exception in libgmp routine(DHKeyExchange class): code%d", err));
}
} // namespace

DHKeyExchange::DHKeyExchange() : keyLength_(0)
{
  mpz_init(prime_);
  mpz_init(generator_);
  mpz_init(privateKey_);
  mpz_init(publicKey_);
}

DHKeyExchange::~DHKeyExchange()
{
  mpz_clear(prime_);
  mpz_clear(generator_);
  mpz_clear(privateKey_);
  mpz_clear(publicKey_);
}

void DHKeyExchange::init(const unsigned char* prime, size_t primeBits,
                         const unsigned char* generator, size_t privateKeyBits)
{
  if (mpz_set_str(prime_, reinterpret_cast<const char*>(prime), 16) == -1) {
    handleError(-1);
  }
  if (mpz_set_str(generator_, reinterpret_cast<const char*>(generator), 16) ==
      -1) {
    handleError(-1);
  }
  mpz_urandomb(privateKey_, global::gmpRandstate, privateKeyBits);
  keyLength_ = (primeBits + 7) / 8;
}

void DHKeyExchange::generatePublicKey()
{
#if HAVE_GMP_SEC
  mpz_powm_sec(publicKey_, generator_, privateKey_, prime_);
#else  // HAVE_GMP_SEC
  mpz_powm(publicKey_, generator_, privateKey_, prime_);
#endif // HAVE_GMP_SEC
}

size_t DHKeyExchange::getPublicKey(unsigned char* out, size_t outLength) const
{
  if (outLength < keyLength_) {
    throw DL_ABORT_EX(
        fmt("Insufficient buffer for public key. expect:%lu, actual:%lu",
            static_cast<unsigned long>(keyLength_),
            static_cast<unsigned long>(outLength)));
  }
  memset(out, 0, outLength);
  size_t publicKeyBytes = (mpz_sizeinbase(publicKey_, 2) + 7) / 8;
  size_t offset = keyLength_ - publicKeyBytes;
  size_t nwritten;
  mpz_export(out + offset, &nwritten, 1, 1, 1, 0, publicKey_);
  return nwritten;
}

void DHKeyExchange::generateNonce(unsigned char* out, size_t outLength) const
{
  util::generateRandomData(out, outLength);
}

size_t DHKeyExchange::computeSecret(unsigned char* out, size_t outLength,
                                    const unsigned char* peerPublicKeyData,
                                    size_t peerPublicKeyLength) const
{
  if (outLength < keyLength_) {
    throw DL_ABORT_EX(
        fmt("Insufficient buffer for secret. expect:%lu, actual:%lu",
            static_cast<unsigned long>(keyLength_),
            static_cast<unsigned long>(outLength)));
  }
  mpz_t peerPublicKey;
  mpz_init(peerPublicKey);
  mpz_import(peerPublicKey, peerPublicKeyLength, 1, 1, 1, 0, peerPublicKeyData);
  mpz_t secret;
  mpz_init(secret);

#if HAVE_GMP_SEC
  mpz_powm_sec(secret, peerPublicKey, privateKey_, prime_);
#else  // HAVE_GMP_SEC
  mpz_powm(secret, peerPublicKey, privateKey_, prime_);
#endif // HAVE_GMP_SEC

  mpz_clear(peerPublicKey);

  memset(out, 0, outLength);
  size_t secretBytes = (mpz_sizeinbase(secret, 2) + 7) / 8;
  size_t offset = keyLength_ - secretBytes;
  size_t nwritten;
  mpz_export(out + offset, &nwritten, 1, 1, 1, 0, secret);
  mpz_clear(secret);
  return nwritten;
}

} // namespace aria2
