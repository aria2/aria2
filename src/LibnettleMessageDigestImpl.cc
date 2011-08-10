/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
#include "LibnettleMessageDigestImpl.h"

#include <algorithm>

#include "array_fun.h"
#include "HashFuncEntry.h"

namespace aria2 {

MessageDigestImpl::MessageDigestImpl(const nettle_hash* hashInfo)
  : hashInfo_(hashInfo),
    ctx_(new char[hashInfo->context_size])
{
  reset();
}

MessageDigestImpl::~MessageDigestImpl()
{
  delete [] ctx_;
}

SharedHandle<MessageDigestImpl> MessageDigestImpl::sha1()
{
  return SharedHandle<MessageDigestImpl>(new MessageDigestImpl(&nettle_sha1));
}

typedef HashFuncEntry<const nettle_hash*> CHashFuncEntry;
typedef FindHashFunc<const nettle_hash*> CFindHashFunc;

namespace {
CHashFuncEntry hashFuncs[] = {
  CHashFuncEntry("sha-1", &nettle_sha1),
  CHashFuncEntry("sha-224", &nettle_sha224),
  CHashFuncEntry("sha-256", &nettle_sha256),
  CHashFuncEntry("sha-384", &nettle_sha384),
  CHashFuncEntry("sha-512", &nettle_sha512),
  CHashFuncEntry("md5", &nettle_md5)
};
} // namespace

SharedHandle<MessageDigestImpl> MessageDigestImpl::create
(const std::string& hashType)
{
  const nettle_hash* hashInfo =
    getHashFunc(vbegin(hashFuncs), vend(hashFuncs), hashType);
  return SharedHandle<MessageDigestImpl>(new MessageDigestImpl(hashInfo));
}

bool MessageDigestImpl::supports(const std::string& hashType)
{
  return vend(hashFuncs) != std::find_if(vbegin(hashFuncs), vend(hashFuncs),
                                         CFindHashFunc(hashType));
}

size_t MessageDigestImpl::getDigestLength(const std::string& hashType)
{
  const nettle_hash* hashInfo =
    getHashFunc(vbegin(hashFuncs), vend(hashFuncs), hashType);
  return hashInfo->digest_size;
}

size_t MessageDigestImpl::getDigestLength() const
{
  return hashInfo_->digest_size;
}

void MessageDigestImpl::reset()
{
  hashInfo_->init(ctx_);
}
void MessageDigestImpl::update(const void* data, size_t length)
{
  hashInfo_->update(ctx_, length, static_cast<const uint8_t*>(data));
}

void MessageDigestImpl::digest(unsigned char* md)
{
  hashInfo_->digest(ctx_, getDigestLength(), md);
}

} // namespace aria2
