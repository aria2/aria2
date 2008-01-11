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
#include "BitfieldMan.h"
#include "Util.h"
#include "array_fun.h"
#include <string.h>

BitfieldMan::BitfieldMan(int32_t blockLength, int64_t totalLength)
  :blockLength(blockLength),
   totalLength(totalLength),
   bitfield(0),
   useBitfield(0),
   filterBitfield(0),
   bitfieldLength(0),
   blocks(0),
   filterEnabled(false),
   randomizer(0),
   cachedNumMissingBlock(0),
   cachedNumFilteredBlock(0),
   cachedCompletedLength(0),
   cachedFilteredComletedLength(0),
   cachedFilteredTotalLength(0)
{
  if(blockLength > 0 && totalLength > 0) {
    blocks = totalLength/blockLength+(totalLength%blockLength ? 1 : 0);
    bitfieldLength = blocks/8+(blocks%8 ? 1 : 0);
    bitfield = new unsigned char[bitfieldLength];
    useBitfield = new unsigned char[bitfieldLength];
    memset(bitfield, 0, bitfieldLength);
    memset(useBitfield, 0, bitfieldLength);
    updateCache();
  }
}

BitfieldMan::BitfieldMan(const BitfieldMan& bitfieldMan)
  :blockLength(0),
   totalLength(0),
   bitfield(0),
   useBitfield(0),
   filterBitfield(0),
   bitfieldLength(0),
   blocks(0),
   filterEnabled(false),
   randomizer(0),
   cachedNumMissingBlock(0),
   cachedNumFilteredBlock(0),
   cachedCompletedLength(0),
   cachedFilteredComletedLength(0),
   cachedFilteredTotalLength(0)
{
  blockLength = bitfieldMan.blockLength;
  totalLength = bitfieldMan.totalLength;
  blocks = bitfieldMan.blocks;
  bitfieldLength = bitfieldMan.bitfieldLength;
  bitfield = new unsigned char[bitfieldLength];
  useBitfield = new unsigned char[bitfieldLength];
  memcpy(bitfield, bitfieldMan.bitfield, bitfieldLength);
  memcpy(useBitfield, bitfieldMan.useBitfield, bitfieldLength);
  filterEnabled = bitfieldMan.filterEnabled;
  if(filterBitfield) {
    filterBitfield = new unsigned char[bitfieldLength];
    memcpy(filterBitfield, bitfieldMan.filterBitfield, bitfieldLength);
  } else {
    filterBitfield = 0;
  }
  this->randomizer = bitfieldMan.randomizer;
  updateCache();
}

BitfieldMan::~BitfieldMan() {
  delete [] bitfield;
  delete [] useBitfield;
  delete [] filterBitfield;
}

int32_t BitfieldMan::countSetBit(const unsigned char* bitfield, int32_t len) const {
  int32_t count = 0;
  int32_t size = sizeof(int32_t);
  for(int32_t i = 0; i < len/size; ++i) {
    count += Util::countBit(*(uint32_t*)&bitfield[i*size]);
  }
  for(int32_t i = len-len%size; i < len; i++) {
    count += Util::countBit((uint32_t)bitfield[i]);
  }
  return count;
}

int32_t BitfieldMan::getNthBitIndex(const unsigned char bitfield, int32_t nth) const {
  int32_t index = -1;
  for(int bs = 7; bs >= 0; bs--) {
    unsigned char mask = 1 << bs;
    if(bitfield & mask) {
      nth--;
      if(nth == 0) {
	index = 7-bs;
	break;
      }
    }
  }
  return index;
}

