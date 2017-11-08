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
#include "BitfieldMan.h"

namespace aria2 {

UnknownLengthPieceStorage::UnknownLengthPieceStorage(
    const std::shared_ptr<DownloadContext>& downloadContext)
    : downloadContext_(downloadContext),
      diskWriterFactory_(std::make_shared<DefaultDiskWriterFactory>()),
      totalLength_(0),
      downloadFinished_(false)
{
}

UnknownLengthPieceStorage::~UnknownLengthPieceStorage() = default;

void UnknownLengthPieceStorage::initStorage()
{
  auto directDiskAdaptor = std::make_shared<DirectDiskAdaptor>();
  directDiskAdaptor->setTotalLength(downloadContext_->getTotalLength());
  directDiskAdaptor->setFileEntries(downloadContext_->getFileEntries().begin(),
                                    downloadContext_->getFileEntries().end());

  directDiskAdaptor->setDiskWriter(
      diskWriterFactory_->newDiskWriter(directDiskAdaptor->getFilePath()));

  diskAdaptor_ = std::move(directDiskAdaptor);
}

#ifdef ENABLE_BITTORRENT

bool UnknownLengthPieceStorage::hasMissingPiece(
    const std::shared_ptr<Peer>& peer)
{
  abort();
}

void UnknownLengthPieceStorage::getMissingPiece(
    std::vector<std::shared_ptr<Piece>>& pieces, size_t minMissingBlocks,
    const std::shared_ptr<Peer>& peer, cuid_t cuid)
{
  abort();
}

void UnknownLengthPieceStorage::getMissingPiece(
    std::vector<std::shared_ptr<Piece>>& pieces, size_t minMissingBlocks,
    const std::shared_ptr<Peer>& peer,
    const std::vector<size_t>& excludedIndexes, cuid_t cuid)
{
  abort();
}

void UnknownLengthPieceStorage::getMissingFastPiece(
    std::vector<std::shared_ptr<Piece>>& pieces, size_t minMissingBlocks,
    const std::shared_ptr<Peer>& peer, cuid_t cuid)
{
  abort();
}

void UnknownLengthPieceStorage::getMissingFastPiece(
    std::vector<std::shared_ptr<Piece>>& pieces, size_t minMissingBlocks,
    const std::shared_ptr<Peer>& peer,
    const std::vector<size_t>& excludedIndexes, cuid_t cuid)
{
  abort();
}

std::shared_ptr<Piece>
UnknownLengthPieceStorage::getMissingPiece(const std::shared_ptr<Peer>& peer,
                                           cuid_t cuid)
{
  abort();
}

std::shared_ptr<Piece> UnknownLengthPieceStorage::getMissingPiece(
    const std::shared_ptr<Peer>& peer,
    const std::vector<size_t>& excludedIndexes, cuid_t cuid)
{
  abort();
}
#endif // ENABLE_BITTORRENT

bool UnknownLengthPieceStorage::hasMissingUnusedPiece() { abort(); }

std::shared_ptr<Piece>
UnknownLengthPieceStorage::getMissingPiece(size_t minSplitSize,
                                           const unsigned char* ignoreBitfield,
                                           size_t length, cuid_t cuid)
{
  if (downloadFinished_) {
    return nullptr;
  }
  if (!piece_) {
    piece_ = std::make_shared<Piece>();
    return piece_;
  }
  else {
    return nullptr;
  }
}

std::shared_ptr<Piece> UnknownLengthPieceStorage::getMissingPiece(size_t index,
                                                                  cuid_t cuid)
{
  if (index == 0) {
    return getMissingPiece(0, nullptr, 0, cuid);
  }
  else {
    return nullptr;
  }
}

std::shared_ptr<Piece> UnknownLengthPieceStorage::getPiece(size_t index)
{
  if (index == 0) {
    if (!piece_) {
      return std::make_shared<Piece>();
    }
    else {
      return piece_;
    }
  }
  else {
    return nullptr;
  }
}

void UnknownLengthPieceStorage::completePiece(
    const std::shared_ptr<Piece>& piece)
{
  if (*piece_ == *piece) {
    downloadFinished_ = true;
    totalLength_ = piece_->getLength();
    diskAdaptor_->setTotalLength(totalLength_);
    piece_.reset();

    createBitfield();
  }
}

void UnknownLengthPieceStorage::cancelPiece(const std::shared_ptr<Piece>& piece,
                                            cuid_t cuid)
{
  if (*piece_ == *piece) {
    piece_.reset();
  }
}

bool UnknownLengthPieceStorage::hasPiece(size_t index)
{
  if (index == 0 && downloadFinished_) {
    return true;
  }
  else {
    return false;
  }
}

bool UnknownLengthPieceStorage::isPieceUsed(size_t index)
{
  if (index == 0 && piece_) {
    return true;
  }
  else {
    return false;
  }
}

int64_t UnknownLengthPieceStorage::getCompletedLength()
{
  // TODO we have to return actual completed length here?
  if (piece_) {
    return piece_->getLength();
  }
  return totalLength_;
}

std::shared_ptr<DiskAdaptor> UnknownLengthPieceStorage::getDiskAdaptor()
{
  return diskAdaptor_;
}

int32_t UnknownLengthPieceStorage::getPieceLength(size_t index)
{
  // TODO Basically, PieceStorage::getPieceLength() is only used by
  // BitTorrent, and it does not use UnknownLengthPieceStorage.
  abort();
}

void UnknownLengthPieceStorage::createBitfield()
{
  if (totalLength_ > 0) {
    bitfield_ = make_unique<BitfieldMan>(downloadContext_->getPieceLength(),
                                         totalLength_);
    bitfield_->setAllBit();
  }
}

void UnknownLengthPieceStorage::markAllPiecesDone()
{
  if (piece_) {
    totalLength_ = piece_->getLength();
    piece_.reset();
  }

  createBitfield();

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

void UnknownLengthPieceStorage::getInFlightPieces(
    std::vector<std::shared_ptr<Piece>>& pieces)
{
}

void UnknownLengthPieceStorage::setDiskWriterFactory(
    const std::shared_ptr<DiskWriterFactory>& diskWriterFactory)
{
  diskWriterFactory_ = diskWriterFactory;
}

const unsigned char* UnknownLengthPieceStorage::getBitfield()
{
  if (bitfield_) {
    return bitfield_->getBitfield();
  }

  return nullptr;
}

size_t UnknownLengthPieceStorage::getBitfieldLength()
{
  if (bitfield_) {
    return bitfield_->getBitfieldLength();
  }

  return 0;
}

} // namespace aria2
