/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2014 Nils Maier
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

#include "util_security.h"

#include "FatalException.h"
#include "util.h"

namespace {
using namespace aria2;

static inline size_t getBlockSize(const std::string& algorithm)
{
  std::string canon;
  if (!MessageDigest::supports(algorithm)) {
    goto err;
  }
  canon = MessageDigest::getCanonicalHashType(algorithm);

  // Per RFC 6234
  if (canon == "sha-1") {
    return 64;
  }
  if (canon == "sha-224") {
    return 64;
  }
  if (canon == "sha-256") {
    return 64;
  }
  if (canon == "sha-384") {
    return 128;
  }
  if (canon == "sha-512") {
    // RFC 4868
    return 128;
  }

err:
  throw FATAL_EXCEPTION(
      fmt("HMAC does not support algorithm %s", algorithm.c_str()));
}

} // namespace

namespace aria2 {
namespace util {
namespace security {

bool compare(const unsigned char a, const unsigned char b)
{
  unsigned char rv = ~(a ^ b);
  rv &= rv >> 4;
  rv &= rv >> 2;
  rv &= rv >> 1;
  return rv;
}

bool compare(const uint8_t* a, const uint8_t* b, size_t length)
{
  unsigned char rv = 0;
  for (size_t i = 0; i < length; ++i) {
    rv |= a[i] ^ b[i];
  }
  return compare(rv, 0);
}

HMAC::HMAC(const std::string& algorithm, const char* secret, size_t length)
    : blockSize_(getBlockSize(algorithm)),
      md_(MessageDigest::create(algorithm)),
      clean_(false)
{
  ipad_.assign(blockSize_, 0x36);
  opad_.assign(blockSize_, 0x5c);

  if (length > blockSize_) {
    md_->reset();
    md_->update(secret, length);
    auto hash = md_->digest();
    for (size_t i = 0uL, e = hash.length(); i < e; ++i) {
      ipad_.replace(i, 1, 1, hash[i] ^ 0x36);
      opad_.replace(i, 1, 1, hash[i] ^ 0x5c);
    }
  }
  else {
    for (size_t i = 0uL, e = length; i < e; ++i) {
      ipad_.replace(i, 1, 1, secret[i] ^ 0x36);
      opad_.replace(i, 1, 1, secret[i] ^ 0x5c);
    }
  }
  reset();
}

std::unique_ptr<HMAC> HMAC::createRandom(const std::string& algorithm)
{
  const auto len = MessageDigest::getDigestLength(algorithm);
  if (len == 0) {
    return nullptr;
  }
  auto buf = make_unique<char[]>(len);
  generateRandomData((unsigned char*)buf.get(), len);
  return create(algorithm, buf.get(), len);
}

bool HMAC::supports(const std::string& algorithm)
{
  if (!MessageDigest::supports(algorithm)) {
    return false;
  }
  const auto canon = MessageDigest::getCanonicalHashType(algorithm);
  return canon == "sha-1" || canon == "sha-224" || canon == "sha-256" ||
         canon == "sha-384" || canon == "sha-512";
}

HMACResult PBKDF2(HMAC* hmac, const char* salt, size_t salt_length,
                  size_t iterations, size_t key_length)
{
  if (!hmac) {
    throw FATAL_EXCEPTION("hmac cannot be null");
  }
  const size_t hmac_length = hmac->length();
  if (key_length == 0) {
    key_length = hmac_length;
  }

  auto work = make_unique<char[]>(hmac_length);
  char* p = work.get();
  std::string rv;

  hmac->reset();

  for (uint32_t counter = 1; key_length; ++counter) {
    hmac->update(salt, salt_length);
    const uint32_t c = htonl(counter);
    hmac->update((char*)&c, sizeof(c));

    auto bytes = hmac->getResult().getBytes();
    memcpy(p, bytes.data(), bytes.length());

    for (size_t i = 1uL; i < iterations; ++i) {
      hmac->update(bytes);
      bytes = hmac->getResult().getBytes();
      for (size_t j = 0uL; j < hmac_length; ++j) {
        p[j] ^= bytes[j];
      }
    }
    auto use = std::min(key_length, hmac_length);
    rv.append(p, use);
    key_length -= use;
  }
  return HMACResult(rv);
}

} // namespace security
} // namespace util
} // namespace aria2
