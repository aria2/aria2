/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Nils Maier
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

#include "InternalDHKeyExchange.h"

#include <cstring>

#include "DlAbortEx.h"
#include "LogFactory.h"
#include "fmt.h"
#include "util.h"

namespace aria2 {

void DHKeyExchange::init(const unsigned char* prime, size_t primeBits,
                         const unsigned char* generator, size_t privateKeyBits)
{
  std::string pr = reinterpret_cast<const char*>(prime);
  if (pr.length() % 2) {
    pr = "0" + pr;
  }
  pr = util::fromHex(pr.begin(), pr.end());
  if (pr.empty()) {
    throw DL_ABORT_EX("No valid prime supplied");
  }
  prime_ = n(pr.c_str(), pr.length());

  std::string gen = reinterpret_cast<const char*>(generator);
  if (gen.length() % 2) {
    gen = "0" + gen;
  }
  gen = util::fromHex(gen.begin(), gen.end());
  if (gen.empty()) {
    throw DL_ABORT_EX("No valid generator supplied");
  }
  generator_ = n(gen.c_str(), gen.length());

  size_t pbytes = (privateKeyBits + 7) / 8;
  unsigned char buf[pbytes];
  util::generateRandomData(buf, pbytes);
  privateKey_ = n(reinterpret_cast<char*>(buf), pbytes);

  keyLength_ = (primeBits + 7) / 8;
}

void DHKeyExchange::generatePublicKey()
{
  publicKey_ = generator_.mul_mod(privateKey_, prime_);
}

size_t DHKeyExchange::getPublicKey(unsigned char* out, size_t outLength) const
{
  if (outLength < keyLength_) {
    throw DL_ABORT_EX(
        fmt("Insufficient buffer for public key. expect:%lu, actual:%lu",
            static_cast<unsigned long>(keyLength_),
            static_cast<unsigned long>(outLength)));
  }
  publicKey_.binary(reinterpret_cast<char*>(out), outLength);
  return keyLength_;
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
  if (prime_.length() < peerPublicKeyLength) {
    throw DL_ABORT_EX(
        fmt("peer public key overflows bignum. max:%lu, actual:%lu",
            static_cast<unsigned long>(prime_.length()),
            static_cast<unsigned long>(peerPublicKeyLength)));
  }

  n peerKey(reinterpret_cast<const char*>(peerPublicKeyData),
            peerPublicKeyLength);
  n secret = peerKey.mul_mod(privateKey_, prime_);
  secret.binary(reinterpret_cast<char*>(out), outLength);

  return outLength;
}

} // namespace aria2
