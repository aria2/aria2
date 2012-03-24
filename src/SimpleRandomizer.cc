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

#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>

#include "a2time.h"

namespace aria2 {

SharedHandle<SimpleRandomizer> SimpleRandomizer::randomizer_;

const SharedHandle<SimpleRandomizer>& SimpleRandomizer::getInstance()
{
  if(!randomizer_) {
    randomizer_.reset(new SimpleRandomizer());
  }
  return randomizer_;
}
  
void SimpleRandomizer::init()
{
#ifndef __MINGW32__
  srandom(time(0)^getpid());
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

long int SimpleRandomizer::getRandomNumber()
{
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
  return val;
#else // !__MINGW32__
  return random();
#endif // !__MINGW32__
}

long int SimpleRandomizer::getMaxRandomNumber()
{
#ifdef __MINGW32__
  return INT32_MAX;
#else // !__MINGW32__
  // TODO Warning: The maximum value of random() in some sytems (e.g.,
  // Solaris and openbsd) is (2**31)-1.
  return RAND_MAX;
#endif // !__MINGW32__
}

long int SimpleRandomizer::getRandomNumber(long int to)
{
  return getRandomNumber() % to;
}

long int SimpleRandomizer::operator()(long int to)
{
  return getRandomNumber(to);
}

} // namespace aria2
