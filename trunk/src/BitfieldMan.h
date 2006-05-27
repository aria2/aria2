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
  unsigned char* filterBitfield;
  int bitfieldLength;
  int blocks;
  bool filterEnabled;
  int countSetBit(const unsigned char* bitfield, int len) const;
  int getNthBitIndex(const unsigned char* bitfield, int len, int nth) const;
  int getMissingIndexRandomly(const unsigned char* bitfield, int len, int randMax) const;
  bool isBitSetInternal(const unsigned char* bitfield, int index) const;
  bool setBitInternal(unsigned char* bitfield, int index, bool on);
  bool setFilterBit(int index);
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

  /**
   * affected by filter
   */
  bool hasMissingPiece(const unsigned char* bitfield, int len) const;
  /**
   * affected by filter
   */
  int getMissingIndex(const unsigned char* bitfield, int len) const;
  /**
   * affected by filter
   */
  int getMissingIndex() const;
  /**
   * affected by filter
   */
  int getFirstMissingUnusedIndex(const unsigned char* bitfield, int len) const;
  /**
   * affected by filter
   */
  int getFirstMissingUnusedIndex() const;
  /**
   * affected by filter
   */
  int getMissingUnusedIndex(const unsigned char* bitfield, int len) const;
  /**
   * affected by filter
   */
  BlockIndexes getAllMissingIndexes() const;
  /**
   * affected by filter
   */
  BlockIndexes getAllMissingIndexes(const unsigned char* bitfield, int len) const;
  /**
   * affected by filter
   */
  int countMissingBlock() const;
  bool setUseBit(int index);
  bool unsetUseBit(int index);

  bool setBit(int index);
  bool unsetBit(int index);

  bool isBitSet(int index) const;
  bool isUseBitSet(int index) const;

  /**
   * affected by filter
   */
  bool isAllBitSet() const;

  const unsigned char* getBitfield() const { return bitfield; }
  int getBitfieldLength() const { return bitfieldLength; }

  /**
   * affected by filter
   */
  int countBlock() const;

  void setBitfield(const unsigned char* bitfield, int bitfieldLength);

  void clearAllBit();
  void setAllBit();

  void addFilter(long long int offset, long long int length);
  void clearFilter();
  
  void enableFilter();
  void disableFilter();
  bool isFilterEnabled() const;
  long long int getFilteredTotalLength() const;
  /**
   * affected by filter
   */
  long long int getCompletedLength() const;
};

#endif // _D_BITFIELD_MAN_H_
