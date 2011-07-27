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
#include "IteratableChunkChecksumValidator.h"

#include <cstring>
#include <cstdlib>

#include "util.h"
#include "message.h"
#include "DiskAdaptor.h"
#include "FileEntry.h"
#include "RecoverableException.h"
#include "DownloadContext.h"
#include "PieceStorage.h"
#include "BitfieldMan.h"
#include "LogFactory.h"
#include "Logger.h"
#include "MessageDigest.h"
#include "fmt.h"
#include "DlAbortEx.h"

namespace aria2 {

#define BUFSIZE (256*1024)
#define ALIGNMENT 512

IteratableChunkChecksumValidator::IteratableChunkChecksumValidator
(const SharedHandle<DownloadContext>& dctx,
 const PieceStorageHandle& pieceStorage)
  : dctx_(dctx),
    pieceStorage_(pieceStorage),
    bitfield_(new BitfieldMan(dctx_->getPieceLength(),
                              dctx_->getTotalLength())),
    currentIndex_(0),
    buffer_(0)
{}

IteratableChunkChecksumValidator::~IteratableChunkChecksumValidator()
{
  delete [] buffer_;
}


void IteratableChunkChecksumValidator::validateChunk()
{
  if(!finished()) {
    std::string actualChecksum;
    try {
      actualChecksum = calculateActualChecksum();
      if(actualChecksum == dctx_->getPieceHashes()[currentIndex_]) {
        bitfield_->setBit(currentIndex_);
      } else {
        A2_LOG_INFO(fmt(EX_INVALID_CHUNK_CHECKSUM,
                        static_cast<unsigned long>(currentIndex_),
                        util::itos(getCurrentOffset(), true).c_str(),
                        dctx_->getPieceHashes()[currentIndex_].c_str(),
                        actualChecksum.c_str()));
        bitfield_->unsetBit(currentIndex_);
      }
    } catch(RecoverableException& ex) {
      A2_LOG_DEBUG_EX(fmt("Caught exception while validating piece index=%lu."
                          " Some part of file may be missing."
                          " Continue operation.",
                          static_cast<unsigned long>(currentIndex_)),
                      ex);
      bitfield_->unsetBit(currentIndex_);
    }

    ++currentIndex_;
    if(finished()) {
      pieceStorage_->setBitfield(bitfield_->getBitfield(), bitfield_->getBitfieldLength());
    }
  }
}

std::string IteratableChunkChecksumValidator::calculateActualChecksum()
{
  off_t offset = getCurrentOffset();
  size_t length;
  // When validating last piece
  if(currentIndex_+1 == dctx_->getNumPieces()) {
    length = dctx_->getTotalLength()-offset;
  } else {
    length = dctx_->getPieceLength();
  }
  return digest(offset, length);
}

void IteratableChunkChecksumValidator::init()
{
  delete [] buffer_;
  buffer_ = new unsigned char[BUFSIZE];
  ctx_ = MessageDigest::create(dctx_->getPieceHashType());
  bitfield_->clearAllBit();
  currentIndex_ = 0;
}

std::string IteratableChunkChecksumValidator::digest(off_t offset, size_t length)
{
  ctx_->reset();
  off_t curoffset = offset/ALIGNMENT*ALIGNMENT;
  off_t max = offset+length;
  off_t woffset;
  if(curoffset < offset) {
    woffset = offset-curoffset;
  } else {
    woffset = 0;
  }
  while(curoffset < max) {
    size_t r = pieceStorage_->getDiskAdaptor()->readData(buffer_, BUFSIZE,
                                                         curoffset);
    if(r == 0 || r < static_cast<size_t>(woffset)) {
      throw DL_ABORT_EX
        (fmt(EX_FILE_READ, dctx_->getBasePath().c_str(),
             "data is too short"));
    }
    size_t wlength;
    if(max < static_cast<off_t>(curoffset+r)) {
      wlength = max-curoffset-woffset;
    } else {
      wlength = r-woffset;
    }
    ctx_->update(buffer_+woffset, wlength);
    curoffset += r;
    woffset = 0;
  }
  return ctx_->hexDigest();
}


bool IteratableChunkChecksumValidator::finished() const
{
  if(currentIndex_ >= dctx_->getNumPieces()) {
    return true;
  } else {
    return false;
  }
}

off_t IteratableChunkChecksumValidator::getCurrentOffset() const
{
  return (off_t)currentIndex_*dctx_->getPieceLength();
}

uint64_t IteratableChunkChecksumValidator::getTotalLength() const
{
  return dctx_->getTotalLength();
}

} // namespace aria2
