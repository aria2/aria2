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
#include "IteratableChecksumValidator.h"

#include <array>
#include <cstdlib>

#include "util.h"
#include "message.h"
#include "PieceStorage.h"
#include "MessageDigest.h"
#include "DiskAdaptor.h"
#include "FileEntry.h"
#include "BitfieldMan.h"
#include "DownloadContext.h"
#include "LogFactory.h"
#include "fmt.h"

namespace aria2 {

IteratableChecksumValidator::IteratableChecksumValidator(
    const std::shared_ptr<DownloadContext>& dctx,
    const std::shared_ptr<PieceStorage>& pieceStorage)
    : dctx_(dctx), pieceStorage_(pieceStorage), currentOffset_(0)
{
}

IteratableChecksumValidator::~IteratableChecksumValidator() = default;

void IteratableChecksumValidator::validateChunk()
{
  // Don't guard with !finished() to allow zero-length file to be
  // verified.
  std::array<unsigned char, 4_k> buf;
  size_t length = pieceStorage_->getDiskAdaptor()->readDataDropCache(
      buf.data(), buf.size(), currentOffset_);
  ctx_->update(buf.data(), length);
  currentOffset_ += length;
  if (finished()) {
    std::string actualDigest = ctx_->digest();
    if (dctx_->getDigest() == actualDigest) {
      pieceStorage_->markAllPiecesDone();
      dctx_->setChecksumVerified(true);
    }
    else {
      A2_LOG_INFO(fmt("Checksum validation failed. expected=%s, actual=%s",
                      util::toHex(dctx_->getDigest()).c_str(),
                      util::toHex(actualDigest).c_str()));
      BitfieldMan bitfield(dctx_->getPieceLength(), dctx_->getTotalLength());
      pieceStorage_->setBitfield(bitfield.getBitfield(),
                                 bitfield.getBitfieldLength());
    }
  }
}

bool IteratableChecksumValidator::finished() const
{
  if (currentOffset_ >= dctx_->getTotalLength()) {
    return true;
  }
  else {
    return false;
  }
}

int64_t IteratableChecksumValidator::getTotalLength() const
{
  return dctx_->getTotalLength();
}

void IteratableChecksumValidator::init()
{
  currentOffset_ = 0;
  ctx_ = MessageDigest::create(dctx_->getHashType());
}

} // namespace aria2
