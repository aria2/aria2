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
#ifndef _D_BITFIELD_MAN_H_
#define _D_BITFIELD_MAN_H_

#include "common.h"
#include "Randomizer.h"
#include <deque>

typedef deque<int> BlockIndexes;

class BitfieldMan {
private:
  uint32_t blockLength;
  uint64_t totalLength;
  unsigned char* bitfield;
  unsigned char* useBitfield;
  unsigned char* filterBitfield;
  uint32_t bitfieldLength;
  uint32_t blocks;
  bool filterEnabled;
  RandomizerHandle randomizer;

  // for caching
  uint32_t cachedNumMissingBlock;
  uint32_t cachedNumFilteredBlock;
  uint64_t cachedCompletedLength;
  uint64_t cachedFilteredComletedLength;
  uint64_t cachedFilteredTotalLength;

  uint32_t countSetBit(const unsigned char* bitfield, uint32_t len) const;
  int32_t getNthBitIndex(const unsigned char bit, uint32_t nth) const;
  int32_t getMissingIndexRandomly(const unsigned char* bitfield, uint32_t len) const;
  bool isBitSetInternal(const unsigned char* bitfield, int32_t index) const;
  bool setBitInternal(unsigned char* bitfield, int32_t index, bool on);
  bool setFilterBit(int32_t index);

  int32_t getStartIndex(int32_t index) const;
  int32_t getEndIndex(int32_t index) const;

  uint64_t getCompletedLength(bool useFilter) const;
public:
  BitfieldMan(uint32_t blockLength, uint64_t totalLength);
  BitfieldMan(const BitfieldMan& bitfieldMan);
  ~BitfieldMan();

  BitfieldMan& operator=(const BitfieldMan& bitfieldMan) {
    if(this != &bitfieldMan) {
      blockLength = bitfieldMan.blockLength;
      totalLength = bitfieldMan.totalLength;
      blocks = bitfieldMan.blocks;
      bitfieldLength = bitfieldMan.bitfieldLength;
      filterEnabled = bitfieldMan.filterEnabled;

      delete [] bitfield;
      bitfield = new unsigned char[bitfieldLength];
      memcpy(bitfield, bitfieldMan.bitfield, bitfieldLength);

      delete [] useBitfield;
      useBitfield = new unsigned char[bitfieldLength];
      memcpy(useBitfield, bitfieldMan.useBitfield, bitfieldLength);

      delete [] filterBitfield;
      if(filterEnabled) {
	filterBitfield = new unsigned char[bitfieldLength];
	memcpy(filterBitfield, bitfieldMan.filterBitfield, bitfieldLength);
      } else {
	filterBitfield = 0;
      }

      updateCache();
    }
    return *this;
  }

  uint32_t getBlockLength() const { return blockLength; }

  uint32_t getLastBlockLength() const {
    return totalLength-blockLength*(blocks-1);
  }

  uint32_t getBlockLength(int32_t index) const {
    if(index == (int32_t)(blocks-1)) {
      return getLastBlockLength();
    } else if(0 <= index && index < (int32_t)(blocks-1)) {
      return getBlockLength();
    } else {
      return 0;
    }
  }

  uint64_t getTotalLength() const { return totalLength; }

  /**
   * affected by filter
   */
  bool hasMissingPiece(const unsigned char* bitfield, uint32_t len) const;
  /**
   * affected by filter
   */
  int32_t getMissingIndex(const unsigned char* bitfield, uint32_t len) const;
  /**
   * affected by filter
   */
  int32_t getMissingIndex() const;
  /**
   * affected by filter
   */
  int32_t getFirstMissingUnusedIndex(const unsigned char* bitfield, uint32_t len) const;
  /**
   * affected by filter
   */
  int32_t getFirstMissingUnusedIndex() const;
  /**
   * affected by filter
   */
  int32_t getMissingUnusedIndex(const unsigned char* bitfield, uint32_t len) const;
  /**
   * affected by filter
   */
  int32_t getMissingUnusedIndex() const;
  /**
   * affected by filter
   */
  int32_t getSparseMissingUnusedIndex() const;
  /**
   * affected by filter
   */
  BlockIndexes getAllMissingIndexes() const;
  /**
   * affected by filter
   */
  BlockIndexes getAllMissingIndexes(const unsigned char* bitfield, uint32_t len) const;
  /**
   * affected by filter
   */
  uint32_t countMissingBlock() const;
  /**
   * affected by filter
   */
  uint32_t countMissingBlockNow() const;

  bool setUseBit(int32_t index);
  bool unsetUseBit(int32_t index);

  bool setBit(int32_t index);
  bool unsetBit(int32_t index);

  bool isBitSet(int32_t index) const;
  bool isUseBitSet(int32_t index) const;

  /**
   * affected by filter
   */
  bool isAllBitSet() const;

  const unsigned char* getBitfield() const { return bitfield; }
  uint32_t getBitfieldLength() const { return bitfieldLength; }

  /**
   * affected by filter
   */
  uint32_t countBlock() const;
  /**
   * affected by filter
   */
  uint32_t countFilteredBlockNow() const;

  int32_t getMaxIndex() const { return blocks-1; }

  void setBitfield(const unsigned char* bitfield, uint32_t bitfieldLength);

  void clearAllBit();
  void setAllBit();

  void clearAllUseBit();
  void setAllUseBit();

  void addFilter(int64_t offset, uint64_t length);
  /**
   * Clears filter and disables filter
   */
  void clearFilter();
  
  void enableFilter();
  void disableFilter();
  bool isFilterEnabled() const;
  /**
   * affected by filter
   */
  uint64_t getFilteredTotalLength() const;
  /**
   * affected by filter
   */
  uint64_t getFilteredTotalLengthNow() const;

  uint64_t getCompletedLength() const;

  uint64_t getCompletedLengthNow() const;

  /**
   * affected by filter
   */
  uint64_t getFilteredCompletedLength() const;
  /**
   * affected by filter
   */
  uint64_t getFilteredCompletedLengthNow() const;

  void setRandomizer(const RandomizerHandle& randomizer) {
    this->randomizer = randomizer;
  }

  RandomizerHandle getRandomizer() const {
    return randomizer;
  }

  void updateCache();
};

#endif // _D_BITFIELD_MAN_H_
