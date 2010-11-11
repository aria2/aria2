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
#include "LibgcryptMessageDigestImpl.h"

#include <algorithm>

#include "array_fun.h"
#include "HashFuncEntry.h"

namespace aria2 {

MessageDigestImpl::MessageDigestImpl(int hashFunc):hashFunc_(hashFunc)
{
  gcry_md_open(&ctx_, hashFunc_, 0);
}

MessageDigestImpl::~MessageDigestImpl()
{
  gcry_md_close(ctx_);
}

SharedHandle<MessageDigestImpl> MessageDigestImpl::sha1()
{
  return SharedHandle<MessageDigestImpl>(new MessageDigestImpl(GCRY_MD_SHA1));
}

typedef HashFuncEntry<int> CHashFuncEntry;
typedef FindHashFunc<int> CFindHashFunc;

namespace {
CHashFuncEntry hashFuncs[] = {
  CHashFuncEntry("sha-1", GCRY_MD_SHA1),
  CHashFuncEntry("sha-224", GCRY_MD_SHA224),
  CHashFuncEntry("sha-256", GCRY_MD_SHA256),
  CHashFuncEntry("sha-384", GCRY_MD_SHA384),
  CHashFuncEntry("sha-512", GCRY_MD_SHA512),
  CHashFuncEntry("md5", GCRY_MD_MD5)
};
} // namespace

SharedHandle<MessageDigestImpl> MessageDigestImpl::create
(const std::string& hashType)
{
  int hashFunc = getHashFunc(vbegin(hashFuncs), vend(hashFuncs), hashType);
  return SharedHandle<MessageDigestImpl>(new MessageDigestImpl(hashFunc));
}

bool MessageDigestImpl::supports(const std::string& hashType)
{
  return vend(hashFuncs) != std::find_if(vbegin(hashFuncs), vend(hashFuncs),
                                         CFindHashFunc(hashType));
}

size_t MessageDigestImpl::getDigestLength(const std::string& hashType)
{
  int hashFunc = getHashFunc(vbegin(hashFuncs), vend(hashFuncs), hashType);
  return gcry_md_get_algo_dlen(hashFunc);
}

size_t MessageDigestImpl::getDigestLength() const
{
  return gcry_md_get_algo_dlen(hashFunc_);
}

void MessageDigestImpl::reset()
{
  gcry_md_reset(ctx_);
}
void MessageDigestImpl::update(const void* data, size_t length)
{
  gcry_md_write(ctx_, data, length);
}

void MessageDigestImpl::digest(unsigned char* md)
{
  memcpy(md, gcry_md_read(ctx_, 0), gcry_md_get_algo_dlen(hashFunc_));
}

} // namespace aria2
