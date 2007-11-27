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
#include "DefaultDiskWriter.h"
#include "DirectDiskAdaptor.h"
#include "prefs.h"
#include "DefaultDiskWriterFactory.h"
#include "DownloadContext.h"
#include "PieceStorage.h"
#include "Piece.h"

UnknownLengthPieceStorage::UnknownLengthPieceStorage(const DownloadContextHandle& downloadContext,
						     const Option* option):
  _downloadContext(downloadContext),
  _option(option),
  _diskAdaptor(0),
  _diskWriterFactory(new DefaultDiskWriterFactory()),
  _totalLength(0),
  _downloadFinished(false),
  _piece(0) {}

UnknownLengthPieceStorage::~UnknownLengthPieceStorage() {}

void UnknownLengthPieceStorage::initStorage()
{
  DiskWriterHandle writer = _diskWriterFactory->newDiskWriter();
  DirectDiskAdaptorHandle directDiskAdaptor = new DirectDiskAdaptor();
  directDiskAdaptor->setDiskWriter(writer);
  directDiskAdaptor->setTotalLength(_downloadContext->getTotalLength());
  _diskAdaptor = directDiskAdaptor;
  string storeDir = _downloadContext->getDir();
//   if(storeDir == "") {
//     storeDir = ".";
//   }
  _diskAdaptor->setStoreDir(storeDir);
  _diskAdaptor->setFileEntries(_downloadContext->getFileEntries());
}

PieceHandle UnknownLengthPieceStorage::getMissingPiece()
{
  if(_downloadFinished) {
    return 0;
  }
  if(_piece.isNull()) {
    _piece = new Piece();
    return _piece;
  } else {
    return 0;
  }
}

PieceHandle UnknownLengthPieceStorage::getMissingPiece(int32_t index)
{
  if(index == 0) {
    return getMissingPiece();
  } else {
    return 0;
  }
}

PieceHandle UnknownLengthPieceStorage::getPiece(int32_t index)
{
  if(index == 0) {
    if(_piece.isNull()) {
      return new Piece();
    } else {
      return _piece;
    }
  } else {
    return 0;
  }
}

void UnknownLengthPieceStorage::completePiece(const PieceHandle& piece)
{
  if(_piece == piece) {
    _downloadFinished = true;
    _totalLength = _piece->getLength();
    _diskAdaptor->setTotalLength(_totalLength);
    _piece = 0;
  }
}

void UnknownLengthPieceStorage::cancelPiece(const PieceHandle& piece)
{
  if(_piece == piece) {
    _piece = 0;
  }
}

bool UnknownLengthPieceStorage::hasPiece(int32_t index)
{
  if(index == 0 && _downloadFinished) {
    return true;
  } else {
    return false;
  }
}

bool UnknownLengthPieceStorage::isPieceUsed(int32_t index)
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

int32_t UnknownLengthPieceStorage::getPieceLength(int32_t index)
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
    _piece = 0;
  }
  _downloadFinished = true;
}

Pieces UnknownLengthPieceStorage::getInFlightPieces()
{
  return Pieces();
}

void UnknownLengthPieceStorage::setDiskWriterFactory(const DiskWriterFactoryHandle& diskWriterFactory)
{
  _diskWriterFactory = diskWriterFactory;
}
