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
#include "MessageDigest.h"

#include <sstream>
#include <iterator>

#include "MessageDigestImpl.h"
#include "util.h"
#include "array_fun.h"

namespace aria2 {

namespace {
struct HashTypeEntry {
  std::string hashType;
  int strength;
  HashTypeEntry(std::string hashType, int strength)
      : hashType(std::move(hashType)), strength(strength)
  {
  }
};
} // namespace

namespace {
HashTypeEntry hashTypes[] = {
    HashTypeEntry("sha-1", 1),   HashTypeEntry("sha-224", 2),
    HashTypeEntry("sha-256", 3), HashTypeEntry("sha-384", 4),
    HashTypeEntry("sha-512", 5), HashTypeEntry("md5", 0),
    HashTypeEntry("adler32", 0),
};
} // namespace

MessageDigest::MessageDigest(std::unique_ptr<MessageDigestImpl> impl)
    : pImpl_{std::move(impl)}
{
}

MessageDigest::~MessageDigest() = default;

std::unique_ptr<MessageDigest> MessageDigest::sha1()
{
  return make_unique<MessageDigest>(MessageDigestImpl::sha1());
}

std::unique_ptr<MessageDigest>
MessageDigest::create(const std::string& hashType)
{
  return make_unique<MessageDigest>(MessageDigestImpl::create(hashType));
}

bool MessageDigest::supports(const std::string& hashType)
{
  return MessageDigestImpl::supports(hashType);
}

std::vector<std::string> MessageDigest::getSupportedHashTypes()
{
  std::vector<std::string> rv;
  for (const auto& i : hashTypes) {
    if (MessageDigestImpl::supports(i.hashType)) {
      rv.push_back(i.hashType);
    }
  }
  return rv;
}

std::string MessageDigest::getSupportedHashTypeString()
{
  auto ht = getSupportedHashTypes();
  std::stringstream ss;
  std::copy(std::begin(ht), std::end(ht),
            std::ostream_iterator<std::string>(ss, ", "));
  auto res = ss.str();
  if (!res.empty()) {
    res.erase(ss.str().length() - 2);
  }
  return res;
}

size_t MessageDigest::getDigestLength(const std::string& hashType)
{
  return MessageDigestImpl::getDigestLength(hashType);
}

bool MessageDigest::isStronger(const std::string& lhs, const std::string& rhs)
{
  auto lEntry = std::find_if(
      std::begin(hashTypes), std::end(hashTypes),
      [&lhs](const HashTypeEntry& entry) { return lhs == entry.hashType; });
  auto rEntry = std::find_if(
      std::begin(hashTypes), std::end(hashTypes),
      [&rhs](const HashTypeEntry& entry) { return rhs == entry.hashType; });
  if (lEntry == std::end(hashTypes)) {
    return false;
  }
  if (rEntry == std::end(hashTypes)) {
    return true;
  }
  return lEntry->strength > rEntry->strength;
}

bool MessageDigest::isValidHash(const std::string& hashType,
                                const std::string& hexDigest)
{
  return util::isHexDigit(hexDigest) && supports(hashType) &&
         getDigestLength(hashType) * 2 == hexDigest.size();
}

std::string MessageDigest::getCanonicalHashType(const std::string& hashType)
{
  // This is really backward compatibility for Metalink3.  aria2 only
  // supported sha-1, sha-256 and md5 at Metalink3 era.  So we don't
  // add alias for sha-224, sha-384 and sha-512.
  if ("sha1" == hashType) {
    return "sha-1";
  }
  else if ("sha256" == hashType) {
    return "sha-256";
  }
  else {
    return hashType;
  }
}

size_t MessageDigest::getDigestLength() const
{
  return pImpl_->getDigestLength();
}

void MessageDigest::reset() { pImpl_->reset(); }

MessageDigest& MessageDigest::update(const void* data, size_t length)
{
  pImpl_->update(data, length);
  return *this;
}

void MessageDigest::digest(unsigned char* md) { pImpl_->digest(md); }

std::string MessageDigest::digest()
{
  size_t length = pImpl_->getDigestLength();
  auto buf = make_unique<unsigned char[]>(length);
  pImpl_->digest(buf.get());
  return std::string(&buf[0], &buf[length]);
}

} // namespace aria2
