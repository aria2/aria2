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

#include <CommonCrypto/CommonDigest.h>

#include "Adler32MessageDigestImpl.h"

namespace aria2 {
namespace {

template <size_t dlen, typename ctx_t, int (*init_fn)(ctx_t*),
          int (*update_fn)(ctx_t*, const void*, CC_LONG),
          int (*final_fn)(unsigned char*, ctx_t*)>
class MessageDigestBase : public MessageDigestImpl {
public:
  MessageDigestBase() { reset(); }
  virtual ~MessageDigestBase() = default;

  static size_t length() { return dlen; }
  virtual size_t getDigestLength() const CXX11_OVERRIDE { return dlen; }
  virtual void reset() CXX11_OVERRIDE { init_fn(&ctx_); }
  virtual void update(const void* data, size_t length) CXX11_OVERRIDE
  {
    auto bytes = reinterpret_cast<const char*>(data);
    while (length) {
      CC_LONG l =
          std::min(length, (size_t)std::numeric_limits<uint32_t>::max());
      update_fn(&ctx_, bytes, l);
      length -= l;
      bytes += l;
    }
  }
  virtual void digest(unsigned char* md) CXX11_OVERRIDE { final_fn(md, &ctx_); }

private:
  ctx_t ctx_;
};

typedef MessageDigestBase<CC_MD5_DIGEST_LENGTH, CC_MD5_CTX, CC_MD5_Init,
                          CC_MD5_Update, CC_MD5_Final>
    MessageDigestMD5;
typedef MessageDigestBase<CC_SHA1_DIGEST_LENGTH, CC_SHA1_CTX, CC_SHA1_Init,
                          CC_SHA1_Update, CC_SHA1_Final>
    MessageDigestSHA1;
typedef MessageDigestBase<CC_SHA224_DIGEST_LENGTH, CC_SHA256_CTX,
                          CC_SHA224_Init, CC_SHA224_Update, CC_SHA224_Final>
    MessageDigestSHA224;
typedef MessageDigestBase<CC_SHA256_DIGEST_LENGTH, CC_SHA256_CTX,
                          CC_SHA256_Init, CC_SHA256_Update, CC_SHA256_Final>
    MessageDigestSHA256;
typedef MessageDigestBase<CC_SHA384_DIGEST_LENGTH, CC_SHA512_CTX,
                          CC_SHA384_Init, CC_SHA384_Update, CC_SHA384_Final>
    MessageDigestSHA384;
typedef MessageDigestBase<CC_SHA512_DIGEST_LENGTH, CC_SHA512_CTX,
                          CC_SHA512_Init, CC_SHA512_Update, CC_SHA512_Final>
    MessageDigestSHA512;

} // namespace

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
