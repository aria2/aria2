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
#include "SharedHandle.h"
#include <deque>

namespace aria2 {

class Randomizer;

class BitfieldMan {
private:
  int32_t blockLength;
  int64_t totalLength;
  unsigned char* bitfield;
  unsigned char* useBitfield;
  unsigned char* filterBitfield;
  int32_t bitfieldLength;
  int32_t blocks;
  bool filterEnabled;
  SharedHandle<Randomizer> randomizer;

  // for caching
  int32_t cachedNumMissingBlock;
  int32_t cachedNumFilteredBlock;
  int64_t cachedCompletedLength;
  int64_t cachedFilteredComletedLength;
  int64_t cachedFilteredTotalLength;

  int32_t countSetBit(const unsigned char* bitfield, int32_t len) const;
  int32_t getNthBitIndex(const unsigned char bit, int32_t nth) const;
  int32_t getMissingIndexRandomly(const unsigned char* bitfield, int32_t len) const;

  template<typename Array>
  int32_t
  getMissingIndexRandomly(const Array& bitfield,
			  int32_t bitfieldLength) const;
  template<typename Array>
  int32_t getFirstMissingIndex(const Array& bitfield, int32_t bitfieldLength) const;

  template<typename Array>
  std::deque<int32_t> getAllMissingIndexes(const Array& bitfield, int32_t bitfieldLength) const;

  bool isBitSetInternal(const unsigned char* bitfield, int32_t index) const;
  bool setBitInternal(unsigned char* bitfield, int32_t index, bool on);
  bool setFilterBit(int32_t index);

  int32_t getStartIndex(int32_t index) const;
  int32_t getEndIndex(int32_t index) const;

  int64_t getCompletedLength(bool useFilter) const;
public:
  BitfieldMan(int32_t blockLength, int64_t totalLength);
  BitfieldMan(const BitfieldMan& bitfieldMan);
  ~BitfieldMan();

  BitfieldMan& operator=(const BitfieldMan& bitfieldMan);

  int32_t getBlockLength() const;

  int32_t getLastBlockLength() const;

  int32_t getBlockLength(int32_t index) const;

  int64_t getTotalLength() const { return totalLength; }

  /**
   * affected by filter
   */
  bool hasMissingPiece(const unsigned char* bitfield, int32_t len) const;
  /**
   * affected by filter
   */
  int32_t getMissingIndex(const unsigned char* bitfield, int32_t len) const;
  /**
   * affected by filter
   */
  int32_t getMissingIndex() const;
  /**
   * affected by filter
   */
  int32_t getFirstMissingUnusedIndex(const unsigned char* bitfield, int32_t len) const;
  /**
   * affected by filter
   */
  int32_t getFirstMissingUnusedIndex() const;
  /**
   * affected by filter
   */
  int32_t getFirstMissingIndex() const;
  /**
   * affected by filter
   */
  int32_t getMissingUnusedIndex(const unsigned char* bitfield, int32_t len) const;
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
  std::deque<int32_t> getAllMissingIndexes() const;
  /**
   * affected by filter
   */
  std::deque<int32_t> getAllMissingIndexes(const unsigned char* bitfield, int32_t len) const;
  /**
   * affected by filter
   */
  int32_t countMissingBlock() const;
  /**
   * affected by filter
   */
  int32_t countMissingBlockNow() const;

  bool setUseBit(int32_t index);
  bool unsetUseBit(int32_t index);

  bool setBit(int32_t index);
  bool unsetBit(int32_t index);

  bool isBitSet(int32_t index) const;
  bool isUseBitSet(int32_t index) const;

  /**
   * affected by filter
   */
  bool isFilteredAllBitSet() const;

  bool isAllBitSet() const;

  const unsigned char* getBitfield() const;

  int32_t getBitfieldLength() const;

  /**
   * affected by filter
   */
  int32_t countFilteredBlock() const;

  int32_t countBlock() const;
  /**
   * affected by filter
   */
  int32_t countFilteredBlockNow() const;

  int32_t getMaxIndex() const;

  void setBitfield(const unsigned char* bitfield, int32_t bitfieldLength);

  void clearAllBit();
  void setAllBit();

  void clearAllUseBit();
  void setAllUseBit();

  void addFilter(int64_t offset, int64_t length);
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
  int64_t getFilteredTotalLength() const;
  /**
   * affected by filter
   */
  int64_t getFilteredTotalLengthNow() const;

  int64_t getCompletedLength() const;

  int64_t getCompletedLengthNow() const;

  /**
   * affected by filter
   */
  int64_t getFilteredCompletedLength() const;
  /**
   * affected by filter
   */
  int64_t getFilteredCompletedLengthNow() const;

  void setRandomizer(const SharedHandle<Randomizer>& randomizer);

  SharedHandle<Randomizer> getRandomizer() const;

  void updateCache();

  bool isBitRangeSet(int32_t startIndex, int32_t endIndex) const;

  void unsetBitRange(int32_t startIndex, int32_t endIndex);

  void setBitRange(int32_t startIndex, int32_t endIndex);

  bool isBitSetOffsetRange(int64_t offset, int64_t length) const;

  int64_t getMissingUnusedLength(int32_t startingIndex) const;

};

} // namespace aria2

#endif // _D_BITFIELD_MAN_H_
