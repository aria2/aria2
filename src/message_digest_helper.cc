/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
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
#include "message_digest_helper.h"

#include <cstring>
#include <cstdlib>

#include "MessageDigest.h"
#include "DlAbortEx.h"
#include "message.h"
#include "DefaultDiskWriter.h"
#include "util.h"
#include "fmt.h"

namespace aria2 {

namespace message_digest {

namespace {

SharedHandle<MessageDigest> sha1Ctx_;

} // namespace

void staticSHA1DigestInit()
{
  staticSHA1DigestFree();
  sha1Ctx_ = MessageDigest::sha1();
}

void staticSHA1DigestFree()
{
  sha1Ctx_.reset();
}

std::string staticSHA1Digest
(const BinaryStreamHandle& bs, int64_t offset, int64_t length)
{
  sha1Ctx_->reset();
  return digest(sha1Ctx_, bs, offset, length);
}

std::string digest
(const SharedHandle<MessageDigest>& ctx,
 const SharedHandle<BinaryStream>& bs,
 int64_t offset, int64_t length)
{
  size_t BUFSIZE = 4096;
  unsigned char BUF[BUFSIZE];
  lldiv_t res = lldiv(length, BUFSIZE);
  int64_t iteration = res.quot;
  size_t tail = res.rem;
  for(int64_t i = 0; i < iteration; ++i) {
    ssize_t readLength = bs->readData(BUF, BUFSIZE, offset);
    if((size_t)readLength != BUFSIZE) {
      throw DL_ABORT_EX(fmt(EX_FILE_READ, "n/a", "data is too short"));
    }
    ctx->update(BUF, readLength);
    offset += readLength;
  }
  if(tail) {
    ssize_t readLength = bs->readData(BUF, tail, offset);
    if((size_t)readLength != tail) {
      throw DL_ABORT_EX(fmt(EX_FILE_READ, "n/a", "data is too short"));
    }
    ctx->update(BUF, readLength);
  }
  return ctx->digest();
}

void digest
(unsigned char* md, size_t mdLength,
 const SharedHandle<MessageDigest>& ctx, const void* data, size_t length)
{
  size_t reqLength = ctx->getDigestLength();
  if(mdLength < reqLength) {
    throw DL_ABORT_EX
      (fmt("Insufficient space for storing message digest:"
           " %lu required, but only %lu is allocated",
           static_cast<unsigned long>(reqLength),
           static_cast<unsigned long>(mdLength)));
  }
  ctx->update(data, length);
  ctx->digest(md);
}

} // namespace message_digest

} // namespace aria2
