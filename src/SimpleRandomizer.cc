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
#include "SimpleRandomizer.h"

#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <iostream>

#ifdef __APPLE__
#  include <Security/SecRandom.h>
#endif // __APPLE__

#ifdef HAVE_LIBGNUTLS
#  include <gnutls/crypto.h>
#endif // HAVE_LIBGNUTLS

#ifdef HAVE_OPENSSL
#  include <openssl/rand.h>
#endif // HAVE_OPENSSL

#include "a2time.h"
#include "a2functional.h"
#include "LogFactory.h"
#include "fmt.h"

namespace aria2 {

std::unique_ptr<SimpleRandomizer> SimpleRandomizer::randomizer_;

const std::unique_ptr<SimpleRandomizer>& SimpleRandomizer::getInstance()
{
  if (!randomizer_) {
    randomizer_.reset(new SimpleRandomizer());
  }
  return randomizer_;
}

namespace {
std::random_device rd;
} // namespace

#ifdef __MINGW32__
SimpleRandomizer::SimpleRandomizer()
{
  BOOL r = ::CryptAcquireContext(&provider_, 0, 0, PROV_RSA_FULL,
                                 CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
  assert(r);
}
#else  // !__MINGW32__
SimpleRandomizer::SimpleRandomizer() : gen_(rd()) {}
#endif // !__MINGW32__

SimpleRandomizer::~SimpleRandomizer()
{
#ifdef __MINGW32__
  CryptReleaseContext(provider_, 0);
#endif
}

long int SimpleRandomizer::getRandomNumber(long int to)
{
  assert(to > 0);
  return std::uniform_int_distribution<long int>(0, to - 1)(*this);
}

void SimpleRandomizer::getRandomBytes(unsigned char* buf, size_t len)
{
#ifdef __MINGW32__
  BOOL r = CryptGenRandom(provider_, len, reinterpret_cast<BYTE*>(buf));
  if (!r) {
    assert(r);
    abort();
  }
#elif defined(__APPLE__)
  auto rv = SecRandomCopyBytes(kSecRandomDefault, len, buf);
  assert(errSecSuccess == rv);
#elif defined(HAVE_LIBGNUTLS)
  auto rv = gnutls_rnd(GNUTLS_RND_RANDOM, buf, len);
  if (rv != 0) {
    assert(0 == rv);
    abort();
  }
#elif defined(HAVE_OPENSSL)
  auto rv = RAND_bytes(buf, len);
  if (rv != 1) {
    assert(1 == rv);
    abort();
  }
#else
  constexpr static size_t blocklen = 256;
  auto iter = len / blocklen;
  auto p = buf;

  for (size_t i = 0; i < iter; ++i) {
    auto rv = getentropy(p, blocklen);
    if (rv != 0) {
      std::cerr << "getentropy: " << strerror(errno) << std::endl;
      assert(0);
      abort();
    }

    p += blocklen;
  }

  auto rem = len - iter * blocklen;
  if (rem == 0) {
    return;
  }

  auto rv = getentropy(p, rem);
  if (rv != 0) {
    std::cerr << "getentropy: " << strerror(errno) << std::endl;
    assert(0);
    abort();
  }
#endif // !__MINGW32__ && !__APPLE__ && !HAVE_OPENSSL && !HAVE_LIBGNUTLS
}

} // namespace aria2