template<typename Array>
int32_t
BitfieldMan::getMissingIndexRandomly(const Array& bitfield,
				     int32_t bitfieldLength) const
{
  /*
  int32_t byte = (int32_t)(((double)bitfieldLength)*
			   randomizer->getRandomNumber()/
			   (randomizer->getMaxRandomNumber()+1.0));
  */
  int32_t byte = randomizer->getRandomNumber(bitfieldLength);

  unsigned char lastMask = 0;
  // the number of bytes in the last byte of bitfield
  int32_t lastByteLength = totalLength%(blockLength*8);
  // the number of block in the last byte of bitfield
  int32_t lastBlockCount = DIV_FLOOR(lastByteLength, blockLength);
  for(int32_t i = 0; i < lastBlockCount; ++i) {
    lastMask >>= 1;
    lastMask |= 0x80;
  }
  for(int32_t i = 0; i < bitfieldLength; ++i) {
    unsigned char mask;
    if(byte == bitfieldLength-1) {
      mask = lastMask;
    } else {
      mask = 0xff;
    }
    if(bitfield[byte]&mask) {
      int32_t index = byte*8+getNthBitIndex(bitfield[byte], 1);
      return index;
    }
    byte++;
    if(byte == bitfieldLength) {
      byte = 0;
    }
  }
  return -1;
}

bool BitfieldMan::hasMissingPiece(const unsigned char* peerBitfield, int32_t length) const {
  if(bitfieldLength != length) {
    return false;
  }
  bool retval = false;
  for(int32_t i = 0; i < bitfieldLength; ++i) {
    unsigned char temp = peerBitfield[i] & ~bitfield[i];
    if(filterEnabled) {
      temp &= filterBitfield[i];
    }
    if(temp&0xff) {
      retval = true;
      break;
    }
  }
  return retval;
}

int32_t BitfieldMan::getMissingIndex(const unsigned char* peerBitfield, int32_t length) const {
  if(bitfieldLength != length) {
    return -1;
  }
  array_fun<unsigned char> bf = array_and(array_negate(bitfield), peerBitfield);
  if(filterEnabled) {
    bf = array_and(bf, filterBitfield);
  }
  return getMissingIndexRandomly(bf, bitfieldLength);
}

int32_t BitfieldMan::getMissingUnusedIndex(const unsigned char* peerBitfield, int32_t length) const {
  if(bitfieldLength != length) {
    return -1;
  }
  array_fun<unsigned char> bf = array_and(array_and(array_negate(bitfield),
						    array_negate(useBitfield)),
					  peerBitfield);
  if(filterEnabled) {
    bf = array_and(bf, filterBitfield);
  }
  return getMissingIndexRandomly(bf, bitfieldLength);
}

template<typename Array>
int32_t BitfieldMan::getFirstMissingIndex(const Array& bitfield, int32_t bitfieldLength) const
{
  for(int32_t i = 0; i < bitfieldLength; ++i) {
    int32_t base = i*8;
    for(int32_t bi = 0; bi < 8 && base+bi < blocks; ++bi) {
      unsigned char mask = 128 >> bi;
      if(bitfield[i] & mask) {
	return base+bi;
      }
    }
  }
  return -1;
}

int32_t BitfieldMan::getFirstMissingUnusedIndex() const {
  array_fun<unsigned char> bf = array_and(array_negate(bitfield),
					  array_negate(useBitfield));
  if(filterEnabled) {
    bf = array_and(bf, filterBitfield);
  }
  return getFirstMissingIndex(bf, bitfieldLength);
}

int32_t BitfieldMan::getFirstMissingIndex() const
{
  array_fun<unsigned char> bf = array_negate(bitfield);
  if(filterEnabled) {
    bf = array_and(bf, filterBitfield);
  }
  return getFirstMissingIndex(bf, bitfieldLength);
}

int32_t BitfieldMan::getMissingIndex() const {
  array_fun<unsigned char> bf = array_negate(bitfield);
  if(filterEnabled) {
    bf = array_and(bf, filterBitfield);
  }
  return getMissingIndexRandomly(bf, bitfieldLength);
}

int32_t BitfieldMan::getMissingUnusedIndex() const {
  array_fun<unsigned char> bf = array_and(array_negate(bitfield),
					  array_negate(useBitfield));
  if(filterEnabled) {
    bf = array_and(bf, filterBitfield);
  }
  return getMissingIndexRandomly(bf, bitfieldLength);
}

