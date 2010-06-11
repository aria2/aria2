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
#ifndef _D_BITFIELD_MAN_H_
#define _D_BITFIELD_MAN_H_

#include "common.h"

#include <vector>

#include "SharedHandle.h"

namespace aria2 {

class BitfieldMan {
private:
  size_t _blockLength;
  uint64_t _totalLength;
  size_t _bitfieldLength;
  size_t _blocks;
  bool _filterEnabled;
  unsigned char* _bitfield;
  unsigned char* _useBitfield;
  unsigned char* _filterBitfield;

  // for caching
  size_t _cachedNumMissingBlock;
  size_t _cachedNumFilteredBlock;
  uint64_t _cachedCompletedLength;
  uint64_t _cachedFilteredCompletedLength;
  uint64_t _cachedFilteredTotalLength;

  bool setBitInternal(unsigned char* bitfield, size_t index, bool on);
  bool setFilterBit(size_t index);

  size_t getStartIndex(size_t index) const;
  size_t getEndIndex(size_t index) const;

  uint64_t getCompletedLength(bool useFilter) const;

  // If _filterBitfield is 0, allocate _bitfieldLength bytes to it and
  // set 0 to all bytes.
  void ensureFilterBitfield();
public:
  // [startIndex, endIndex)
  class Range {
  public:
    size_t startIndex;
    size_t endIndex;
    Range(size_t startIndex = 0, size_t endIndex = 0):startIndex(startIndex),
                                                      endIndex(endIndex) {}
  
    size_t getSize() const {
      return endIndex-startIndex;
    }

    size_t getMidIndex() const {
      return (endIndex-startIndex)/2+startIndex;
    }

    bool operator<(const Range& range) const {
      return getSize() < range.getSize();
    }
  };
public:
  BitfieldMan(size_t blockLength, uint64_t totalLength);
  BitfieldMan(const BitfieldMan& bitfieldMan);
  ~BitfieldMan();

  BitfieldMan& operator=(const BitfieldMan& bitfieldMan);

  size_t getBlockLength() const
  {
    return _blockLength;
  }

  size_t getLastBlockLength() const
  {
    return _totalLength-_blockLength*(_blocks-1);
  }

  size_t getBlockLength(size_t index) const;

  uint64_t getTotalLength() const { return _totalLength; }

  // Returns true iff there is a bit index which is set in _bitfield,
  // but not set in this object.
  //
  // affected by filter
  bool hasMissingPiece(const unsigned char* bitfield, size_t len) const;

  // affected by filter
  bool getFirstMissingUnusedIndex(size_t& index) const;

  // Appends at most n missing unused index to out. This function
  // doesn't delete existing elements in out.  Returns the number of
  // appended elements.
  //
  // affected by filter
  size_t getFirstNMissingUnusedIndex(std::vector<size_t>& out, size_t n) const;

  // Stores first missing bit index to index. Returns true if such bit
  // index is found. Otherwise returns false.
  //
  // affected by filter
  bool getFirstMissingIndex(size_t& index) const;

  // Stores missing bit index to index. index is selected so that it
  // divides longest missing bit subarray into 2 equally sized
  // subarray. Set bits in ignoreBitfield are excluded. Returns true
  // if such bit index is found. Otherwise returns false.
  //
  // affected by filter
  bool getSparseMissingUnusedIndex
  (size_t& index,
   const unsigned char* ignoreBitfield,
   size_t ignoreBitfieldLength) const;

  // affected by filter
  bool getAllMissingIndexes(unsigned char* misbitfield, size_t mislen) const;

  // affected by filter
  bool getAllMissingIndexes(unsigned char* misbitfield, size_t mislen,
                            const unsigned char* bitfield, size_t len) const;
  // affected by filter
  bool getAllMissingUnusedIndexes(unsigned char* misbitfield, size_t mislen,
                                  const unsigned char* bitfield,
                                  size_t len) const;
  // affected by filter
  size_t countMissingBlock() const;

  // affected by filter
  size_t countMissingBlockNow() const;

  bool setUseBit(size_t index);
  bool unsetUseBit(size_t index);

  bool setBit(size_t index);
  bool unsetBit(size_t index);

  bool isBitSet(size_t index) const;
  bool isUseBitSet(size_t index) const;

  // affected by filter
  bool isFilteredAllBitSet() const;

  bool isAllBitSet() const;

  bool isAllFilterBitSet() const;

  const unsigned char* getBitfield() const
  {
    return _bitfield;
  }

  size_t getBitfieldLength() const
  {
    return _bitfieldLength;
  }

  // affected by filter
  size_t countFilteredBlock() const
  {
    return _cachedNumFilteredBlock;
  }

  size_t countBlock() const
  {
    return _blocks;
  }

  // affected by filter
  size_t countFilteredBlockNow() const;

  size_t getMaxIndex() const
  {
    return _blocks-1;
  }

  void setBitfield(const unsigned char* bitfield, size_t bitfieldLength);

  void clearAllBit();
  void setAllBit();

  void clearAllUseBit();
  void setAllUseBit();

  void addFilter(uint64_t offset, uint64_t length);
  void removeFilter(uint64_t offset, uint64_t length);
  // Add filter not in the range of [offset, offset+length) bytes
  void addNotFilter(uint64_t offset, uint64_t length);

  // Clears filter and disables filter
  void clearFilter();
  
  void enableFilter();
  void disableFilter();
  bool isFilterEnabled() const
  {
    return _filterEnabled;
  }

  // affected by filter
  uint64_t getFilteredTotalLength() const
  {
    return _cachedFilteredTotalLength;
  }

  // affected by filter
  uint64_t getFilteredTotalLengthNow() const;

  uint64_t getCompletedLength() const
  {
    return _cachedCompletedLength;
  }

  uint64_t getCompletedLengthNow() const;

  // affected by filter
  uint64_t getFilteredCompletedLength() const
  {
    return _cachedFilteredCompletedLength;
  }

  // affected by filter
  uint64_t getFilteredCompletedLengthNow() const;

  void updateCache();

  bool isBitRangeSet(size_t startIndex, size_t endIndex) const;

  void unsetBitRange(size_t startIndex, size_t endIndex);

  void setBitRange(size_t startIndex, size_t endIndex);

  bool isBitSetOffsetRange(uint64_t offset, uint64_t length) const;

  uint64_t getMissingUnusedLength(size_t startingIndex) const;

  const unsigned char* getFilterBitfield() const
  {
    return _filterBitfield;
  }
};

} // namespace aria2

#endif // _D_BITFIELD_MAN_H_
