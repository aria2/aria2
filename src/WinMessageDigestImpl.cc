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

#include "WinMessageDigestImpl.h"

#include <wincrypt.h>

#include "array_fun.h"
#include "a2functional.h"
#include "HashFuncEntry.h"
#include "DlAbortEx.h"

namespace {
using namespace aria2;

class Context {
private:
  HCRYPTPROV provider_;
public:
  Context() {
    if (!::CryptAcquireContext(&provider_, nullptr, nullptr, PROV_RSA_FULL,
                               CRYPT_VERIFYCONTEXT)) {
      throw DL_ABORT_EX("Failed to get cryptographic provider");
    }
  }
  ~Context() {
    ::CryptReleaseContext(provider_, 0);
  }

  HCRYPTPROV get() {
    return provider_;
  }
};

// XXX static OK?
static Context context_;

} // namespace

namespace aria2 {

template<ALG_ID id>
class MessageDigestBase : public MessageDigestImpl {
private:
  HCRYPTHASH hash_;
  DWORD len_;

  void destroy() {
    if (hash_) {
      ::CryptDestroyHash(hash_);
      hash_ = 0;
    }
  }

public:
  MessageDigestBase() : hash_(0), len_(0) { reset(); }
  virtual ~MessageDigestBase() { destroy(); }

  virtual size_t getDigestLength() const CXX11_OVERRIDE {
    return len_;
  }
  virtual void reset() CXX11_OVERRIDE {
    destroy();
    if (!::CryptCreateHash(context_.get(), id, 0, 0, &hash_)) {
      throw DL_ABORT_EX("Failed to create hash");
    }

    DWORD len = sizeof(len_);
    if (!::CryptGetHashParam(hash_, HP_HASHSIZE, reinterpret_cast<BYTE*>(&len_),
                             &len, 0)) {
      throw DL_ABORT_EX("Failed to create hash");
    }
  }
  virtual void update(const void* data, size_t length) CXX11_OVERRIDE {
    auto bytes = reinterpret_cast<const unsigned char*>(data);
    while (length) {
      DWORD l = std::min(length, (size_t)std::numeric_limits<uint32_t>::max());
      if (!::CryptHashData(hash_, bytes, l, 0)) {
        throw DL_ABORT_EX("Failed to update hash");
      }
      length -= l;
      bytes += l;
    }
  }
  virtual void digest(unsigned char* md) CXX11_OVERRIDE {
    DWORD len = len_;
    if (!::CryptGetHashParam(hash_, HP_HASHVAL, md, &len, 0)) {
      throw DL_ABORT_EX("Failed to create hash digest");
    }
  }
};

typedef MessageDigestBase<CALG_MD5> MessageDigestMD5;
typedef MessageDigestBase<CALG_SHA1> MessageDigestSHA1;
typedef MessageDigestBase<CALG_SHA_256> MessageDigestSHA256;
typedef MessageDigestBase<CALG_SHA_384> MessageDigestSHA384;
typedef MessageDigestBase<CALG_SHA_512> MessageDigestSHA512;

std::unique_ptr<MessageDigestImpl> MessageDigestImpl::sha1()
{
  return std::unique_ptr<MessageDigestImpl>(new MessageDigestSHA1());
}

std::unique_ptr<MessageDigestImpl> MessageDigestImpl::create(
    const std::string& hashType)
{
  if (hashType == "sha-1") {
    return make_unique<MessageDigestSHA1>();
  }
  if (hashType == "sha-256") {
    return make_unique<MessageDigestSHA256>();
  }
  if (hashType == "sha-384") {
    return make_unique<MessageDigestSHA384>();
  }
  if (hashType == "sha-512") {
    return make_unique<MessageDigestSHA512>();
  }
  if (hashType == "md5") {
    return make_unique<MessageDigestMD5>();
  }
  return nullptr;
}

bool MessageDigestImpl::supports(const std::string& hashType)
{
  try {
    return !!create(hashType);
  }
  catch (RecoverableException& ex) {
    // no op
  }
  return false;
}

size_t MessageDigestImpl::getDigestLength(const std::string& hashType)
{
  std::unique_ptr<MessageDigestImpl> impl = create(hashType);
  if (!impl) {
    return 0;
  }
  return impl->getDigestLength();
}

} // namespace aria2
