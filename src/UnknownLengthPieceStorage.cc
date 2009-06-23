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

UnknownLengthPieceStorage::UnknownLengthPieceStorage(const DownloadContextHandle& downloadContext,
						     const Option* option):
  _downloadContext(downloadContext),
  _option(option),
  _diskWriterFactory(new DefaultDiskWriterFactory()),
  _totalLength(0),
  _downloadFinished(false) {}

UnknownLengthPieceStorage::~UnknownLengthPieceStorage() {}

void UnknownLengthPieceStorage::initStorage()
{
  DirectDiskAdaptorHandle directDiskAdaptor(new DirectDiskAdaptor());
  directDiskAdaptor->setTotalLength(_downloadContext->getTotalLength());
  directDiskAdaptor->setFileEntries(_downloadContext->getFileEntries());

  DiskWriterHandle writer =
    _diskWriterFactory->newDiskWriter(directDiskAdaptor->getFilePath());
  directDiskAdaptor->setDiskWriter(writer);

  _diskAdaptor = directDiskAdaptor;
}

#ifdef ENABLE_BITTORRENT

bool UnknownLengthPieceStorage::hasMissingPiece(const SharedHandle<Peer>& peer)
{
  abort();
}

SharedHandle<Piece> UnknownLengthPieceStorage::getMissingPiece(const SharedHandle<Peer>& peer)
{
  abort();
}

SharedHandle<Piece> UnknownLengthPieceStorage::getMissingPiece
(const SharedHandle<Peer>& peer, const std::deque<size_t>& excludedIndexes)
{
  abort();
}

SharedHandle<Piece> UnknownLengthPieceStorage::getMissingFastPiece(const SharedHandle<Peer>& peer)
{
  abort();
}

SharedHandle<Piece> UnknownLengthPieceStorage::getMissingFastPiece
(const SharedHandle<Peer>& peer, const std::deque<size_t>& excludedIndexes)
{
  abort();
}

#endif // ENABLE_BITTORRENT

SharedHandle<Piece> UnknownLengthPieceStorage::getSparseMissingUnusedPiece
(const unsigned char* ignoreBitfield, size_t length)
{
  if(_downloadFinished) {
    return SharedHandle<Piece>();
  }
  if(_piece.isNull()) {
    _piece.reset(new Piece());
    return _piece;
  } else {
    return SharedHandle<Piece>();
  }
}

PieceHandle UnknownLengthPieceStorage::getMissingPiece(size_t index)
{
  if(index == 0) {
    return getSparseMissingUnusedPiece(0, 0);
  } else {
    return SharedHandle<Piece>();
  }
}

PieceHandle UnknownLengthPieceStorage::getPiece(size_t index)
{
  if(index == 0) {
    if(_piece.isNull()) {
      return SharedHandle<Piece>(new Piece());
    } else {
      return _piece;
    }
  } else {
    return SharedHandle<Piece>();
  }
}

void UnknownLengthPieceStorage::completePiece(const PieceHandle& piece)
{
  if(_piece == piece) {
    _downloadFinished = true;
    _totalLength = _piece->getLength();
    _diskAdaptor->setTotalLength(_totalLength);
    _piece.reset();
  }
}

void UnknownLengthPieceStorage::cancelPiece(const PieceHandle& piece)
{
  if(_piece == piece) {
    _piece.reset();
  }
}

bool UnknownLengthPieceStorage::hasPiece(size_t index)
{
  if(index == 0 && _downloadFinished) {
    return true;
  } else {
    return false;
  }
}

bool UnknownLengthPieceStorage::isPieceUsed(size_t index)
{
  if(index == 0 && !_piece.isNull()) {
    return true;
  } else {
    return false;
  }
}

DiskAdaptorHandle UnknownLengthPieceStorage::getDiskAdaptor()
{
  return _diskAdaptor;
}

size_t UnknownLengthPieceStorage::getPieceLength(size_t index)
{
  if(index == 0) {
    return _totalLength;
  } else {
    return 0;
  }
}

void UnknownLengthPieceStorage::markAllPiecesDone()
{
  if(!_piece.isNull()) {
    _totalLength = _piece->getLength();
    _piece.reset();
  }
  _downloadFinished = true;
}

void UnknownLengthPieceStorage::markPiecesDone(uint64_t length)
{
  // TODO not implemented yet
  abort();
}

void UnknownLengthPieceStorage::markPieceMissing(size_t index)
{
  // TODO not implemented yet
  abort();
}

void UnknownLengthPieceStorage::getInFlightPieces(std::deque<SharedHandle<Piece> >& pieces)
{}

void UnknownLengthPieceStorage::setDiskWriterFactory(const DiskWriterFactoryHandle& diskWriterFactory)
{
  _diskWriterFactory = diskWriterFactory;
}

} // namespace aria2
