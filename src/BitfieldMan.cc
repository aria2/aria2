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
  :blockLength(blockLength), totalLength(totalLength) {
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
}

BitfieldMan::~BitfieldMan() {
  delete [] bitfield;
  delete [] useBitfield;
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
  }
  int max = countSetBit(tempBitfield, bitfieldLength);
  int index = getMissingIndexRandomly(tempBitfield, bitfieldLength, max);
  return index;
}

int BitfieldMan::getMissingUnusedIndex(const unsigned char* peerBitfield, int length) const {
  if(bitfieldLength != length) {
    return -1;
  }
  unsigned char* tempBitfield = new unsigned char[bitfieldLength];
  for(int i = 0; i < bitfieldLength; i++) {
    tempBitfield[i] = peerBitfield[i] & ~bitfield[i] & ~useBitfield[i];
  }
  int max = countSetBit(tempBitfield, bitfieldLength);
  /*
  int max = 0;
  for(int i = 0; i < bitfieldLength; i++) {
    unsigned char bit = tempBitfield[i];
    for(int bs = 7; bs >= 0 && i*8+7-bs < blocks; bs--) {
      unsigned char mask = 1 << bs;
      if(bit & mask) {
	max++;
      }
    }
  }
  */
  int index = getMissingIndexRandomly(tempBitfield, bitfieldLength, max);
  /*
  int index = -1;
  int nth = 1+(int)(((double)max)*random()/(RAND_MAX+1.0));
  for(int i = 0; i < bitfieldLength && index == -1; i++) {
    unsigned char bit = tempBitfield[i];
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
  */
  return index;
}

int BitfieldMan::getFirstMissingUnusedIndex(const unsigned char* peerBitfield, int length) const {
  if(bitfieldLength != length) {
    return -1;
  }
  for(int i = 0; i < bitfieldLength; i++) {
    unsigned char bit = peerBitfield[i] & ~bitfield[i] & ~useBitfield[i];
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
    for(int bs = 7; bs >= 0 && i*8+7-bs < blocks; bs--) {
      unsigned char mask = 1 << bs;
      if(bit & mask) {
	return i*8+7-bs;
      }
    }
  }
  return -1;
}

vector<int> BitfieldMan::getAllMissingIndexes() const {
  vector<int> missingIndexes;
  for(int i = 0; i < bitfieldLength; i++) {
    unsigned char bit = ~bitfield[i];
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
  return blocks-countSetBit(bitfield, bitfieldLength);
}

bool BitfieldMan::setUseBit(int index) {
  if(blocks <= index) { return false; }
  unsigned char mask = 128 >> index%8;
  useBitfield[index/8] |= mask;
  return true;
}

bool BitfieldMan::unsetUseBit(int index) {
  if(blocks <= index) { return false; }
  unsigned char mask = 128 >> index%8;
  useBitfield[index/8] &= ~mask;
  return true;
}

bool BitfieldMan::setBit(int index) {
  if(blocks <= index) { return false; }
  unsigned char mask = 128 >> index%8;
  bitfield[index/8] |= mask;
  return true;
}
bool BitfieldMan::unsetBit(int index) {
  if(blocks <= index) { return false; }
  unsigned char mask = 128 >> index%8;
  bitfield[index/8] &= ~mask;
  return true;
}

bool BitfieldMan::isAllBitSet() const {
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
