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
#ifndef D_BITFIELD_MAN_H
#define D_BITFIELD_MAN_H

#include "common.h"

#include <vector>

namespace aria2 {

class BitfieldMan {
private:
  int64_t totalLength_;
  int64_t cachedCompletedLength_;
  int64_t cachedFilteredCompletedLength_;
  int64_t cachedFilteredTotalLength_;

  unsigned char* bitfield_;
  unsigned char* useBitfield_;
  unsigned char* filterBitfield_;

  size_t bitfieldLength_;
  size_t cachedNumMissingBlock_;
  size_t cachedNumFilteredBlock_;
  size_t blocks_;

  int32_t blockLength_;

  bool filterEnabled_;

  bool setBitInternal(unsigned char* bitfield, size_t index, bool on);
  bool setFilterBit(size_t index);

  size_t getStartIndex(size_t index) const;
  size_t getEndIndex(size_t index) const;

  int64_t getCompletedLength(bool useFilter) const;

  // If filterBitfield_ is 0, allocate bitfieldLength_ bytes to it and
  // set 0 to all bytes.
  void ensureFilterBitfield();

public:
  // [startIndex, endIndex)
  struct Range {
    size_t startIndex;
    size_t endIndex;
    Range(size_t startIndex = 0, size_t endIndex = 0);
    size_t getSize() const;
    size_t getMidIndex() const;
    bool operator<(const Range& range) const;
    bool operator==(const Range& range) const;
  };

public:
  BitfieldMan(int32_t blockLength, int64_t totalLength);
  BitfieldMan(const BitfieldMan& bitfieldMan);
  ~BitfieldMan();

  BitfieldMan& operator=(const BitfieldMan& bitfieldMan);

  int32_t getBlockLength() const { return blockLength_; }

  int32_t getLastBlockLength() const;

  int32_t getBlockLength(size_t index) const;

  int64_t getTotalLength() const { return totalLength_; }

  // Returns true iff there is a bit index which is set in bitfield_,
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
  bool getSparseMissingUnusedIndex(size_t& index, int32_t minSplitSize,
                                   const unsigned char* ignoreBitfield,
                                   size_t ignoreBitfieldLength) const;

  // Stores missing bit index to index. This function first try to
  // select smallest index starting offsetIndex in the order:
  // offsetIndex, offsetIndex+base**1, offsetIndex+base**2, ...  For
  // each sequence [offsetIndex+base**i, offsetIndex+base**(i+1))
  // (first sequence is special case and it is [offsetIndex,
  // offsetIndex+base)) test isBitSet() and isUseBitSet() from the
  // beginning of the sequence.  If isBitSet(x) == false is found
  // first, select x as index.  If isUseBit(x) == true is found first
  // or isBitSet(x) == false is not found, then quit search and go to
  // the next sequence(increment i).  If no index found in the above
  // algorithm, call getSparseMissingUnusedIndex() and return its
  // result.
  //
  // affected by filter
  bool getGeomMissingUnusedIndex(size_t& index, int32_t minSplitSize,
                                 const unsigned char* ignoreBitfield,
                                 size_t ignoreBitfieldLength, double base,
                                 size_t offsetIndex) const;

  // Stores missing bit index to index. This function selects smallest
  // index of missing piece, considering minSplitSize.  Set bits in
  // ignoreBitfield are excluded. Returns true if such bit index is
  // found. Otherwise returns false.
  //
  // affected by filter
  bool getInorderMissingUnusedIndex(size_t& index, int32_t minSplitSize,
                                    const unsigned char* ignoreBitfield,
                                    size_t ignoreBitfieldLength) const;

  // Just like getInorderMissingUnusedIndex() above, but limit the
  // search area in [startIndex, endIndex).  |endIndex| is normalized
  // to min(|endIndex|, blocks_)
  //
  // affected by filter
  bool getInorderMissingUnusedIndex(size_t& index, size_t startIndex,
                                    size_t endIndex, int32_t minSplitSize,
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
  // Returns true if index bit is set in filterBitfield_.  If
  // filterBitfield_ is NULL, returns false.
  bool isFilterBitSet(size_t index) const;

  const unsigned char* getBitfield() const { return bitfield_; }

  size_t getBitfieldLength() const { return bitfieldLength_; }

  // affected by filter
  size_t countFilteredBlock() const { return cachedNumFilteredBlock_; }

  size_t countBlock() const { return blocks_; }

  // affected by filter
  size_t countFilteredBlockNow() const;

  size_t getMaxIndex() const { return blocks_ - 1; }

  void setBitfield(const unsigned char* bitfield, size_t bitfieldLength);

  void clearAllBit();
  void setAllBit();

  void clearAllUseBit();
  void setAllUseBit();

  void addFilter(int64_t offset, int64_t length);
  void removeFilter(int64_t offset, int64_t length);
  // Add filter not in the range of [offset, offset+length) bytes
  void addNotFilter(int64_t offset, int64_t length);

  // Clears filter and disables filter
  void clearFilter();

  void enableFilter();
  void disableFilter();
  bool isFilterEnabled() const { return filterEnabled_; }

  // affected by filter
  int64_t getFilteredTotalLength() const { return cachedFilteredTotalLength_; }

  // affected by filter
  int64_t getFilteredTotalLengthNow() const;

  int64_t getCompletedLength() const { return cachedCompletedLength_; }

  int64_t getCompletedLengthNow() const;

  // affected by filter
  int64_t getFilteredCompletedLength() const
  {
    return cachedFilteredCompletedLength_;
  }

  // affected by filter
  int64_t getFilteredCompletedLengthNow() const;

  void updateCache();

  bool isBitRangeSet(size_t startIndex, size_t endIndex) const;

  void unsetBitRange(size_t startIndex, size_t endIndex);

  void setBitRange(size_t startIndex, size_t endIndex);

  bool isBitSetOffsetRange(int64_t offset, int64_t length) const;

  // Returns completed length in bytes in range [offset,
  // offset+length). This function will not affected by filter.
  int64_t getOffsetCompletedLength(int64_t offset, int64_t length) const;

  int64_t getMissingUnusedLength(size_t startingIndex) const;

  const unsigned char* getFilterBitfield() const { return filterBitfield_; }
};

} // namespace aria2

#endif // D_BITFIELD_MAN_H