// [startIndex, endIndex)
class Range {
public:
  int32_t startIndex;
  int32_t endIndex;
  Range(int32_t startIndex = 0, int32_t endIndex = 0):startIndex(startIndex),
						      endIndex(endIndex) {}
  
  int32_t getSize() const {
    return endIndex-startIndex;
  }

  int32_t getMidIndex() const {
    return (endIndex-startIndex)/2+startIndex;
  }

  bool operator<(const Range& range) const {
    return getSize() < range.getSize();
  }
};

int32_t BitfieldMan::getStartIndex(int32_t index) const {
  while(index < blocks && (isUseBitSet(index) || isBitSet(index))) {
    index++;
  }
  if((int32_t)blocks <= index) {
    return -1;
  } else {
    return index;
  }
}

int32_t BitfieldMan::getEndIndex(int32_t index) const {
  while(index < blocks && (!isUseBitSet(index) && !isBitSet(index))) {
    index++;
  }
  return index;
}

int32_t BitfieldMan::getSparseMissingUnusedIndex() const {
  Range maxRange;
  int32_t index = 0;
  Range currentRange;
  while(index < blocks) {
    currentRange.startIndex = getStartIndex(index);
    if(currentRange.startIndex == -1) {
      break;
    }
    currentRange.endIndex = getEndIndex(currentRange.startIndex);
    if(maxRange < currentRange) {
      maxRange = currentRange;
    }
    index = currentRange.endIndex;
  }
  if(maxRange.getSize()) {
    if(maxRange.startIndex == 0) {
      return 0;
    } else if(isUseBitSet(maxRange.startIndex-1)) {
      return maxRange.getMidIndex();      
    } else {
      return maxRange.startIndex;
    }
  } else {
    return -1;
  }
}

template<typename Array>
BlockIndexes BitfieldMan::getAllMissingIndexes(const Array& bitfield, int32_t bitfieldLength) const
{
  BlockIndexes missingIndexes;
  for(int32_t i = 0; i < bitfieldLength; ++i) {
    int32_t base = i*8;
    for(int32_t bi = 0; bi < 8 && base+bi < blocks; ++bi) {
      unsigned char mask = 128 >> bi;
      if(bitfield[i] & mask) {
	missingIndexes.push_back(base+bi);
      }
    }
  }
  return missingIndexes;
}

BlockIndexes BitfieldMan::getAllMissingIndexes() const {
  array_fun<unsigned char> bf = array_negate(bitfield);
  if(filterEnabled) {
    bf = array_and(bf, filterBitfield);
  }
  return getAllMissingIndexes(bf, bitfieldLength);
}

BlockIndexes BitfieldMan::getAllMissingIndexes(const unsigned char* peerBitfield, int32_t peerBitfieldLength) const {
  if(bitfieldLength != peerBitfieldLength) {
    return BlockIndexes();
  }
  array_fun<unsigned char> bf = array_and(array_negate(bitfield),
					  peerBitfield);
  if(filterEnabled) {
    bf = array_and(bf, filterBitfield);
  }
  return getAllMissingIndexes(bf, bitfieldLength);
}

int32_t BitfieldMan::countMissingBlock() const {
  return cachedNumMissingBlock;
}

int32_t BitfieldMan::countMissingBlockNow() const {
  if(filterEnabled) {
    unsigned char* temp = new unsigned char[bitfieldLength];
    for(int32_t i = 0; i < bitfieldLength; ++i) {
      temp[i] = bitfield[i]&filterBitfield[i];
    }
    int32_t count =  countSetBit(filterBitfield, bitfieldLength)-
      countSetBit(temp, bitfieldLength);
    delete [] temp;
    return count;
  } else {
    return blocks-countSetBit(bitfield, bitfieldLength);
  }
}

int32_t BitfieldMan::countFilteredBlock() const {
  return cachedNumFilteredBlock;
}

int32_t BitfieldMan::countBlock() const {
  return blocks;
}

int32_t BitfieldMan::countFilteredBlockNow() const {
  if(filterEnabled) {
    return countSetBit(filterBitfield, bitfieldLength);
  } else {
    return 0;
  }
}

