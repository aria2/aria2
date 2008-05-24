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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "Util.h"
#include "StringFormat.h"
#include <cerrno>
#include <cstring>
#include <cstdlib>

namespace aria2 {

MessageDigestContext* MessageDigestHelper::_sha1Ctx = 0;

void MessageDigestHelper::staticSHA1DigestInit()
{
  staticSHA1DigestFree();
  _sha1Ctx = new MessageDigestContext();
  _sha1Ctx->trySetAlgo(MessageDigestContext::SHA1);
  _sha1Ctx->digestInit();
}

void MessageDigestHelper::staticSHA1DigestFree()
{
  delete _sha1Ctx;
}

std::string MessageDigestHelper::staticSHA1Digest(const BinaryStreamHandle& bs,
						  off_t offset,
						  uint64_t length)
{
  _sha1Ctx->digestReset();
  return digest(_sha1Ctx, bs, offset, length);
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
      throw DlAbortEx
	(StringFormat(EX_FILE_READ, "n/a", strerror(errno)).str());
    }
    ctx->digestUpdate(BUF, readLength);
    offset += readLength;
  }
  if(tail) {
    ssize_t readLength = bs->readData(BUF, tail, offset);
    if((size_t)readLength != tail) {
      throw DlAbortEx
	(StringFormat(EX_FILE_READ, "n/a", strerror(errno)).str());
    }
    ctx->digestUpdate(BUF, readLength);
  }
  std::string rawMD = ctx->digestFinal();
  return Util::toHex((const unsigned char*)rawMD.c_str(), rawMD.size());
}

std::string MessageDigestHelper::digest(const std::string& algo, const std::string& filename)
{
  DiskWriterHandle writer(new DefaultDiskWriter());
  writer->openExistingFile(filename);
  return digest(algo, writer, 0, writer->size());
}

std::string MessageDigestHelper::digest(const std::string& algo, const void* data, size_t length)
{
  MessageDigestContext ctx;
  ctx.trySetAlgo(algo);
  ctx.digestInit();
  ctx.digestUpdate(data, length);
  std::string rawMD = ctx.digestFinal();
  return Util::toHex((const unsigned char*)rawMD.c_str(), rawMD.size());
}

void MessageDigestHelper::digest(unsigned char* md, size_t mdLength,
				 const std::string& algo, const void* data, size_t length)
{
  if(mdLength < MessageDigestContext::digestLength(algo)) {
    throw DlAbortEx
      (StringFormat("Insufficient space for storing message digest: %d required, but only %d is allocated",
		    MessageDigestContext::digestLength(algo), mdLength).str());
  }
  MessageDigestContext ctx;
  ctx.trySetAlgo(algo);
  ctx.digestInit();
  ctx.digestUpdate(data, length);
  ctx.digestFinal(md);
}

} // namespace aria2
