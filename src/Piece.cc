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
#include "Piece.h"
#include "Util.h"
#include "BitfieldManFactory.h"

Piece::Piece():index(0), length(0), _blockLength(BLOCK_LENGTH), bitfield(0) {}

Piece::Piece(int32_t index, int32_t length, int32_t blockLength):index(index), length(length), _blockLength(blockLength) {
  bitfield =
    BitfieldManFactory::getFactoryInstance()->createBitfieldMan(_blockLength, length);
}

Piece::Piece(const Piece& piece) {
  index = piece.index;
  length = piece.length;
  _blockLength = piece._blockLength;
  if(piece.bitfield == 0) {
    bitfield = 0;
  } else {
    bitfield = new BitfieldMan(*piece.bitfield);
  }
}

void Piece::completeBlock(int32_t blockIndex) {
  bitfield->setBit(blockIndex);
  bitfield->unsetUseBit(blockIndex);
  removeSubPiece(blockIndex);
}

void Piece::clearAllBlock() {
  bitfield->clearAllBit();
  bitfield->clearAllUseBit();
}

void Piece::setAllBlock() {
  bitfield->setAllBit();
}

bool Piece::pieceComplete() const {
  return bitfield->isAllBitSet();
}

void Piece::cancelBlock(int32_t blockIndex) {
  bitfield->unsetUseBit(blockIndex);
}

int32_t Piece::getMissingUnusedBlockIndex() const {
  int32_t blockIndex = bitfield->getFirstMissingUnusedIndex();
  if(blockIndex == -1) {
    return blockIndex;
  }
  bitfield->setUseBit(blockIndex);
  return blockIndex;
}

int32_t Piece::getMissingBlockIndex() const {
  int32_t blockIndex = bitfield->getMissingIndex();
  if(blockIndex == -1) {
    return blockIndex;
  }
  bitfield->setUseBit(blockIndex);
  return blockIndex;
}

BlockIndexes Piece::getAllMissingBlockIndexes() const {
  return bitfield->getAllMissingIndexes();
}

string Piece::toString() const {
  return "piece: index="+Util::itos(index)+", length="+Util::itos(length);
}

void Piece::reconfigure(int32_t length)
{
  delete bitfield;
  this->length = length;
  bitfield =
    BitfieldManFactory::getFactoryInstance()->createBitfieldMan(_blockLength, length);
}

void Piece::setBitfield(const unsigned char* bitfield, int32_t len)
{
  this->bitfield->setBitfield(bitfield, len);
}

void Piece::addSubPiece(const PieceHandle& subPiece)
{
  _subPieces.push_back(subPiece);
}

PieceHandle Piece::getSubPiece(int32_t blockIndex)
{
  Pieces::iterator itr = getSubPieceIterator(blockIndex);
  if(itr == _subPieces.end()) {
    return 0;
  } else {
    return *itr;
  }
}

void Piece::removeSubPiece(int32_t blockIndex)
{
  Pieces::iterator itr = getSubPieceIterator(blockIndex);
  if(itr != _subPieces.end()) {
    _subPieces.erase(itr);
  }  
}

Pieces::iterator Piece::getSubPieceIterator(int32_t blockIndex)
{
  for(Pieces::iterator itr = _subPieces.begin(); itr != _subPieces.end(); ++itr) {
    if((*itr)->getIndex() == blockIndex) {
      return itr;
    }
  }
  return _subPieces.end();
}

bool Piece::isRangeComplete(int32_t offset, int32_t length)
{
  int32_t startIndex = START_INDEX(offset, _blockLength);
  int32_t endIndex = END_INDEX(offset, length, _blockLength);
  if(countBlock() <= endIndex) {
    endIndex = countBlock()-1;
  }
  if(startIndex+1 < endIndex) {
    if(!bitfield->isBitRangeSet(startIndex+1, endIndex-1)) {
      return false;
    }
  }
  if(startIndex == endIndex) {
    if(hasBlock(startIndex)) {
      return true;
    }
    PieceHandle subPiece = getSubPiece(startIndex);
    if(subPiece.isNull()) {
      return false;
    }
    return subPiece->isRangeComplete(offset, length);
  } else {
    if(!hasBlock(startIndex)) {
      PieceHandle subPiece = getSubPiece(startIndex);
      if(subPiece.isNull()) {
	return false;
      }
      if(!subPiece->isRangeComplete(offset-startIndex*_blockLength, length)) {
	return false;
      }
    }
    if(!hasBlock(endIndex)) {
      PieceHandle subPiece = getSubPiece(endIndex);
      if(subPiece.isNull()) {
	return false;
      }
      if(!subPiece->isRangeComplete(0, offset+length-endIndex*_blockLength)) {
	return false;
      }
    }
    return true;
  }
}

int32_t Piece::getCompletedLength()
{
  int32_t length = 0;
  for(int32_t i = 0; i < countBlock(); ++i) {
    if(hasBlock(i)) {
      length += getBlockLength(i);
    } else {
      PieceHandle subPiece = getSubPiece(i);
      if(!subPiece.isNull()) {
	length += subPiece->getCompletedLength();
      }
    }
  }
  return length;
}

