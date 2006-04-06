/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "BitfieldMan.h"
#include <string.h>

BitfieldMan::BitfieldMan(int blockLength, long long int totalLength)
  :blockLength(blockLength), totalLength(totalLength), filterBitfield(0),
   filterEnabled(false) {
  if(blockLength > 0 && totalLength > 0) {
    blocks = totalLength/blockLength+(totalLength%blockLength ? 1 : 0);
    bitfieldLength = blocks/8+(blocks%8 ? 1 : 0);
    bitfield = new unsigned char[bitfieldLength];
    useBitfield = new unsigned char[bitfieldLength];
    memset(bitfield, 0, bitfieldLength);
    memset(useBitfield, 0, bitfieldLength);
  }
}

BitfieldMan::BitfieldMan(const BitfieldMan& bitfieldMan) {
  blockLength = bitfieldMan.blockLength;
  totalLength = bitfieldMan.totalLength;
  blocks = bitfieldMan.blocks;
  bitfieldLength = bitfieldMan.bitfieldLength;
  bitfield = new unsigned char[bitfieldLength];
  useBitfield = new unsigned char[bitfieldLength];
  memcpy(bitfield, bitfieldMan.bitfield, bitfieldLength);
  memcpy(useBitfield, bitfieldMan.useBitfield, bitfieldLength);
  filterEnabled = bitfieldMan.filterEnabled;
  if(bitfieldMan.filterBitfield) {
    filterBitfield = new unsigned char[bitfieldLength];
    memcpy(filterBitfield, bitfieldMan.filterBitfield, bitfieldLength);
  } else {
    filterBitfield = 0;
  }
}

BitfieldMan::~BitfieldMan() {
  delete [] bitfield;
  delete [] useBitfield;
  if(filterBitfield) {
    delete [] filterBitfield;
  }
}

BitfieldMan& BitfieldMan::operator=(const BitfieldMan& bitfieldMan) {
  if(this != &bitfieldMan) {
    blockLength = bitfieldMan.blockLength;
    totalLength = bitfieldMan.totalLength;
    if(bitfieldLength != bitfieldMan.bitfieldLength) {
      delete [] bitfield;
      delete [] useBitfield;
      bitfield = new unsigned char[bitfieldMan.bitfieldLength];
      useBitfield = new unsigned char[bitfieldMan.bitfieldLength];
    }
    blocks = bitfieldMan.blocks;
    bitfieldLength = bitfieldMan.bitfieldLength;
    memcpy(bitfield, bitfieldMan.bitfield, bitfieldLength);
    memcpy(useBitfield, bitfieldMan.useBitfield, bitfieldLength);
    filterEnabled = bitfieldMan.filterEnabled;
    if(bitfieldLength != bitfieldMan.bitfieldLength) {
      delete [] filterBitfield;
      filterBitfield = 0;
    }
    if(bitfieldMan.filterBitfield) {
      if(!filterBitfield) {
	filterBitfield = new unsigned char[bitfieldLength];
      }
      memcpy(filterBitfield, bitfieldMan.filterBitfield, bitfieldLength);
    }
  }
  return *this;
}

int BitfieldMan::countSetBit(const unsigned char* bitfield, int len) const {
  int count = 0;
  for(int i = 0; i < len; i++) {
    unsigned char bit = bitfield[i];
    for(int bs = 7; bs >= 0 && i*8+7-bs < blocks; bs--) {
      unsigned char mask = 1 << bs;
      if(bit & mask) {
	count++;
      }
    }
  }
  return count;
}

int BitfieldMan::getMissingIndexRandomly(const unsigned char* bitfield, int len, int randMax) const {
  int index = -1;
  int nth = 1+(int)(((double)randMax)*random()/(RAND_MAX+1.0));
  for(int i = 0; i < len && index == -1; i++) {
    unsigned char bit = bitfield[i];
    for(int bs = 7; bs >= 0 && i*8+7-bs < blocks; bs--) {
      unsigned char mask = 1 << bs;
      if(bit & mask) {
	nth--;
	if(nth == 0) {
	  index = i*8+7-bs;
	  break;
	}
      }
    }
  }
  return index;
}

