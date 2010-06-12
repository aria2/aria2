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
#include "Piece.h"
#include "util.h"
#include "BitfieldMan.h"
#include "A2STR.h"
#include "util.h"
#include "a2functional.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "messageDigest.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

Piece::Piece():_index(0), _length(0), _blockLength(BLOCK_LENGTH), _bitfield(0)
#ifdef ENABLE_MESSAGE_DIGEST
              , _nextBegin(0)
#endif // ENABLE_MESSAGE_DIGEST
{}

Piece::Piece(size_t index, size_t length, size_t blockLength):
  _index(index), _length(length), _blockLength(blockLength),
  _bitfield(new BitfieldMan(_blockLength, length))
#ifdef ENABLE_MESSAGE_DIGEST
                                                             , _nextBegin(0)
#endif // ENABLE_MESSAGE_DIGEST
{}

Piece::Piece(const Piece& piece) {
  _index = piece._index;
  _length = piece._length;
  _blockLength = piece._blockLength;
  if(piece._bitfield == 0) {
    _bitfield = 0;
  } else {
    _bitfield = new BitfieldMan(*piece._bitfield);
  }
#ifdef ENABLE_MESSAGE_DIGEST
  _nextBegin = piece._nextBegin;
  // TODO Is this OK?
  _mdctx = piece._mdctx;
#endif // ENABLE_MESSAGE_DIGEST
}

Piece::~Piece()
{
  delete _bitfield;
}

Piece& Piece::operator=(const Piece& piece)
{
  if(this != &piece) {
    _index = piece._index;
    _length = piece._length;
    delete _bitfield;
    if(piece._bitfield) {
      _bitfield = new BitfieldMan(*piece._bitfield);
    } else {
      _bitfield = 0;
    }
  }
  return *this;
}

void Piece::completeBlock(size_t blockIndex) {
  _bitfield->setBit(blockIndex);
  _bitfield->unsetUseBit(blockIndex);
}

void Piece::clearAllBlock() {
  _bitfield->clearAllBit();
  _bitfield->clearAllUseBit();
}

void Piece::setAllBlock() {
  _bitfield->setAllBit();
}

bool Piece::pieceComplete() const {
  return _bitfield->isAllBitSet();
}

size_t Piece::countBlock() const
{
  return _bitfield->countBlock();
}

size_t Piece::getBlockLength(size_t index) const
{
  return _bitfield->getBlockLength(index);
}

size_t Piece::getBlockLength() const
{
  return _bitfield->getBlockLength();
}

const unsigned char* Piece::getBitfield() const
{
  return _bitfield->getBitfield();
}

size_t Piece::getBitfieldLength() const
{
  return _bitfield->getBitfieldLength();
}

bool Piece::isBlockUsed(size_t index) const
{
  return _bitfield->isUseBitSet(index);
}

void Piece::cancelBlock(size_t blockIndex) {
  _bitfield->unsetUseBit(blockIndex);
}

size_t Piece::countCompleteBlock() const
{
  return _bitfield->countBlock()-_bitfield->countMissingBlock();
}

size_t Piece::countMissingBlock() const
{
  return _bitfield->countMissingBlock();
}

bool Piece::hasBlock(size_t blockIndex) const
{
  return _bitfield->isBitSet(blockIndex);
}

bool Piece::getMissingUnusedBlockIndex(size_t& index) const
{
  if(_bitfield->getFirstMissingUnusedIndex(index)) {
    _bitfield->setUseBit(index);
    return true;
  } else {
    return false;
  }
}

size_t Piece::getMissingUnusedBlockIndex
(std::vector<size_t>& indexes, size_t n) const
{
  size_t num = _bitfield->getFirstNMissingUnusedIndex(indexes, n);
  if(num) {
    for(std::vector<size_t>::const_iterator i = indexes.end()-num,
          eoi = indexes.end(); i != eoi; ++i) {
      _bitfield->setUseBit(*i);
    }
  }
  return num;
}

bool Piece::getFirstMissingBlockIndexWithoutLock(size_t& index) const
{
  return _bitfield->getFirstMissingIndex(index);
}

bool Piece::getAllMissingBlockIndexes
(unsigned char* misbitfield, size_t mislen) const
{
  return _bitfield->getAllMissingIndexes(misbitfield, mislen);
}

std::string Piece::toString() const {
  return strconcat("piece: index=", util::itos(_index),
                   ", length=", util::itos(_length));
}

void Piece::reconfigure(size_t length)
{
  delete _bitfield;
  _length = length;
  _bitfield = new BitfieldMan(_blockLength, _length);
}

void Piece::setBitfield(const unsigned char* bitfield, size_t len)
{
  _bitfield->setBitfield(bitfield, len);
}

size_t Piece::getCompletedLength()
{
  return _bitfield->getCompletedLength();
}

#ifdef ENABLE_MESSAGE_DIGEST

void Piece::setHashAlgo(const std::string& algo)
{
  _hashAlgo = algo;
}

bool Piece::updateHash
(uint32_t begin, const unsigned char* data, size_t dataLength)
{
  if(_hashAlgo.empty()) {
    return false;
  }
  if(begin == _nextBegin && _nextBegin+dataLength <= _length) {

    if(_mdctx.isNull()) {
      _mdctx.reset(new MessageDigestContext());      
      _mdctx->trySetAlgo(_hashAlgo);
      _mdctx->digestInit();
    }

    _mdctx->digestUpdate(data, dataLength);
    _nextBegin += dataLength;
    return true;
  } else {
    return false;
  }
}

bool Piece::isHashCalculated() const
{
  return !_mdctx.isNull() && _nextBegin == _length;
}

std::string Piece::getHashString()
{
  if(_mdctx.isNull()) {
    return A2STR::NIL;
  } else {
    std::string hash = util::toHex(_mdctx->digestFinal());
    destroyHashContext();
    return hash;
  }
}

void Piece::destroyHashContext()
{
  _mdctx.reset();
  _nextBegin = 0;
}

#endif // ENABLE_MESSAGE_DIGEST

} // namespace aria2
