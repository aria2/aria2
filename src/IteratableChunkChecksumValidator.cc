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
#include "IteratableChunkChecksumValidator.h"
#include "Util.h"
#include "message.h"
#include "MessageDigestHelper.h"
#include "DiskAdaptor.h"
#include "RecoverableException.h"
#include "DownloadContext.h"
#include "PieceStorage.h"
#include "BitfieldMan.h"
#include "LogFactory.h"
#include "Logger.h"

IteratableChunkChecksumValidator::
IteratableChunkChecksumValidator(const DownloadContextHandle& dctx,
				 const PieceStorageHandle& pieceStorage):
  _dctx(dctx),
  _pieceStorage(pieceStorage),
  _bitfield(new BitfieldMan(_dctx->getPieceLength(), _dctx->getTotalLength())),
  _currentIndex(0),
  _logger(LogFactory::getInstance()) {}

IteratableChunkChecksumValidator::~IteratableChunkChecksumValidator() {}


void IteratableChunkChecksumValidator::validateChunk()
{
  if(!finished()) {
    string actualChecksum;
    try {
      actualChecksum = calculateActualChecksum();
    } catch(RecoverableException* ex) {
      _logger->debug("Caught exception while validating piece index=%d. Some part of file may be missing. Continue operation.", ex, _currentIndex);
      delete ex;
      _bitfield->unsetBit(_currentIndex);
      _currentIndex++;
      return;
    }
    if(actualChecksum == _dctx->getPieceHashes()[_currentIndex]) {
      _bitfield->setBit(_currentIndex);
    } else {
      _logger->info(EX_INVALID_CHUNK_CHECKSUM,
		    _currentIndex,
		    Util::llitos(getCurrentOffset(), true).c_str(),
		    _dctx->getPieceHashes()[_currentIndex].c_str(),
		    actualChecksum.c_str());
      _bitfield->unsetBit(_currentIndex);
    }
    _currentIndex++;
    if(finished()) {
      _pieceStorage->setBitfield(_bitfield->getBitfield(), _bitfield->getBitfieldLength());
    }
  }
}

string IteratableChunkChecksumValidator::calculateActualChecksum()
{
  int64_t offset = getCurrentOffset();
  int32_t length;
  // When validating last piece
  if(_currentIndex+1 == (uint32_t)_dctx->getNumPieces()) {
    length = _dctx->getTotalLength()-offset;
  } else {
    length = _dctx->getPieceLength();
  }
  return MessageDigestHelper::digest(_dctx->getPieceHashAlgo(),
				     _pieceStorage->getDiskAdaptor(),
				     offset, length);
}

void IteratableChunkChecksumValidator::init()
{
  _bitfield->clearAllBit();
  _currentIndex = 0;
}

bool IteratableChunkChecksumValidator::finished() const
{
  return _currentIndex >= (uint32_t)_dctx->getNumPieces();
}

int64_t IteratableChunkChecksumValidator::getCurrentOffset() const
{
  return (int64_t)_currentIndex*_dctx->getPieceLength();
}

int64_t IteratableChunkChecksumValidator::getTotalLength() const
{
  return _dctx->getTotalLength();
}