bool BitfieldMan::setBitInternal(unsigned char* bitfield, int32_t index, bool on) {
  if(blocks <= index) { return false; }
  unsigned char mask = 128 >> index%8;
  if(on) {
    bitfield[index/8] |= mask;
  } else {
    bitfield[index/8] &= ~mask;
  }
  return true;
}

bool BitfieldMan::setUseBit(int32_t index) {
  return setBitInternal(useBitfield, index, true);
}

bool BitfieldMan::unsetUseBit(int32_t index) {
  return setBitInternal(useBitfield, index, false);
}

bool BitfieldMan::setBit(int32_t index) {
  bool b = setBitInternal(bitfield, index, true);
  updateCache();
  return b;
}

bool BitfieldMan::unsetBit(int32_t index) {
  bool b = setBitInternal(bitfield, index, false);
  updateCache();
  return b;
}

bool BitfieldMan::isFilteredAllBitSet() const {
  if(filterEnabled) {
    for(int32_t i = 0; i < bitfieldLength; ++i) {
      if((bitfield[i]&filterBitfield[i]) != filterBitfield[i]) {
	return false;
      }
    }
    return true;
  } else {
    return isAllBitSet();
  }
}

bool BitfieldMan::isAllBitSet() const {
  if(bitfieldLength == 0) {
    return true;
  }
  for(int32_t i = 0; i < bitfieldLength-1; ++i) {
    if(bitfield[i] != 0xff) {
      return false;
    }
  }
  unsigned char b = ~((128 >> (blocks-1)%8)-1);
  if(bitfield[bitfieldLength-1] != b) {
    return false;
  }
  return true;
}

bool BitfieldMan::isBitSetInternal(const unsigned char* bitfield, int32_t index) const {
  if(index < 0 || blocks <= index) { return false; }
  unsigned char mask = 128 >> index%8;
  return (bitfield[index/8] & mask) != 0;
}

bool BitfieldMan::isBitSet(int32_t index) const {
  return isBitSetInternal(bitfield, index);
}

bool BitfieldMan::isUseBitSet(int32_t index) const {
  return isBitSetInternal(useBitfield, index);
}

void BitfieldMan::setBitfield(const unsigned char* bitfield, int32_t bitfieldLength) {
  if(this->bitfieldLength != bitfieldLength) {
    return;
  }
  memcpy(this->bitfield, bitfield, this->bitfieldLength);
  memset(this->useBitfield, 0, this->bitfieldLength);
  updateCache();
}

void BitfieldMan::clearAllBit() {
  memset(this->bitfield, 0, this->bitfieldLength);
  updateCache();
}

void BitfieldMan::setAllBit() {
  for(int32_t i = 0; i < blocks; ++i) {
    setBitInternal(bitfield, i, true);
  }
  updateCache();
}

void BitfieldMan::clearAllUseBit() {
  memset(this->useBitfield, 0, this->bitfieldLength);
  updateCache();
}

void BitfieldMan::setAllUseBit() {
  for(int32_t i = 0; i < blocks; ++i) {
    setBitInternal(useBitfield, i, true);
  }
}

bool BitfieldMan::setFilterBit(int32_t index) {
  return setBitInternal(filterBitfield, index, true);
}

void BitfieldMan::addFilter(int64_t offset, int64_t length) {
  if(!filterBitfield) {
    filterBitfield = new unsigned char[bitfieldLength];
    memset(filterBitfield, 0, bitfieldLength);
  }
  int32_t startBlock = offset/blockLength;
  int32_t endBlock = (offset+length-1)/blockLength;
  for(int i = startBlock; i <= endBlock && i < blocks; i++) {
    setFilterBit(i);
  }
  updateCache();
}

void BitfieldMan::enableFilter() {
  filterEnabled = true;
  updateCache();
}

void BitfieldMan::disableFilter() {
  filterEnabled = false;
  updateCache();
}

void BitfieldMan::clearFilter() {
  if(filterBitfield) {
    delete [] filterBitfield;
    filterBitfield = 0;
  }
  filterEnabled = false;
  updateCache();
}

