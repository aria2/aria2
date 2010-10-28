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
#include "MessageDigestHelper.h"
#include "messageDigest.h"
#include "DlAbortEx.h"
#include "message.h"
#include "DefaultDiskWriter.h"
#include "util.h"
#include "StringFormat.h"
#include <cerrno>
#include <cstring>
#include <cstdlib>

namespace aria2 {

MessageDigestContext* MessageDigestHelper::sha1Ctx_ = 0;

void MessageDigestHelper::staticSHA1DigestInit()
{
  staticSHA1DigestFree();
  sha1Ctx_ = new MessageDigestContext();
  sha1Ctx_->trySetAlgo(MessageDigestContext::SHA1);
  sha1Ctx_->digestInit();
}

void MessageDigestHelper::staticSHA1DigestFree()
{
  delete sha1Ctx_;
}

std::string MessageDigestHelper::staticSHA1Digest(const BinaryStreamHandle& bs,
                                                  off_t offset,
                                                  uint64_t length)
{
  sha1Ctx_->digestReset();
  return digest(sha1Ctx_, bs, offset, length);
}

std::string MessageDigestHelper::digest(const std::string& algo,
                                        const BinaryStreamHandle& bs,
                                        off_t offset,
                                        uint64_t length)
{
  MessageDigestContext ctx;
  ctx.trySetAlgo(algo);
  ctx.digestInit();
  return digest(&ctx, bs, offset, length);
}

std::string MessageDigestHelper::digest(MessageDigestContext* ctx,
                                        const SharedHandle<BinaryStream>& bs,
                                        off_t offset, uint64_t length)
{
  size_t BUFSIZE = 4096;
  unsigned char BUF[BUFSIZE];
  lldiv_t res = lldiv(length, BUFSIZE);
  uint64_t iteration = res.quot;
  size_t tail = res.rem;
  for(uint64_t i = 0; i < iteration; ++i) {
    ssize_t readLength = bs->readData(BUF, BUFSIZE, offset);
    if((size_t)readLength != BUFSIZE) {
      throw DL_ABORT_EX
        (StringFormat(EX_FILE_READ, "n/a", strerror(errno)).str());
    }
    ctx->digestUpdate(BUF, readLength);
    offset += readLength;
  }
  if(tail) {
    ssize_t readLength = bs->readData(BUF, tail, offset);
    if((size_t)readLength != tail) {
      throw DL_ABORT_EX
        (StringFormat(EX_FILE_READ, "n/a", strerror(errno)).str());
    }
    ctx->digestUpdate(BUF, readLength);
  }
  std::string rawMD = ctx->digestFinal();
  return util::toHex(rawMD);
}

std::string MessageDigestHelper::digest(const std::string& algo, const std::string& filename)
{
  DiskWriterHandle writer(new DefaultDiskWriter(filename));
  writer->openExistingFile();
  return digest(algo, writer, 0, writer->size());
}

std::string MessageDigestHelper::digest(const std::string& algo, const void* data, size_t length)
{
  MessageDigestContext ctx;
  ctx.trySetAlgo(algo);
  ctx.digestInit();
  ctx.digestUpdate(data, length);
  std::string rawMD = ctx.digestFinal();
  return util::toHex(rawMD);
}

void MessageDigestHelper::digest(unsigned char* md, size_t mdLength,
                                 const std::string& algo, const void* data, size_t length)
{
  if(mdLength < MessageDigestContext::digestLength(algo)) {
    throw DL_ABORT_EX
      (StringFormat
       ("Insufficient space for storing message digest:"
        " %lu required, but only %lu is allocated",
        static_cast<unsigned long>(MessageDigestContext::digestLength(algo)),
        static_cast<unsigned long>(mdLength)).str());
  }
  MessageDigestContext ctx;
  ctx.trySetAlgo(algo);
  ctx.digestInit();
  ctx.digestUpdate(data, length);
  ctx.digestFinal(md);
}

} // namespace aria2
