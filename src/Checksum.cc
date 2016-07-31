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
#include "Checksum.h"
#include "MessageDigest.h"

namespace aria2 {

Checksum::Checksum(std::string hashType, std::string digest)
    : hashType_(std::move(hashType)), digest_(std::move(digest))
{
}

Checksum::Checksum() : hashType_("sha-1") {}

Checksum::~Checksum() = default;

bool Checksum::isEmpty() const { return digest_.empty(); }

void Checksum::setDigest(std::string digest) { digest_ = std::move(digest); }

void Checksum::setHashType(std::string hashType)
{
  hashType_ = std::move(hashType);
}

void Checksum::swap(Checksum& other)
{
  using std::swap;
  if (this != &other) {
    swap(hashType_, other.hashType_);
    swap(digest_, other.digest_);
  }
}

void swap(Checksum& a, Checksum& b) { a.swap(b); }

bool HashTypeStronger::operator()(const Checksum& lhs,
                                  const Checksum& rhs) const
{
  return MessageDigest::isStronger(lhs.getHashType(), rhs.getHashType());
}

} // namespace aria2
