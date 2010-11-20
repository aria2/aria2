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
#include "LibsslMessageDigestImpl.h"

#include <algorithm>

#include "array_fun.h"
#include "HashFuncEntry.h"

namespace aria2 {

MessageDigestImpl::MessageDigestImpl(const EVP_MD* hashFunc):hashFunc_(hashFunc)
{
  EVP_MD_CTX_init(&ctx_);
  reset();
}

MessageDigestImpl::~MessageDigestImpl()
{
  EVP_MD_CTX_cleanup(&ctx_);
}

SharedHandle<MessageDigestImpl> MessageDigestImpl::sha1()
{
  return SharedHandle<MessageDigestImpl>(new MessageDigestImpl(EVP_sha1()));
}

typedef HashFuncEntry<const EVP_MD*> CHashFuncEntry;
typedef FindHashFunc<const EVP_MD*> CFindHashFunc;

namespace {
CHashFuncEntry hashFuncs[] = {
  CHashFuncEntry("sha-1", EVP_sha1()),
#ifdef HAVE_EVP_SHA224
  CHashFuncEntry("sha-224", EVP_sha224()),
#endif // HAVE_EVP_SHA224
#ifdef HAVE_EVP_SHA256
  CHashFuncEntry("sha-256", EVP_sha256()),
#endif // HAVE_EVP_SHA256
#ifdef HAVE_EVP_SHA384
  CHashFuncEntry("sha-384", EVP_sha384()),
#endif // HAVE_EVP_SHA384
#ifdef HAVE_EVP_SHA512
  CHashFuncEntry("sha-512", EVP_sha512()),
#endif // HAVE_EVP_SHA512
  CHashFuncEntry("md5", EVP_md5())
};
} // namespace

SharedHandle<MessageDigestImpl> MessageDigestImpl::create
(const std::string& hashType)
{
  const EVP_MD* hashFunc = getHashFunc(vbegin(hashFuncs), vend(hashFuncs),
                                       hashType);
  return SharedHandle<MessageDigestImpl>(new MessageDigestImpl(hashFunc));
}

bool MessageDigestImpl::supports(const std::string& hashType)
{
  return vend(hashFuncs) != std::find_if(vbegin(hashFuncs), vend(hashFuncs),
                                         CFindHashFunc(hashType));
}

size_t MessageDigestImpl::getDigestLength(const std::string& hashType)
{
  const EVP_MD* hashFunc = getHashFunc(vbegin(hashFuncs), vend(hashFuncs),
                                       hashType);
  return EVP_MD_size(hashFunc);
}

size_t MessageDigestImpl::getDigestLength() const
{
  return EVP_MD_size(hashFunc_);
}

void MessageDigestImpl::reset()
{
  EVP_DigestInit_ex(&ctx_, hashFunc_, 0);
}
void MessageDigestImpl::update(const void* data, size_t length)
{
  EVP_DigestUpdate(&ctx_, data, length);
}

void MessageDigestImpl::digest(unsigned char* md)
{
  unsigned int len;
  EVP_DigestFinal_ex(&ctx_, md, &len);
}

} // namespace aria2
