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
#include "FileEntry.h"
#include "RecoverableException.h"
#include "DownloadContext.h"
#include "PieceStorage.h"
#include "BitfieldMan.h"
#include "LogFactory.h"
#include "Logger.h"
#include "messageDigest.h"
#include "StringFormat.h"
#include <cerrno>
#include <cstring>

namespace aria2 {

#define BUFSIZE (256*1024)
#define ALIGNMENT 512

IteratableChunkChecksumValidator::
IteratableChunkChecksumValidator(const DownloadContextHandle& dctx,
				 const PieceStorageHandle& pieceStorage):
  _dctx(dctx),
  _pieceStorage(pieceStorage),
  _bitfield(new BitfieldMan(_dctx->getPieceLength(), _dctx->getTotalLength())),
  _currentIndex(0),
  _logger(LogFactory::getInstance()),
  _buffer(0) {}

IteratableChunkChecksumValidator::~IteratableChunkChecksumValidator()
{
#ifdef HAVE_POSIX_MEMALIGN
  free(_buffer);
#else // !HAVE_POSIX_MEMALIGN
  delete [] _buffer;
#endif // !HAVE_POSIX_MEMALIGN
}


void IteratableChunkChecksumValidator::validateChunk()
{
  if(!finished()) {
    std::string actualChecksum;
    try {
      actualChecksum = calculateActualChecksum();
    } catch(RecoverableException& ex) {
      _logger->debug("Caught exception while validating piece index=%d. Some part of file may be missing. Continue operation.", ex, _currentIndex);
      _bitfield->unsetBit(_currentIndex);
      _currentIndex++;
      return;
    }
    if(actualChecksum == _dctx->getPieceHashes()[_currentIndex]) {
      _bitfield->setBit(_currentIndex);
    } else {
      _logger->info(EX_INVALID_CHUNK_CHECKSUM,
		    _currentIndex,
		    Util::itos(getCurrentOffset(), true).c_str(),
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

std::string IteratableChunkChecksumValidator::calculateActualChecksum()
{
  off_t offset = getCurrentOffset();
  size_t length;
  // When validating last piece
  if(_currentIndex+1 == _dctx->getNumPieces()) {
    length = _dctx->getTotalLength()-offset;
  } else {
    length = _dctx->getPieceLength();
  }
  return digest(offset, length);
}

void IteratableChunkChecksumValidator::init()
{
#ifdef HAVE_POSIX_MEMALIGN
  free(_buffer);
  _buffer = (unsigned char*)Util::allocateAlignedMemory(ALIGNMENT, BUFSIZE);
#else // !HAVE_POSIX_MEMALIGN
  delete [] _buffer;
  _buffer = new unsigned char[BUFSIZE];
#endif // !HAVE_POSIX_MEMALIGN
  if(_dctx->getFileEntries().size() == 1) {
    _pieceStorage->getDiskAdaptor()->enableDirectIO();
  }
  _ctx.reset(new MessageDigestContext());
  _ctx->trySetAlgo(_dctx->getPieceHashAlgo());
  _ctx->digestInit();
  _bitfield->clearAllBit();
  _currentIndex = 0;
}

std::string IteratableChunkChecksumValidator::digest(off_t offset, size_t length)
{
  _ctx->digestReset();
  off_t curoffset = offset/ALIGNMENT*ALIGNMENT;
  off_t max = offset+length;
  off_t woffset;
  if(curoffset < offset) {
    woffset = offset-curoffset;
  } else {
    woffset = 0;
  }
  while(curoffset < max) {
    size_t r = _pieceStorage->getDiskAdaptor()->readData(_buffer, BUFSIZE,
							 curoffset);
    if(r == 0) {
      throw DlAbortEx
	(StringFormat(EX_FILE_READ, _dctx->getActualBasePath().c_str(),
		      strerror(errno)).str());
    }
    size_t wlength;
    if(max < curoffset+r) {
      wlength = max-curoffset-woffset;
    } else {
      wlength = r-woffset;
    }
    _ctx->digestUpdate(_buffer+woffset, wlength);
    curoffset += r;
    woffset = 0;
  }
  return Util::toHex((const unsigned char*)_ctx->digestFinal().c_str(), _ctx->digestLength());
}


bool IteratableChunkChecksumValidator::finished() const
{
  if(_currentIndex >= _dctx->getNumPieces()) {
    _pieceStorage->getDiskAdaptor()->disableDirectIO();
    return true;
  } else {
    return false;
  }
}

off_t IteratableChunkChecksumValidator::getCurrentOffset() const
{
  return (off_t)_currentIndex*_dctx->getPieceLength();
}

uint64_t IteratableChunkChecksumValidator::getTotalLength() const
{
  return _dctx->getTotalLength();
}

} // namespace aria2