bool BitfieldMan::isFilterEnabled() const {
  return filterEnabled;
}

int64_t BitfieldMan::getFilteredTotalLength() const {
  return cachedFilteredTotalLength;
}

int64_t BitfieldMan::getFilteredTotalLengthNow() const {
  if(!filterBitfield) {
    return 0;
  }
  int32_t filteredBlocks = countSetBit(filterBitfield, bitfieldLength);
  if(filteredBlocks == 0) {
    return 0;
  }
  if(isBitSetInternal(filterBitfield, blocks-1)) {
    return ((int64_t)filteredBlocks-1)*blockLength+getLastBlockLength();
  } else {
    return ((int64_t)filteredBlocks)*blockLength;
  }
}

int64_t BitfieldMan::getCompletedLength(bool useFilter) const {
  unsigned char* temp = new unsigned char[bitfieldLength];
  if(useFilter) {
    for(int32_t i = 0; i < bitfieldLength; ++i) {
      temp[i] = bitfield[i];
      if(filterEnabled) {
	temp[i] &= filterBitfield[i];
      }
    }
  } else {
    memcpy(temp, bitfield, bitfieldLength);
  }
  int32_t completedBlocks = countSetBit(temp, bitfieldLength);
  int64_t completedLength = 0;
  if(completedBlocks == 0) {
    completedLength = 0;
  } else {
    if(isBitSetInternal(temp, blocks-1)) {
      completedLength = ((int64_t)completedBlocks-1)*blockLength+getLastBlockLength();
    } else {
      completedLength = ((int64_t)completedBlocks)*blockLength;
    }
  }
  delete [] temp;
  return completedLength;
}

int64_t BitfieldMan::getCompletedLength() const {
  return cachedCompletedLength;
}

int64_t BitfieldMan::getCompletedLengthNow() const {
  return getCompletedLength(false);
}

int64_t BitfieldMan::getFilteredCompletedLength() const {
  return cachedFilteredComletedLength;
}

int64_t BitfieldMan::getFilteredCompletedLengthNow() const {
  return getCompletedLength(true);
}

void BitfieldMan::updateCache()
{
  cachedNumMissingBlock = countMissingBlockNow();
  cachedNumFilteredBlock = countFilteredBlockNow();
  cachedFilteredTotalLength = getFilteredTotalLengthNow();
  cachedCompletedLength = getCompletedLengthNow();
  cachedFilteredComletedLength = getFilteredCompletedLengthNow();
}

bool BitfieldMan::isBitRangeSet(int32_t startIndex, int32_t endIndex) const
{
  for(int32_t i =  startIndex; i <= endIndex; ++i) {
    if(!isBitSet(i)) {
      return false;
    }
  }
  return true;
}

void BitfieldMan::unsetBitRange(int32_t startIndex, int32_t endIndex)
{
  for(int32_t i = startIndex; i <= endIndex; ++i) {
    unsetBit(i);
  }
  updateCache();
}

void BitfieldMan::setBitRange(int32_t startIndex, int32_t endIndex)
{
  for(int32_t i = startIndex; i <= endIndex; ++i) {
    setBit(i);
  }
  updateCache();
}

bool BitfieldMan::isBitSetOffsetRange(int64_t offset, int64_t length) const
{
  if(length <= 0) {
    return false;
  }
  if(totalLength <= offset) {
    return false;
  }
  if(totalLength < offset+length) {
    length = totalLength-offset;
  }
  int32_t startBlock = offset/blockLength;
  int32_t endBlock = (offset+length-1)/blockLength;
  for(int32_t i = startBlock; i <= endBlock; i++) {
    if(!isBitSet(i)) {
      return false;
    }
  }
  return true;
}

int64_t BitfieldMan::getMissingUnusedLength(int32_t startingIndex) const
{
  if(startingIndex < 0 || blocks <= startingIndex) {
    return 0;
  }
  int64_t length = 0;
  for(int32_t i = startingIndex; i < blocks; ++i) {
    if(isBitSet(i) || isUseBitSet(i)) {
      break;
    }
    length += getBlockLength(i);
  }
  return length;
}
