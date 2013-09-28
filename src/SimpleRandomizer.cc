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

#include "a2time.h"

namespace aria2 {

std::unique_ptr<SimpleRandomizer> SimpleRandomizer::randomizer_;

const std::unique_ptr<SimpleRandomizer>& SimpleRandomizer::getInstance()
{
  if(!randomizer_) {
    randomizer_.reset(new SimpleRandomizer());
  }
  return randomizer_;
}

void SimpleRandomizer::init()
{
#ifndef __MINGW32__
  // Just in case std::random_device() is fixed, add time and pid too.
  eng_.seed(std::random_device()()^time(nullptr)^getpid());
#endif // !__MINGW32__
}

SimpleRandomizer::SimpleRandomizer()
{
#ifdef __MINGW32__
  BOOL r = CryptAcquireContext(&cryProvider_, 0, 0, PROV_RSA_FULL,
                               CRYPT_VERIFYCONTEXT|CRYPT_SILENT);
  assert(r);
#endif // __MINGW32__
}

SimpleRandomizer::~SimpleRandomizer()
{
#ifdef __MINGW32__
  CryptReleaseContext(cryProvider_, 0);
#endif // __MINGW32__
}

long int SimpleRandomizer::getRandomNumber(long int to)
{
  assert(to > 0);
#ifdef __MINGW32__
  int32_t val;
  BOOL r = CryptGenRandom(cryProvider_, sizeof(val),
                          reinterpret_cast<BYTE*>(&val));
  assert(r);
  if(val == INT32_MIN) {
    val = INT32_MAX;
  } else if(val < 0) {
    val = -val;
  }
  return val % to;
#else // !__MINGW32__
  return std::uniform_int_distribution<long int>(0, to - 1)(eng_);
#endif // !__MINGW32__
}

long int SimpleRandomizer::operator()(long int to)
{
  return getRandomNumber(to);
}

void SimpleRandomizer::getRandomBytes(unsigned char *buf, size_t len)
{
#ifdef __MINGW32__
  if (!CryptGenRandom(cryProvider_, len, (PBYTE)buf)) {
    throw std::bad_alloc();
  }
#else
  uint32_t val;
  size_t q = len / sizeof(val);
  size_t r = len % sizeof(val);
  auto gen = std::bind(std::uniform_int_distribution<uint32_t>
                       (0, std::numeric_limits<uint32_t>::max()),
                       eng_);
  for(; q > 0; --q) {
    val = gen();
    memcpy(buf, &val, sizeof(val));
    buf += sizeof(val);
  }
  val = gen();
  memcpy(buf, &val, r);
#endif
}

} // namespace aria2
