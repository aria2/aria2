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
# include "MessageDigest.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

Piece::Piece():index_(0), length_(0), blockLength_(BLOCK_LENGTH), bitfield_(0)
#ifdef ENABLE_MESSAGE_DIGEST
              , nextBegin_(0)
#endif // ENABLE_MESSAGE_DIGEST
{}

Piece::Piece(size_t index, size_t length, size_t blockLength):
  index_(index), length_(length), blockLength_(blockLength),
  bitfield_(new BitfieldMan(blockLength_, length))
#ifdef ENABLE_MESSAGE_DIGEST
                                                             , nextBegin_(0)
#endif // ENABLE_MESSAGE_DIGEST
{}

Piece::~Piece()
{
  delete bitfield_;
}

void Piece::completeBlock(size_t blockIndex) {
  bitfield_->setBit(blockIndex);
  bitfield_->unsetUseBit(blockIndex);
}

void Piece::clearAllBlock() {
  bitfield_->clearAllBit();
  bitfield_->clearAllUseBit();
}

void Piece::setAllBlock() {
  bitfield_->setAllBit();
}

bool Piece::pieceComplete() const {
  return bitfield_->isAllBitSet();
}

size_t Piece::countBlock() const
{
  return bitfield_->countBlock();
}

size_t Piece::getBlockLength(size_t index) const
{
  return bitfield_->getBlockLength(index);
}

size_t Piece::getBlockLength() const
{
  return bitfield_->getBlockLength();
}

const unsigned char* Piece::getBitfield() const
{
  return bitfield_->getBitfield();
}

size_t Piece::getBitfieldLength() const
{
  return bitfield_->getBitfieldLength();
}

bool Piece::isBlockUsed(size_t index) const
{
  return bitfield_->isUseBitSet(index);
}

void Piece::cancelBlock(size_t blockIndex) {
  bitfield_->unsetUseBit(blockIndex);
}

size_t Piece::countCompleteBlock() const
{
  return bitfield_->countBlock()-bitfield_->countMissingBlock();
}

size_t Piece::countMissingBlock() const
{
  return bitfield_->countMissingBlock();
}

bool Piece::hasBlock(size_t blockIndex) const
{
  return bitfield_->isBitSet(blockIndex);
}

bool Piece::getMissingUnusedBlockIndex(size_t& index) const
{
  if(bitfield_->getFirstMissingUnusedIndex(index)) {
    bitfield_->setUseBit(index);
    return true;
  } else {
    return false;
  }
}

size_t Piece::getMissingUnusedBlockIndex
(std::vector<size_t>& indexes, size_t n) const
{
  size_t num = bitfield_->getFirstNMissingUnusedIndex(indexes, n);
  if(num) {
    for(std::vector<size_t>::const_iterator i = indexes.end()-num,
          eoi = indexes.end(); i != eoi; ++i) {
      bitfield_->setUseBit(*i);
    }
  }
  return num;
}

bool Piece::getFirstMissingBlockIndexWithoutLock(size_t& index) const
{
  return bitfield_->getFirstMissingIndex(index);
}

bool Piece::getAllMissingBlockIndexes
(unsigned char* misbitfield, size_t mislen) const
{
  return bitfield_->getAllMissingIndexes(misbitfield, mislen);
}

std::string Piece::toString() const {
  return strconcat("piece: index=", util::itos(index_),
                   ", length=", util::itos(length_));
}

void Piece::reconfigure(size_t length)
{
  delete bitfield_;
  length_ = length;
  bitfield_ = new BitfieldMan(blockLength_, length_);
}

void Piece::setBitfield(const unsigned char* bitfield, size_t len)
{
  bitfield_->setBitfield(bitfield, len);
}

size_t Piece::getCompletedLength()
{
  return bitfield_->getCompletedLength();
}

#ifdef ENABLE_MESSAGE_DIGEST

void Piece::setHashAlgo(const std::string& algo)
{
  hashAlgo_ = algo;
}

bool Piece::updateHash
(uint32_t begin, const unsigned char* data, size_t dataLength)
{
  if(hashAlgo_.empty()) {
    return false;
  }
  if(begin == nextBegin_ && nextBegin_+dataLength <= length_) {
    if(!mdctx_) {
      mdctx_ = MessageDigest::create(hashAlgo_);
    }
    mdctx_->update(data, dataLength);
    nextBegin_ += dataLength;
    return true;
  } else {
    return false;
  }
}

bool Piece::isHashCalculated() const
{
  return mdctx_ && nextBegin_ == length_;
}

std::string Piece::getHashString()
{
  if(!mdctx_) {
    return A2STR::NIL;
  } else {
    std::string hash = mdctx_->hexDigest();
    destroyHashContext();
    return hash;
  }
}

void Piece::destroyHashContext()
{
  mdctx_.reset();
  nextBegin_ = 0;
}

#endif // ENABLE_MESSAGE_DIGEST

} // namespace aria2
