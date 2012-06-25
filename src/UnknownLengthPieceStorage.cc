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
#include "UnknownLengthPieceStorage.h"

#include <cstdlib>

#include "DefaultDiskWriter.h"
#include "DirectDiskAdaptor.h"
#include "prefs.h"
#include "DefaultDiskWriterFactory.h"
#include "DownloadContext.h"
#include "Piece.h"
#include "FileEntry.h"

namespace aria2 {

UnknownLengthPieceStorage::UnknownLengthPieceStorage
(const SharedHandle<DownloadContext>& downloadContext,
 const Option* option):
  downloadContext_(downloadContext),
  option_(option),
  diskWriterFactory_(new DefaultDiskWriterFactory()),
  totalLength_(0),
  downloadFinished_(false) {}

UnknownLengthPieceStorage::~UnknownLengthPieceStorage() {}

void UnknownLengthPieceStorage::initStorage()
{
  DirectDiskAdaptorHandle directDiskAdaptor(new DirectDiskAdaptor());
  directDiskAdaptor->setTotalLength(downloadContext_->getTotalLength());
  directDiskAdaptor->setFileEntries(downloadContext_->getFileEntries().begin(),
                                    downloadContext_->getFileEntries().end());

  DiskWriterHandle writer =
    diskWriterFactory_->newDiskWriter(directDiskAdaptor->getFilePath());
  directDiskAdaptor->setDiskWriter(writer);

  diskAdaptor_ = directDiskAdaptor;
}

#ifdef ENABLE_BITTORRENT

bool UnknownLengthPieceStorage::hasMissingPiece(const SharedHandle<Peer>& peer)
{
  abort();
}

void UnknownLengthPieceStorage::getMissingPiece
(std::vector<SharedHandle<Piece> >& pieces,
 size_t minMissingBlocks,
 const SharedHandle<Peer>& peer,
 cuid_t cuid)
{
  abort();
}

void UnknownLengthPieceStorage::getMissingPiece
(std::vector<SharedHandle<Piece> >& pieces,
 size_t minMissingBlocks,
 const SharedHandle<Peer>& peer,
 const std::vector<size_t>& excludedIndexes,
 cuid_t cuid)
{
  abort();
}

void UnknownLengthPieceStorage::getMissingFastPiece
(std::vector<SharedHandle<Piece> >& pieces,
 size_t minMissingBlocks,
 const SharedHandle<Peer>& peer,
 cuid_t cuid)
{
  abort();
}

void UnknownLengthPieceStorage::getMissingFastPiece
(std::vector<SharedHandle<Piece> >& pieces,
 size_t minMissingBlocks,
 const SharedHandle<Peer>& peer,
 const std::vector<size_t>& excludedIndexes,
 cuid_t cuid)
{
  abort();
}

SharedHandle<Piece> UnknownLengthPieceStorage::getMissingPiece
(const SharedHandle<Peer>& peer,
 cuid_t cuid)
{
  abort();
}

SharedHandle<Piece> UnknownLengthPieceStorage::getMissingPiece
(const SharedHandle<Peer>& peer,
 const std::vector<size_t>& excludedIndexes,
 cuid_t cuid)
{
  abort();
}
#endif // ENABLE_BITTORRENT

bool UnknownLengthPieceStorage::hasMissingUnusedPiece()
{
  abort();
}

SharedHandle<Piece> UnknownLengthPieceStorage::getMissingPiece
(size_t minSplitSize,
 const unsigned char* ignoreBitfield,
 size_t length,
 cuid_t cuid)
{
  if(downloadFinished_) {
    return SharedHandle<Piece>();
  }
  if(!piece_) {
    piece_.reset(new Piece());
    return piece_;
  } else {
    return SharedHandle<Piece>();
  }
}

SharedHandle<Piece> UnknownLengthPieceStorage::getMissingPiece
(size_t index,
 cuid_t cuid)
{
  if(index == 0) {
    return getMissingPiece(0, 0, 0, cuid);
  } else {
    return SharedHandle<Piece>();
  }
}

SharedHandle<Piece> UnknownLengthPieceStorage::getPiece(size_t index)
{
  if(index == 0) {
    if(!piece_) {
      return SharedHandle<Piece>(new Piece());
    } else {
      return piece_;
    }
  } else {
    return SharedHandle<Piece>();
  }
}

void UnknownLengthPieceStorage::completePiece(const SharedHandle<Piece>& piece)
{
  if(*piece_ == *piece) {
    downloadFinished_ = true;
    totalLength_ = piece_->getLength();
    diskAdaptor_->setTotalLength(totalLength_);
    piece_.reset();
  }
}

void UnknownLengthPieceStorage::cancelPiece
(const SharedHandle<Piece>& piece,
 cuid_t cuid)
{
  if(*piece_ == *piece) {
    piece_.reset();
  }
}

bool UnknownLengthPieceStorage::hasPiece(size_t index)
{
  if(index == 0 && downloadFinished_) {
    return true;
  } else {
    return false;
  }
}

bool UnknownLengthPieceStorage::isPieceUsed(size_t index)
{
  if(index == 0 && piece_) {
    return true;
  } else {
    return false;
  }
}

DiskAdaptorHandle UnknownLengthPieceStorage::getDiskAdaptor()
{
  return diskAdaptor_;
}

int32_t UnknownLengthPieceStorage::getPieceLength(size_t index)
{
  if(index == 0) {
    return totalLength_;
  } else {
    return 0;
  }
}

void UnknownLengthPieceStorage::markAllPiecesDone()
{
  if(piece_) {
    totalLength_ = piece_->getLength();
    piece_.reset();
  }
  downloadFinished_ = true;
}

void UnknownLengthPieceStorage::markPiecesDone(int64_t length)
{
  // TODO not implemented yet
  abort();
}

void UnknownLengthPieceStorage::markPieceMissing(size_t index)
{
  // TODO not implemented yet
  abort();
}

void UnknownLengthPieceStorage::getInFlightPieces
(std::vector<SharedHandle<Piece> >& pieces)
{}

void UnknownLengthPieceStorage::setDiskWriterFactory
(const DiskWriterFactoryHandle& diskWriterFactory)
{
  diskWriterFactory_ = diskWriterFactory;
}

} // namespace aria2
