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
#include "BitfieldMan.h"

namespace aria2 {

Piece::Piece():index(0), length(0), _blockLength(BLOCK_LENGTH), bitfield(0) {}

Piece::Piece(size_t index, size_t length, size_t blockLength):index(index), length(length), _blockLength(blockLength) {
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

Piece::~Piece()
{
  delete bitfield;
}

Piece& Piece::operator=(const Piece& piece)
{
  if(this != &piece) {
    index = piece.index;
    length = piece.length;
    delete bitfield;
    if(piece.bitfield) {
      bitfield = new BitfieldMan(*piece.bitfield);
    } else {
      bitfield = 0;
    }
  }
  return *this;
}

bool Piece::operator==(const Piece& piece) const
{
  return index == piece.index;
}

void Piece::completeBlock(size_t blockIndex) {
  bitfield->setBit(blockIndex);
  bitfield->unsetUseBit(blockIndex);
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

size_t Piece::countBlock() const
{
  return bitfield->countBlock();
}

size_t Piece::getBlockLength(size_t index) const
{
  return bitfield->getBlockLength(index);
}

size_t Piece::getBlockLength() const
{
  return bitfield->getBlockLength();
}

const unsigned char* Piece::getBitfield() const
{
  return bitfield->getBitfield();
}

size_t Piece::getBitfieldLength() const
{
  return bitfield->getBitfieldLength();
}

bool Piece::isBlockUsed(size_t index) const
{
  return bitfield->isUseBitSet(index);
}

void Piece::cancelBlock(size_t blockIndex) {
  bitfield->unsetUseBit(blockIndex);
}

size_t Piece::countCompleteBlock() const
{
  return bitfield->countBlock()-bitfield->countMissingBlock();
}

bool Piece::hasBlock(size_t blockIndex) const
{
  return bitfield->isBitSet(blockIndex);
}

bool Piece::getMissingUnusedBlockIndex(size_t& index) const
{
  if(bitfield->getFirstMissingUnusedIndex(index)) {
    bitfield->setUseBit(index);
    return true;
  } else {
    return false;
  }
}

bool Piece::getMissingBlockIndex(size_t& index) const
{
  if(bitfield->getMissingIndex(index)) {
    bitfield->setUseBit(index);
    return true;
  } else {
    return false;
  }
}

bool Piece::getFirstMissingBlockIndexWithoutLock(size_t& index) const
{
  return bitfield->getFirstMissingIndex(index);
}

std::deque<size_t> Piece::getAllMissingBlockIndexes() const {
  return bitfield->getAllMissingIndexes();
}

std::string Piece::toString() const {
  return "piece: index="+Util::itos(index)+", length="+Util::itos(length);
}

void Piece::reconfigure(size_t length)
{
  delete bitfield;
  this->length = length;
  bitfield =
    BitfieldManFactory::getFactoryInstance()->createBitfieldMan(_blockLength, length);
}

void Piece::setBitfield(const unsigned char* bitfield, size_t len)
{
  this->bitfield->setBitfield(bitfield, len);
}

size_t Piece::getCompletedLength()
{
  return bitfield->getCompletedLength();
}

} // namespace aria2