int BitfieldMan::getMissingIndex(const unsigned char* peerBitfield, int length) const {
  if(bitfieldLength != length) {
    return -1;
  }
  unsigned char* tempBitfield = new unsigned char[bitfieldLength];
  for(int i = 0; i < bitfieldLength; i++) {
    tempBitfield[i] = peerBitfield[i] & ~bitfield[i];
    if(filterEnabled) {
      tempBitfield[i] &= filterBitfield[i];
    }
  }
  int max = countSetBit(tempBitfield, bitfieldLength);
  int index = getMissingIndexRandomly(tempBitfield, bitfieldLength, max);
  delete [] tempBitfield;
  return index;
}

int BitfieldMan::getMissingUnusedIndex(const unsigned char* peerBitfield, int length) const {
  if(bitfieldLength != length) {
    return -1;
  }
  unsigned char* tempBitfield = new unsigned char[bitfieldLength];
  for(int i = 0; i < bitfieldLength; i++) {
    tempBitfield[i] = peerBitfield[i] & ~bitfield[i] & ~useBitfield[i];
    if(filterEnabled) {
      tempBitfield[i] &= filterBitfield[i];
    }
  }
  int max = countSetBit(tempBitfield, bitfieldLength);
  int index = getMissingIndexRandomly(tempBitfield, bitfieldLength, max);
  delete [] tempBitfield;
  return index;
}

int BitfieldMan::getFirstMissingUnusedIndex(const unsigned char* peerBitfield, int length) const {
  if(bitfieldLength != length) {
    return -1;
  }
  for(int i = 0; i < bitfieldLength; i++) {
    unsigned char bit = peerBitfield[i] & ~bitfield[i] & ~useBitfield[i];
    if(filterEnabled) {
      bit &= filterBitfield[i];
    }
    for(int bs = 7; bs >= 0 && i*8+7-bs < blocks; bs--) {
      unsigned char mask = 1 << bs;
      if(bit & mask) {
	return i*8+7-bs;
      }
    }
  }
  return -1;
}

int BitfieldMan::getFirstMissingUnusedIndex() const {
  for(int i = 0; i < bitfieldLength; i++) {
    unsigned char bit = ~bitfield[i] & ~useBitfield[i];
    if(filterEnabled) {
      bit &= filterBitfield[i];
    }
    for(int bs = 7; bs >= 0 && i*8+7-bs < blocks; bs--) {
      unsigned char mask = 1 << bs;
      if(bit & mask) {
	return i*8+7-bs;
      }
    }
  }
  return -1;
}

BlockIndexes BitfieldMan::getAllMissingIndexes() const {
  BlockIndexes missingIndexes;
  for(int i = 0; i < bitfieldLength; i++) {
    unsigned char bit = ~bitfield[i];
    if(filterEnabled) {
      bit &= filterBitfield[i];
    }
    for(int bs = 7; bs >= 0 && i*8+7-bs < blocks; bs--) {
      unsigned char mask = 1 << bs;
      if(bit & mask) {
	missingIndexes.push_back(i*8+7-bs);
      }
    }
  }
  return missingIndexes;
}

int BitfieldMan::countMissingBlock() const {
  if(filterEnabled) {
    unsigned char* temp = new unsigned char[bitfieldLength];
    for(int i = 0; i < bitfieldLength; i++) {
      temp[i] = bitfield[i]&filterBitfield[i];
    }
    int count =  countSetBit(filterBitfield, bitfieldLength)-
      countSetBit(temp, bitfieldLength);
    delete [] temp;
    return count;
  } else {
    return blocks-countSetBit(bitfield, bitfieldLength);
  }
}

int BitfieldMan::countBlock() const {
  if(filterEnabled) {
    return countSetBit(filterBitfield, bitfieldLength);
  } else {
    return blocks;
  }
}

bool BitfieldMan::setBitInternal(unsigned char* bitfield, int index, bool on) {
  if(blocks <= index) { return false; }
  unsigned char mask = 128 >> index%8;
  if(on) {
    bitfield[index/8] |= mask;
  } else {
    bitfield[index/8] &= ~mask;
  }
  return true;
}

