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
#include "LibsslDHKeyExchange.h"

#include <cstring>

#include <openssl/rand.h>
#include <openssl/err.h>

#include "DlAbortEx.h"
#include "fmt.h"

namespace aria2 {

namespace {
void handleError(const std::string& funName)
{
  throw DL_ABORT_EX(
      fmt("Exception in libssl routine %s(DHKeyExchange class): %s",
          funName.c_str(), ERR_error_string(ERR_get_error(), nullptr)));
}
} // namespace

DHKeyExchange::DHKeyExchange()
    : bnCtx_(nullptr),
      keyLength_(0),
      prime_(nullptr),
      generator_(nullptr),
      privateKey_(nullptr),
      publicKey_(nullptr)
{
}

DHKeyExchange::~DHKeyExchange()
{
  BN_CTX_free(bnCtx_);
  BN_free(prime_);
  BN_free(generator_);
  BN_free(privateKey_);
  BN_free(publicKey_);
}

void DHKeyExchange::init(const unsigned char* prime, size_t primeBits,
                         const unsigned char* generator, size_t privateKeyBits)
{
  BN_CTX_free(bnCtx_);
  bnCtx_ = BN_CTX_new();
  if (!bnCtx_) {
    handleError("BN_CTX_new in init");
  }

  BN_free(prime_);
  prime_ = nullptr;
  BN_free(generator_);
  generator_ = nullptr;
  BN_free(privateKey_);
  privateKey_ = nullptr;

  if (BN_hex2bn(&prime_, reinterpret_cast<const char*>(prime)) == 0) {
    handleError("BN_hex2bn in init");
  }
  if (BN_hex2bn(&generator_, reinterpret_cast<const char*>(generator)) == 0) {
    handleError("BN_hex2bn in init");
  }
  privateKey_ = BN_new();
  if (BN_rand(privateKey_, privateKeyBits, -1, false) == 0) {
    handleError("BN_new in init");
  }
  keyLength_ = (primeBits + 7) / 8;
}

void DHKeyExchange::generatePublicKey()
{
  BN_free(publicKey_);
  publicKey_ = BN_new();
  BN_mod_exp(publicKey_, generator_, privateKey_, prime_, bnCtx_);
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
  size_t publicKeyBytes = BN_num_bytes(publicKey_);
  size_t offset = keyLength_ - publicKeyBytes;
  size_t nwritten = BN_bn2bin(publicKey_, out + offset);
  if (nwritten != publicKeyBytes) {
    throw DL_ABORT_EX(
        fmt("BN_bn2bin in DHKeyExchange::getPublicKey, %lu bytes written,"
            " but %lu bytes expected.",
            static_cast<unsigned long>(nwritten),
            static_cast<unsigned long>(publicKeyBytes)));
  }
  return nwritten;
}

void DHKeyExchange::generateNonce(unsigned char* out, size_t outLength) const
{
  if (RAND_bytes(out, outLength) != 1) {
    handleError("RAND_bytes in generateNonce");
  }
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

  BIGNUM* peerPublicKey =
      BN_bin2bn(peerPublicKeyData, peerPublicKeyLength, nullptr);
  if (!peerPublicKey) {
    handleError("BN_bin2bn in computeSecret");
  }

  BIGNUM* secret = BN_new();
  BN_mod_exp(secret, peerPublicKey, privateKey_, prime_, bnCtx_);
  BN_free(peerPublicKey);

  memset(out, 0, outLength);
  size_t secretBytes = BN_num_bytes(secret);
  size_t offset = keyLength_ - secretBytes;
  size_t nwritten = BN_bn2bin(secret, out + offset);
  BN_free(secret);
  if (nwritten != secretBytes) {
    throw DL_ABORT_EX(
        fmt("BN_bn2bin in DHKeyExchange::getPublicKey, %lu bytes written,"
            " but %lu bytes expected.",
            static_cast<unsigned long>(nwritten),
            static_cast<unsigned long>(secretBytes)));
  }
  return nwritten;
}

} // namespace aria2
