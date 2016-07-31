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

#include "MessageDigestImpl.h"

#include "crypto_hash.h"

#include "Adler32MessageDigestImpl.h"

namespace {
using namespace aria2;
using namespace crypto;

template <hash::Algorithms algo>
class MessageDigestBase : public MessageDigestImpl {
public:
  MessageDigestBase() : ctx_{hash::create(algo)} {}
  virtual ~MessageDigestBase() = default;

  static size_t length()
  {
    auto ctx = hash::create(algo);
    return ctx->length();
  }

  virtual size_t getDigestLength() const CXX11_OVERRIDE
  {
    return ctx_->length();
  }

  virtual void reset() CXX11_OVERRIDE { ctx_->reset(); }

  virtual void update(const void* data, size_t length) CXX11_OVERRIDE
  {
    ctx_->update(data, length);
  }

  virtual void digest(unsigned char* md) CXX11_OVERRIDE
  {
    auto rv = ctx_->finalize();
    memcpy(md, rv.data(), rv.length());
  }

private:
  std::unique_ptr<hash::Algorithm> ctx_;
};

typedef MessageDigestBase<hash::algoMD5> MessageDigestMD5;
typedef MessageDigestBase<hash::algoSHA1> MessageDigestSHA1;
typedef MessageDigestBase<hash::algoSHA224> MessageDigestSHA224;
typedef MessageDigestBase<hash::algoSHA256> MessageDigestSHA256;
typedef MessageDigestBase<hash::algoSHA384> MessageDigestSHA384;
typedef MessageDigestBase<hash::algoSHA512> MessageDigestSHA512;

} // namespace

namespace aria2 {

std::unique_ptr<MessageDigestImpl> MessageDigestImpl::sha1()
{
  return make_unique<MessageDigestSHA1>();
}

MessageDigestImpl::hashes_t MessageDigestImpl::hashes = {
    {"sha-1", make_hi<MessageDigestSHA1>()},
    {"sha-224", make_hi<MessageDigestSHA224>()},
    {"sha-256", make_hi<MessageDigestSHA256>()},
    {"sha-384", make_hi<MessageDigestSHA384>()},
    {"sha-512", make_hi<MessageDigestSHA512>()},
    {"md5", make_hi<MessageDigestMD5>()},
    ADLER32_MESSAGE_DIGEST};

} // namespace aria2
