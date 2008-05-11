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
  size_t blockLength;
  uint64_t totalLength;
  unsigned char* bitfield;
  unsigned char* useBitfield;
  unsigned char* filterBitfield;
  size_t bitfieldLength;
  size_t blocks;
  bool filterEnabled;
  SharedHandle<Randomizer> randomizer;

  // for caching
  size_t cachedNumMissingBlock;
  size_t cachedNumFilteredBlock;
  uint64_t cachedCompletedLength;
  uint64_t cachedFilteredComletedLength;
  uint64_t cachedFilteredTotalLength;

  size_t countSetBit(const unsigned char* bitfield, size_t len) const;
  size_t getNthBitIndex(const unsigned char bit, size_t nth) const;
  bool getMissingIndexRandomly(size_t& index, const unsigned char* bitfield, size_t len) const;

  template<typename Array>
  bool getMissingIndexRandomly(size_t& index, const Array& bitfield,
			       size_t bitfieldLength) const;
  template<typename Array>
  bool getFirstMissingIndex(size_t& index, const Array& bitfield, size_t bitfieldLength) const;

  template<typename Array>
  std::deque<size_t> getAllMissingIndexes(const Array& bitfield, size_t bitfieldLength) const;

  bool isBitSetInternal(const unsigned char* bitfield, size_t index) const;
  bool setBitInternal(unsigned char* bitfield, size_t index, bool on);
  bool setFilterBit(size_t index);

  size_t getStartIndex(size_t index) const;
  size_t getEndIndex(size_t index) const;

  uint64_t getCompletedLength(bool useFilter) const;
public:
  BitfieldMan(size_t blockLength, uint64_t totalLength);
  BitfieldMan(const BitfieldMan& bitfieldMan);
  ~BitfieldMan();

  BitfieldMan& operator=(const BitfieldMan& bitfieldMan);

  size_t getBlockLength() const;

  size_t getLastBlockLength() const;

  size_t getBlockLength(size_t index) const;

  uint64_t getTotalLength() const { return totalLength; }

  /**
   * affected by filter
   */
  bool hasMissingPiece(const unsigned char* bitfield, size_t len) const;
  /**
   * affected by filter
   */
  bool getMissingIndex(size_t& index, const unsigned char* bitfield, size_t len) const;
  /**
   * affected by filter
   */
  bool getMissingIndex(size_t& index) const;
  /**
   * affected by filter
   */
  bool getFirstMissingUnusedIndex(size_t& index, const unsigned char* bitfield, size_t len) const;
  /**
   * affected by filter
   */
  bool getFirstMissingUnusedIndex(size_t& index) const;
  /**
   * affected by filter
   */
  bool getFirstMissingIndex(size_t& index) const;
  /**
   * affected by filter
   */
  bool getMissingUnusedIndex(size_t& index, const unsigned char* bitfield, size_t len) const;
  /**
   * affected by filter
   */
  bool getMissingUnusedIndex(size_t& index) const;
  /**
   * affected by filter
   */
  bool getSparseMissingUnusedIndex(size_t& index) const;
  /**
   * affected by filter
   */
  std::deque<size_t> getAllMissingIndexes() const;
  /**
   * affected by filter
   */
  std::deque<size_t> getAllMissingIndexes(const unsigned char* bitfield, size_t len) const;
  /**
   * affected by filter
   */
  std::deque<size_t> getAllMissingUnusedIndexes(const unsigned char* bitfield,
						size_t len) const;
  /**
   * affected by filter
   */
  size_t countMissingBlock() const;
  /**
   * affected by filter
   */
  size_t countMissingBlockNow() const;

  bool setUseBit(size_t index);
  bool unsetUseBit(size_t index);

  bool setBit(size_t index);
  bool unsetBit(size_t index);

  bool isBitSet(size_t index) const;
  bool isUseBitSet(size_t index) const;

  /**
   * affected by filter
   */
  bool isFilteredAllBitSet() const;

  bool isAllBitSet() const;

  const unsigned char* getBitfield() const;

  size_t getBitfieldLength() const;

  /**
   * affected by filter
   */
  size_t countFilteredBlock() const;

  size_t countBlock() const;
  /**
   * affected by filter
   */
  size_t countFilteredBlockNow() const;

  size_t getMaxIndex() const;

  void setBitfield(const unsigned char* bitfield, size_t bitfieldLength);

  void clearAllBit();
  void setAllBit();

  void clearAllUseBit();
  void setAllUseBit();

  void addFilter(uint64_t offset, uint64_t length);
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

  void setRandomizer(const SharedHandle<Randomizer>& randomizer);

  SharedHandle<Randomizer> getRandomizer() const;

  void updateCache();

  bool isBitRangeSet(size_t startIndex, size_t endIndex) const;

  void unsetBitRange(size_t startIndex, size_t endIndex);

  void setBitRange(size_t startIndex, size_t endIndex);

  bool isBitSetOffsetRange(uint64_t offset, uint64_t length) const;

  uint64_t getMissingUnusedLength(size_t startingIndex) const;

};

} // namespace aria2

#endif // _D_BITFIELD_MAN_H_
