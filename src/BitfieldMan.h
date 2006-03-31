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
#ifndef _D_BITFIELD_MAN_H_
#define _D_BITFIELD_MAN_H_

#include "common.h"
#include <deque>

typedef deque<int> BlockIndexes;

class BitfieldMan {
private:
  int blockLength;
  long long int totalLength;
  unsigned char* bitfield;
  unsigned char* useBitfield;
  int bitfieldLength;
  int blocks;

  int countSetBit(const unsigned char* bitfield, int len) const;
  int getMissingIndexRandomly(const unsigned char* bitfield, int len, int randMax) const;
  bool isBitSetInternal(const unsigned char* bitfield, int index) const;
public:
  BitfieldMan(int blockLength, long long int totalLength);
  BitfieldMan(const BitfieldMan& bitfieldMan);
  ~BitfieldMan();

  BitfieldMan& operator=(const BitfieldMan& bitfieldMan);

  int getBlockLength() const { return blockLength; }
  int getLastBlockLength() const {
    return totalLength-blockLength*(blocks-1);
  }
  int getBlockLength(int index) const {
    if(index == blocks-1) {
      return getLastBlockLength();
    } else if(0 <= index && index < blocks-1) {
      return getBlockLength();
    } else {
      return 0;
    }
  }
  long long int getTotalLength() const { return totalLength; }

  int getMissingIndex(const unsigned char* bitfield, int len) const;
  int getFirstMissingUnusedIndex(const unsigned char* bitfield, int len) const;
  int getFirstMissingUnusedIndex() const;
  int getMissingUnusedIndex(const unsigned char* bitfield, int len) const;
  BlockIndexes getAllMissingIndexes() const;
  int countMissingBlock() const;
  bool setUseBit(int index);
  bool unsetUseBit(int index);

  bool setBit(int index);
  bool unsetBit(int index);

  bool isBitSet(int index) const;
  bool isUseBitSet(int index) const;

  bool isAllBitSet() const;

  const unsigned char* getBitfield() const { return bitfield; }
  int getBitfieldLength() const { return bitfieldLength; }

  int countBlock() const { return blocks; }

  void setBitfield(const unsigned char* bitfield, int bitfieldLength);

  void clearAllBit();
  void setAllBit();

  void addFilter(long long int offset, long long int length);
  void clearFilter();
};

#endif // _D_BITFIELD_MAN_H_
