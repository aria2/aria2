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
#include "MessageDigestImpl.h"
#include "util.h"
#include "array_fun.h"

namespace aria2 {

namespace {
struct HashTypeEntry {
  std::string hashType;
  int strength;
  HashTypeEntry(const std::string& hashType, int strength):
    hashType(hashType), strength(strength) {}
};
} // namespace

namespace {
HashTypeEntry hashTypes[] = {
  HashTypeEntry("sha-1", 1),
  HashTypeEntry("sha-224", 2),
  HashTypeEntry("sha-256", 3),
  HashTypeEntry("sha-384", 4),
  HashTypeEntry("sha-512", 5),
  HashTypeEntry("md5", 0)
};
} // namespace aria2

MessageDigest::MessageDigest()
{}

MessageDigest::~MessageDigest()
{}

SharedHandle<MessageDigest> MessageDigest::sha1()
{
  SharedHandle<MessageDigest> md(new MessageDigest());
  md->pImpl_ = MessageDigestImpl::sha1();
  return md;
}

SharedHandle<MessageDigest> MessageDigest::create(const std::string& hashType)
{
  SharedHandle<MessageDigest> md(new MessageDigest());
  md->pImpl_ = MessageDigestImpl::create(hashType);
  return md;
}

bool MessageDigest::supports(const std::string& hashType)
{
  return MessageDigestImpl::supports(hashType);
}

std::string MessageDigest::getSupportedHashTypeString()
{
  std::string s;
  for(HashTypeEntry* i = vbegin(hashTypes), *eoi = vend(hashTypes); i != eoi;
      ++i) {
    if(MessageDigestImpl::supports(i->hashType)) {
      if(!s.empty()) {
        s += ", ";
      }
      s += i->hashType;
    }
  }
  return s;
}

size_t MessageDigest::getDigestLength(const std::string& hashType)
{
  return MessageDigestImpl::getDigestLength(hashType);
}

namespace {
class FindHashTypeEntry {
private:
  const std::string& hashType_;
public:
  FindHashTypeEntry(const std::string& hashType):hashType_(hashType)
  {}

  bool operator()(const HashTypeEntry& entry) const
  {
    return hashType_ == entry.hashType;
  }
};
} // namespace

bool MessageDigest::isStronger(const std::string& lhs, const std::string& rhs)
{
  HashTypeEntry* lEntry = std::find_if(vbegin(hashTypes), vend(hashTypes),
                                       FindHashTypeEntry(lhs));
  HashTypeEntry* rEntry = std::find_if(vbegin(hashTypes), vend(hashTypes),
                                       FindHashTypeEntry(rhs));
  if(lEntry == vend(hashTypes) || rEntry == vend(hashTypes)) {
    return false;
  }
  return lEntry->strength > rEntry->strength;
}

bool MessageDigest::isValidHash
(const std::string& hashType, const std::string& hexDigest)
{
  return util::isHexDigit(hexDigest) &&
    supports(hashType) && getDigestLength(hashType)*2 == hexDigest.size();
}

std::string MessageDigest::getCanonicalHashType(const std::string& hashType)
{
  // This is really backward compatibility for Metalink3.  aria2 only
  // supported sha-1, sha-256 and md5 at Metalink3 era.  So we don't
  // add alias for sha-224, sha-384 and sha-512.
  if("sha1" == hashType) {
    return "sha-1";
  } else if("sha256" == hashType) {
    return "sha-256";
  } else {
    return hashType;
  }
}

size_t MessageDigest::getDigestLength() const
{
  return pImpl_->getDigestLength();
}

void MessageDigest::reset()
{
  pImpl_->reset();
}

void MessageDigest::update(const void* data, size_t length)
{
  pImpl_->update(data, length);
}

void MessageDigest::digest(unsigned char* md)
{
  pImpl_->digest(md);
}

std::string MessageDigest::digest()
{
  size_t length = pImpl_->getDigestLength();
  array_ptr<unsigned char> buf(new unsigned char[length]);
  pImpl_->digest(buf);
  std::string hd(&buf[0], &buf[length]);
  return hd;
}

} // namespace aria2
