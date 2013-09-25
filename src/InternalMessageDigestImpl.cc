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

#include "md5.h"
#include "sha1.h"

namespace aria2 {
namespace {

template<size_t dlen,
         typename ctx_t,
         int (*init_fn)(ctx_t**),
         void (*update_fn)(ctx_t*, const void*, size_t),
         void (*final_fn)(ctx_t*, uint8_t*),
         void (*free_fn)(ctx_t**)>
class MessageDigestBase : public MessageDigestImpl {
public:
  MessageDigestBase() { reset(); }
  virtual ~MessageDigestBase()
  {
    free_fn(&ctx_);
  }

  static  size_t length()
  {
    return dlen;
  }
  virtual size_t getDigestLength() const CXX11_OVERRIDE
  {
    return dlen;
  }
  virtual void reset() CXX11_OVERRIDE
  {
    init_fn(&ctx_);
  }
  virtual void update(const void* data, size_t length) CXX11_OVERRIDE
  {
    auto bytes = reinterpret_cast<const char*>(data);
    while (length) {
      size_t l = std::min(length, (size_t)std::numeric_limits<uint32_t>::max());
      update_fn(ctx_, bytes, l);
      length -= l;
      bytes += l;
    }
  }
  virtual void digest(unsigned char* md) CXX11_OVERRIDE
  {
    final_fn(ctx_, md);
  }
private:
  ctx_t* ctx_;
};

typedef MessageDigestBase<MD5_LENGTH,
                          struct MD5_CTX,
                          MD5_Init,
                          MD5_Update,
                          MD5_Final,
                          MD5_Free>
MessageDigestMD5;
typedef MessageDigestBase<SHA1_LENGTH,
                          SHA1_CTX,
                          SHA1_Init,
                          SHA1_Update,
                          SHA1_Final,
                          SHA1_Free>
MessageDigestSHA1;

} // namespace

std::unique_ptr<MessageDigestImpl> MessageDigestImpl::sha1()
{
  return make_unique<MessageDigestSHA1>();
}

MessageDigestImpl::hashes_t MessageDigestImpl::hashes = {
  { "sha-1", make_hi<MessageDigestSHA1>() },
  { "md5", make_hi<MessageDigestMD5>() },
};

} // namespace aria2
