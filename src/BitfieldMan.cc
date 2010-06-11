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
#include "BitfieldMan.h"

#include <cassert>
#include <cstring>

#include "util.h"
#include "array_fun.h"
#include "bitfield.h"

using namespace aria2::expr;

namespace aria2 {

BitfieldMan::BitfieldMan(size_t blockLength, uint64_t totalLength)
  :_blockLength(blockLength),
   _totalLength(totalLength),
   _bitfieldLength(0),
   _blocks(0),
   _filterEnabled(false),
   _bitfield(0),
   _useBitfield(0),
   _filterBitfield(0),
   _cachedNumMissingBlock(0),
   _cachedNumFilteredBlock(0),
   _cachedCompletedLength(0),
   _cachedFilteredCompletedLength(0),
   _cachedFilteredTotalLength(0)
{
  if(_blockLength > 0 && _totalLength > 0) {
    _blocks = _totalLength/_blockLength+(_totalLength%_blockLength ? 1 : 0);
    _bitfieldLength = _blocks/8+(_blocks%8 ? 1 : 0);
    _bitfield = new unsigned char[_bitfieldLength];
    _useBitfield = new unsigned char[_bitfieldLength];
    memset(_bitfield, 0, _bitfieldLength);
    memset(_useBitfield, 0, _bitfieldLength);
    updateCache();
  }
}

BitfieldMan::BitfieldMan(const BitfieldMan& bitfieldMan)
  :_blockLength(bitfieldMan._blockLength),
   _totalLength(bitfieldMan._totalLength),
   _bitfieldLength(bitfieldMan._bitfieldLength),
   _blocks(bitfieldMan._blocks),
   _filterEnabled(bitfieldMan._filterEnabled),
   _bitfield(new unsigned char[_bitfieldLength]),
   _useBitfield(new unsigned char[_bitfieldLength]),
   _filterBitfield(0),
   _cachedNumMissingBlock(0),
   _cachedNumFilteredBlock(0),
   _cachedCompletedLength(0),
   _cachedFilteredCompletedLength(0),
   _cachedFilteredTotalLength(0)
{
  memcpy(_bitfield, bitfieldMan._bitfield, _bitfieldLength);
  memcpy(_useBitfield, bitfieldMan._useBitfield, _bitfieldLength);
  if(_filterEnabled) {
    _filterBitfield = new unsigned char[_bitfieldLength];
    memcpy(_filterBitfield, bitfieldMan._filterBitfield, _bitfieldLength);
  }
  updateCache();
}

BitfieldMan& BitfieldMan::operator=(const BitfieldMan& bitfieldMan)
{
  if(this != &bitfieldMan) {
    _blockLength = bitfieldMan._blockLength;
    _totalLength = bitfieldMan._totalLength;
    _blocks = bitfieldMan._blocks;
    _bitfieldLength = bitfieldMan._bitfieldLength;
    _filterEnabled = bitfieldMan._filterEnabled;

    delete [] _bitfield;
    _bitfield = new unsigned char[_bitfieldLength];
    memcpy(_bitfield, bitfieldMan._bitfield, _bitfieldLength);

    delete [] _useBitfield;
    _useBitfield = new unsigned char[_bitfieldLength];
    memcpy(_useBitfield, bitfieldMan._useBitfield, _bitfieldLength);

    delete [] _filterBitfield;
    if(_filterEnabled) {
      _filterBitfield = new unsigned char[_bitfieldLength];
      memcpy(_filterBitfield, bitfieldMan._filterBitfield, _bitfieldLength);
    } else {
      _filterBitfield = 0;
    }

    updateCache();
  }
  return *this;
}

BitfieldMan::~BitfieldMan() {
  delete [] _bitfield;
  delete [] _useBitfield;
  delete [] _filterBitfield;
}

size_t BitfieldMan::getBlockLength(size_t index) const
{
  if(index == _blocks-1) {
    return getLastBlockLength();
  } else if(index < _blocks-1) {
    return getBlockLength();
  } else {
    return 0;
  }
}

bool BitfieldMan::hasMissingPiece
(const unsigned char* peerBitfield, size_t length) const
{
  if(_bitfieldLength != length) {
    return false;
  }
  bool retval = false;
  for(size_t i = 0; i < _bitfieldLength; ++i) {
    unsigned char temp = peerBitfield[i] & ~_bitfield[i];
    if(_filterEnabled) {
      temp &= _filterBitfield[i];
    }
    if(temp&0xff) {
      retval = true;
      break;
    }
  }
  return retval;
}

bool BitfieldMan::getFirstMissingUnusedIndex(size_t& index) const
{
  if(_filterEnabled) {
    return bitfield::getFirstMissingIndex
      (index, ~array(_bitfield)&~array(_useBitfield)&array(_filterBitfield),
       _blocks);
  } else {
    return bitfield::getFirstMissingIndex
      (index, ~array(_bitfield)&~array(_useBitfield), _blocks);
  }
}

size_t BitfieldMan::getFirstNMissingUnusedIndex
(std::vector<size_t>& out, size_t n) const
{
  if(_filterEnabled) {
    return bitfield::getFirstNMissingIndex
      (std::back_inserter(out), n,
       ~array(_bitfield)&~array(_useBitfield)&array(_filterBitfield), _blocks);
  } else {
    return bitfield::getFirstNMissingIndex
      (std::back_inserter(out), n,
       ~array(_bitfield)&~array(_useBitfield), _blocks);
  }
}

bool BitfieldMan::getFirstMissingIndex(size_t& index) const
{
  if(_filterEnabled) {
    return bitfield::getFirstMissingIndex
      (index, ~array(_bitfield)&array(_filterBitfield), _blocks);
  } else {
    return bitfield::getFirstMissingIndex(index, ~array(_bitfield), _blocks);
  }
}

template<typename Array>
static size_t getStartIndex(size_t index, const Array& bitfield, size_t blocks) {
  while(index < blocks && bitfield::test(bitfield, blocks, index)) {
    ++index;
  }
  if(blocks <= index) {
    return blocks;
  } else {
    return index;
  }
}

template<typename Array>
static size_t getEndIndex(size_t index, const Array& bitfield, size_t blocks) {
  while(index < blocks && !bitfield::test(bitfield, blocks, index)) {
    ++index;
  }
  return index;
}

template<typename Array>
static bool getSparseMissingUnusedIndex
(size_t& index,
 const Array& bitfield,
 const unsigned char* useBitfield,
 size_t blocks)
{
  BitfieldMan::Range maxRange;
  BitfieldMan::Range currentRange;
  {
    size_t nextIndex = 0;
    while(nextIndex < blocks) {
      currentRange.startIndex =
        getStartIndex(nextIndex, bitfield, blocks);
      if(currentRange.startIndex == blocks) {
        break;
      }
      currentRange.endIndex =
        getEndIndex(currentRange.startIndex, bitfield, blocks);
      if(maxRange < currentRange) {
        maxRange = currentRange;
      }
      nextIndex = currentRange.endIndex;
    }
  }
  if(maxRange.getSize()) {
    if(maxRange.startIndex == 0) {
      index = 0;
    } else if(bitfield::test(useBitfield, blocks, maxRange.startIndex-1)) {
      index = maxRange.getMidIndex();
    } else {
      index = maxRange.startIndex;
    }
    return true;
  } else {
    return false;
  }
}

bool BitfieldMan::getSparseMissingUnusedIndex
(size_t& index,
 const unsigned char* ignoreBitfield,
 size_t ignoreBitfieldLength) const
{
  if(_filterEnabled) {
    return aria2::getSparseMissingUnusedIndex
      (index, array(ignoreBitfield)|~array(_filterBitfield)|array(_bitfield)|array(_useBitfield),
       _useBitfield, _blocks);
  } else {
    return aria2::getSparseMissingUnusedIndex
      (index, array(ignoreBitfield)|array(_bitfield)|array(_useBitfield),
       _useBitfield, _blocks);
  }
}

template<typename Array>
static bool copyBitfield(unsigned char* dst, const Array& src, size_t blocks)
{
  unsigned char bits = 0;
  size_t len = (blocks+7)/8;
  for(size_t i = 0; i < len-1; ++i) {
    dst[i] = src[i];
    bits |= dst[i];
  }
  dst[len-1] = src[len-1]&bitfield::lastByteMask(blocks);
  bits |= dst[len-1];
  return bits != 0;
}

bool BitfieldMan::getAllMissingIndexes(unsigned char* misbitfield, size_t len)
  const
{
  assert(len == _bitfieldLength);
  if(_filterEnabled) {
    return copyBitfield
      (misbitfield, ~array(_bitfield)&array(_filterBitfield), _blocks);
  } else {
    return copyBitfield(misbitfield, ~array(_bitfield), _blocks);
  }
}

bool BitfieldMan::getAllMissingIndexes(unsigned char* misbitfield, size_t len,
                                       const unsigned char* peerBitfield,
                                       size_t peerBitfieldLength) const
{
  assert(len == _bitfieldLength);
  if(_bitfieldLength != peerBitfieldLength) {
    return false;
  }
  if(_filterEnabled) {
    return copyBitfield
      (misbitfield,
       ~array(_bitfield)&array(peerBitfield)&array(_filterBitfield),
       _blocks);
  } else {
    return copyBitfield
      (misbitfield, ~array(_bitfield)&array(peerBitfield),
       _blocks);
  }
}

bool BitfieldMan::getAllMissingUnusedIndexes(unsigned char* misbitfield,
                                             size_t len,
                                             const unsigned char* peerBitfield,
                                             size_t peerBitfieldLength) const
{
  assert(len == _bitfieldLength);
  if(_bitfieldLength != peerBitfieldLength) {
    return false;
  }
  if(_filterEnabled) {
    return copyBitfield
      (misbitfield,
       ~array(_bitfield)&~array(_useBitfield)&array(peerBitfield)&
       array(_filterBitfield),
       _blocks);
  } else {
    return copyBitfield
      (misbitfield,
       ~array(_bitfield)&~array(_useBitfield)&array(peerBitfield),
       _blocks);
  }
}

size_t BitfieldMan::countMissingBlock() const {
  return _cachedNumMissingBlock;
}

size_t BitfieldMan::countMissingBlockNow() const {
  if(_filterEnabled) {
    array_ptr<unsigned char> temp(new unsigned char[_bitfieldLength]);
    for(size_t i = 0; i < _bitfieldLength; ++i) {
      temp[i] = _bitfield[i]&_filterBitfield[i];
    }
    size_t count =  bitfield::countSetBit(_filterBitfield, _blocks)-
      bitfield::countSetBit(temp, _blocks);
    return count;
  } else {
    return _blocks-bitfield::countSetBit(_bitfield, _blocks);
  }
}

size_t BitfieldMan::countFilteredBlockNow() const {
  if(_filterEnabled) {
    return bitfield::countSetBit(_filterBitfield, _blocks);
  } else {
    return 0;
  }
}

bool BitfieldMan::setBitInternal(unsigned char* bitfield, size_t index, bool on) {
  if(_blocks <= index) { return false; }
  unsigned char mask = 128 >> (index%8);
  if(on) {
    bitfield[index/8] |= mask;
  } else {
    bitfield[index/8] &= ~mask;
  }
  return true;
}

bool BitfieldMan::setUseBit(size_t index) {
  return setBitInternal(_useBitfield, index, true);
}

bool BitfieldMan::unsetUseBit(size_t index) {
  return setBitInternal(_useBitfield, index, false);
}

bool BitfieldMan::setBit(size_t index) {
  bool b = setBitInternal(_bitfield, index, true);
  updateCache();
  return b;
}

bool BitfieldMan::unsetBit(size_t index) {
  bool b = setBitInternal(_bitfield, index, false);
  updateCache();
  return b;
}

bool BitfieldMan::isFilteredAllBitSet() const {
  if(_filterEnabled) {
    for(size_t i = 0; i < _bitfieldLength; ++i) {
      if((_bitfield[i]&_filterBitfield[i]) != _filterBitfield[i]) {
        return false;
      }
    }
    return true;
  } else {
    return isAllBitSet();
  }
}

static bool testAllBitSet
(const unsigned char* bitfield, size_t length, size_t blocks)
{
  if(length == 0) {
    return true;
  }
  for(size_t i = 0; i < length-1; ++i) {
    if(bitfield[i] != 0xff) {
      return false;
    }
  }
  return bitfield[length-1] == bitfield::lastByteMask(blocks);
}

bool BitfieldMan::isAllBitSet() const
{
  return testAllBitSet(_bitfield, _bitfieldLength, _blocks);
}

bool BitfieldMan::isAllFilterBitSet() const
{
  if(!_filterBitfield) {
    return false;
  }
  return testAllBitSet(_filterBitfield, _bitfieldLength, _blocks);
}

bool BitfieldMan::isBitSet(size_t index) const
{
  return bitfield::test(_bitfield, _blocks, index);
}

bool BitfieldMan::isUseBitSet(size_t index) const
{
  return bitfield::test(_useBitfield, _blocks, index);
}

void BitfieldMan::setBitfield(const unsigned char* bitfield, size_t bitfieldLength) {
  if(_bitfieldLength != bitfieldLength) {
    return;
  }
  memcpy(_bitfield, bitfield, _bitfieldLength);
  memset(_useBitfield, 0, _bitfieldLength);
  updateCache();
}

void BitfieldMan::clearAllBit() {
  memset(_bitfield, 0, _bitfieldLength);
  updateCache();
}

void BitfieldMan::setAllBit() {
  for(size_t i = 0; i < _blocks; ++i) {
    setBitInternal(_bitfield, i, true);
  }
  updateCache();
}

void BitfieldMan::clearAllUseBit() {
  memset(_useBitfield, 0, _bitfieldLength);
  updateCache();
}

void BitfieldMan::setAllUseBit() {
  for(size_t i = 0; i < _blocks; ++i) {
    setBitInternal(_useBitfield, i, true);
  }
}

bool BitfieldMan::setFilterBit(size_t index) {
  return setBitInternal(_filterBitfield, index, true);
}

void BitfieldMan::ensureFilterBitfield()
{
  if(!_filterBitfield) {
    _filterBitfield = new unsigned char[_bitfieldLength];
    memset(_filterBitfield, 0, _bitfieldLength);
  }
}

void BitfieldMan::addFilter(uint64_t offset, uint64_t length) {
  ensureFilterBitfield();
  if(length > 0) {
    size_t startBlock = offset/_blockLength;
    size_t endBlock = (offset+length-1)/_blockLength;
    for(size_t i = startBlock; i <= endBlock && i < _blocks; i++) {
      setFilterBit(i);
    }
  }
  updateCache();
}

void BitfieldMan::removeFilter(uint64_t offset, uint64_t length) {
  ensureFilterBitfield();
  if(length > 0) {
    size_t startBlock = offset/_blockLength;
    size_t endBlock = (offset+length-1)/_blockLength;
    for(size_t i = startBlock; i <= endBlock && i < _blocks; i++) {
      setBitInternal(_filterBitfield, i, false);
    }
  }
  updateCache();
}

void BitfieldMan::addNotFilter(uint64_t offset, uint64_t length)
{
  ensureFilterBitfield();
  if(length > 0 && _blocks > 0) {
    size_t startBlock = offset/_blockLength;
    if(_blocks <= startBlock) {
      startBlock = _blocks;
    }
    size_t endBlock = (offset+length-1)/_blockLength;
    for(size_t i = 0; i < startBlock; ++i) {
      setFilterBit(i);
    }
    for(size_t i = endBlock+1; i < _blocks; ++i) {
      setFilterBit(i);
    }
  }
  updateCache();
}

void BitfieldMan::enableFilter() {
  ensureFilterBitfield();
  _filterEnabled = true;
  updateCache();
}

void BitfieldMan::disableFilter() {
  _filterEnabled = false;
  updateCache();
}

void BitfieldMan::clearFilter() {
  if(_filterBitfield) {
    delete [] _filterBitfield;
    _filterBitfield = 0;
  }
  _filterEnabled = false;
  updateCache();
}

uint64_t BitfieldMan::getFilteredTotalLengthNow() const {
  if(!_filterBitfield) {
    return 0;
  }
  size_t filteredBlocks = bitfield::countSetBit(_filterBitfield, _blocks);
  if(filteredBlocks == 0) {
    return 0;
  }
  if(bitfield::test(_filterBitfield, _blocks, _blocks-1)) {
    return ((uint64_t)filteredBlocks-1)*_blockLength+getLastBlockLength();
  } else {
    return ((uint64_t)filteredBlocks)*_blockLength;
  }
}

uint64_t BitfieldMan::getCompletedLength(bool useFilter) const {
  unsigned char* temp;
  if(useFilter) {
    temp = new unsigned char[_bitfieldLength];
    for(size_t i = 0; i < _bitfieldLength; ++i) {
      temp[i] = _bitfield[i];
      if(_filterEnabled) {
        temp[i] &= _filterBitfield[i];
      }
    }
  } else {
    temp = _bitfield;
  }
  size_t completedBlocks = bitfield::countSetBit(temp, _blocks);
  uint64_t completedLength = 0;
  if(completedBlocks == 0) {
    completedLength = 0;
  } else {
    if(bitfield::test(temp, _blocks, _blocks-1)) {
      completedLength = ((uint64_t)completedBlocks-1)*_blockLength+getLastBlockLength();
    } else {
      completedLength = ((uint64_t)completedBlocks)*_blockLength;
    }
  }
  if(useFilter) {
    delete [] temp;
  }
  return completedLength;
}

uint64_t BitfieldMan::getCompletedLengthNow() const {
  return getCompletedLength(false);
}

uint64_t BitfieldMan::getFilteredCompletedLengthNow() const {
  return getCompletedLength(true);
}

void BitfieldMan::updateCache()
{
  _cachedNumMissingBlock = countMissingBlockNow();
  _cachedNumFilteredBlock = countFilteredBlockNow();
  _cachedFilteredTotalLength = getFilteredTotalLengthNow();
  _cachedCompletedLength = getCompletedLengthNow();
  _cachedFilteredCompletedLength = getFilteredCompletedLengthNow();
}

bool BitfieldMan::isBitRangeSet(size_t startIndex, size_t endIndex) const
{
  for(size_t i =  startIndex; i <= endIndex; ++i) {
    if(!isBitSet(i)) {
      return false;
    }
  }
  return true;
}

void BitfieldMan::unsetBitRange(size_t startIndex, size_t endIndex)
{
  for(size_t i = startIndex; i <= endIndex; ++i) {
    unsetBit(i);
  }
  updateCache();
}

void BitfieldMan::setBitRange(size_t startIndex, size_t endIndex)
{
  for(size_t i = startIndex; i <= endIndex; ++i) {
    setBit(i);
  }
  updateCache();
}

bool BitfieldMan::isBitSetOffsetRange(uint64_t offset, uint64_t length) const
{
  if(length <= 0) {
    return false;
  }
  if(_totalLength <= offset) {
    return false;
  }
  if(_totalLength < offset+length) {
    length = _totalLength-offset;
  }
  size_t startBlock = offset/_blockLength;
  size_t endBlock = (offset+length-1)/_blockLength;
  for(size_t i = startBlock; i <= endBlock; i++) {
    if(!isBitSet(i)) {
      return false;
    }
  }
  return true;
}

uint64_t BitfieldMan::getMissingUnusedLength(size_t startingIndex) const
{
  if(startingIndex < 0 || _blocks <= startingIndex) {
    return 0;
  }
  uint64_t length = 0;
  for(size_t i = startingIndex; i < _blocks; ++i) {
    if(isBitSet(i) || isUseBitSet(i)) {
      break;
    }
    length += getBlockLength(i);
  }
  return length;
}

} // namespace aria2
