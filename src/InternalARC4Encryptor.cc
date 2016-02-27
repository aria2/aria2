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

#include "InternalARC4Encryptor.h"

namespace aria2 {

void ARC4Encryptor::init(const unsigned char* key, size_t keyLength)
{
  j = 0;
  for (auto& c : state_)
    c = j++;

  j = 0;
  for (i = 0; i < sizeof(state_); ++i) {
    j = (j + state_[i] + key[i % keyLength]) & 0xff;
    auto tmp = state_[i];
    state_[i] = state_[j];
    state_[j] = tmp;
  }
  i = j = 0;
}

void ARC4Encryptor::encrypt(size_t len, unsigned char* out,
                            const unsigned char* in)
{
  for (auto c = 0u; c < len; ++c) {
    i = (i + 1) & 0xff;
    j = (j + state_[i]) & 0xff;
    auto sj = state_[i];
    auto si = state_[i] = state_[j];
    state_[j] = sj;
    out[c] = in[c] ^ state_[(si + sj) & 0xff];
  }
}

} // namespace aria2