bool BitfieldMan::setUseBit(int index) {
  return setBitInternal(useBitfield, index, true);
}

bool BitfieldMan::unsetUseBit(int index) {
  return setBitInternal(useBitfield, index, false);
}

bool BitfieldMan::setBit(int index) {
  return setBitInternal(bitfield, index, true);
}

bool BitfieldMan::unsetBit(int index) {
  return setBitInternal(bitfield, index, false);
}

bool BitfieldMan::isAllBitSet() const {
  if(filterEnabled) {
    for(int i = 0; i < bitfieldLength-1; i++) {
      if((bitfield[i]&filterBitfield[i]) != filterBitfield[i]) {
	return false;
      }
    }
    return true;
  } else {
    for(int i = 0; i < bitfieldLength-1; i++) {
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
}

bool BitfieldMan::isBitSetInternal(const unsigned char* bitfield, int index) const {
  if(blocks <= index) { return false; }
  unsigned char mask = 128 >> index%8;
  return (bitfield[index/8] & mask) != 0;
}

bool BitfieldMan::isBitSet(int index) const {
  return isBitSetInternal(bitfield, index);
}

bool BitfieldMan::isUseBitSet(int index) const {
  return isBitSetInternal(useBitfield, index);
}

void BitfieldMan::setBitfield(const unsigned char* bitfield, int bitfieldLength) {
  if(this->bitfieldLength != bitfieldLength) {
    return;
  }
  memcpy(this->bitfield, bitfield, this->bitfieldLength);
  memset(this->useBitfield, 0, this->bitfieldLength);
}

void BitfieldMan::clearAllBit() {
  memset(this->bitfield, 0, this->bitfieldLength);
  memset(this->useBitfield, 0, this->bitfieldLength);
}

void BitfieldMan::setAllBit() {
  for(int i = 0; i < blocks; i++) {
    setBit(i);
  }
}

bool BitfieldMan::setFilterBit(int index) {
  return setBitInternal(filterBitfield, index, true);
}

void BitfieldMan::addFilter(long long int offset, long long int length) {
  if(!filterBitfield) {
    filterBitfield = new unsigned char[bitfieldLength];
    memset(filterBitfield, 0, bitfieldLength);
  }
  int startBlock = offset/blockLength;
  int endBlock = (offset+length-1)/blockLength;
  for(int i = startBlock; i <= endBlock && i < blocks; i++) {
    setFilterBit(i);
  }
}

void BitfieldMan::enableFilter() {
  filterEnabled = true;
}

void BitfieldMan::disableFilter() {
  filterEnabled = false;
}

void BitfieldMan::clearFilter() {
  if(filterBitfield) {
    delete [] filterBitfield;
    filterBitfield = 0;
  }
  filterEnabled = false;
}

bool BitfieldMan::isFilterEnabled() const {
  return filterEnabled;
}

long long int BitfieldMan::getFilteredTotalLength() const {
  if(!filterBitfield) {
    return 0;
  }
  int filteredBlocks = countSetBit(filterBitfield, bitfieldLength);
  if(filteredBlocks == 0) {
    return 0;
  }
  if(isBitSetInternal(filterBitfield, blocks-1)) {
    return ((long long int)filteredBlocks-1)*blockLength+getLastBlockLength();
  } else {
    return ((long long int)filteredBlocks)*blockLength;
  }
}

long long int BitfieldMan::getCompletedLength() const {
  unsigned char* temp = new unsigned char[bitfieldLength];
  for(int i = 0; i < bitfieldLength; i++) {
    temp[i] = bitfield[i];
    if(filterEnabled) {
      temp[i] &= filterBitfield[i];
    }
  }
  int completedBlocks = countSetBit(temp, bitfieldLength);
  long long int completedLength = 0;
  if(completedBlocks == 0) {
    completedLength = 0;
  } else {
    if(isBitSetInternal(temp, blocks-1)) {
      completedLength = ((long long int)completedBlocks-1)*blockLength+getLastBlockLength();
    } else {
      completedLength = ((long long int)completedBlocks)*blockLength;
    }
  }
  delete [] temp;
  return completedLength;
}
