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
#include "BitfieldMan.h"

#include <cassert>
#include <cstring>

#include "array_fun.h"
#include "bitfield.h"

using namespace aria2::expr;

namespace aria2 {

BitfieldMan::BitfieldMan(int32_t blockLength, int64_t totalLength)
    : totalLength_(totalLength),
      cachedCompletedLength_(0),
      cachedFilteredCompletedLength_(0),
      cachedFilteredTotalLength_(0),
      bitfield_(nullptr),
      useBitfield_(nullptr),
      filterBitfield_(nullptr),
      bitfieldLength_(0),
      cachedNumMissingBlock_(0),
      cachedNumFilteredBlock_(0),
      blocks_(0),
      blockLength_(blockLength),
      filterEnabled_(false)
{
  if (blockLength_ > 0 && totalLength_ > 0) {
    blocks_ = (totalLength_ + blockLength_ - 1) / blockLength_;
    bitfieldLength_ = blocks_ / 8 + (blocks_ % 8 ? 1 : 0);
    bitfield_ = new unsigned char[bitfieldLength_];
    useBitfield_ = new unsigned char[bitfieldLength_];
    memset(bitfield_, 0, bitfieldLength_);
    memset(useBitfield_, 0, bitfieldLength_);
    updateCache();
  }
}

BitfieldMan::BitfieldMan(const BitfieldMan& bitfieldMan)
    : totalLength_(bitfieldMan.totalLength_),
      cachedCompletedLength_(0),
      cachedFilteredCompletedLength_(0),
      cachedFilteredTotalLength_(0),
      bitfield_(new unsigned char[bitfieldMan.bitfieldLength_]),
      useBitfield_(new unsigned char[bitfieldMan.bitfieldLength_]),
      filterBitfield_(nullptr),
      bitfieldLength_(bitfieldMan.bitfieldLength_),
      cachedNumMissingBlock_(0),
      cachedNumFilteredBlock_(0),
      blocks_(bitfieldMan.blocks_),
      blockLength_(bitfieldMan.blockLength_),
      filterEnabled_(bitfieldMan.filterEnabled_)
{
  memcpy(bitfield_, bitfieldMan.bitfield_, bitfieldLength_);
  memcpy(useBitfield_, bitfieldMan.useBitfield_, bitfieldLength_);
  if (filterEnabled_) {
    filterBitfield_ = new unsigned char[bitfieldLength_];
    memcpy(filterBitfield_, bitfieldMan.filterBitfield_, bitfieldLength_);
  }
  updateCache();
}

BitfieldMan& BitfieldMan::operator=(const BitfieldMan& bitfieldMan)
{
  if (this != &bitfieldMan) {
    totalLength_ = bitfieldMan.totalLength_;
    blockLength_ = bitfieldMan.blockLength_;
    blocks_ = bitfieldMan.blocks_;
    bitfieldLength_ = bitfieldMan.bitfieldLength_;
    filterEnabled_ = bitfieldMan.filterEnabled_;

    delete[] bitfield_;
    bitfield_ = new unsigned char[bitfieldLength_];
    memcpy(bitfield_, bitfieldMan.bitfield_, bitfieldLength_);

    delete[] useBitfield_;
    useBitfield_ = new unsigned char[bitfieldLength_];
    memcpy(useBitfield_, bitfieldMan.useBitfield_, bitfieldLength_);

    delete[] filterBitfield_;
    if (filterEnabled_) {
      filterBitfield_ = new unsigned char[bitfieldLength_];
      memcpy(filterBitfield_, bitfieldMan.filterBitfield_, bitfieldLength_);
    }
    else {
      filterBitfield_ = nullptr;
    }

    updateCache();
  }
  return *this;
}

BitfieldMan::~BitfieldMan()
{
  delete[] bitfield_;
  delete[] useBitfield_;
  delete[] filterBitfield_;
}

int32_t BitfieldMan::getLastBlockLength() const
{
  return totalLength_ - blockLength_ * (blocks_ - 1);
}

int32_t BitfieldMan::getBlockLength(size_t index) const
{
  if (index == blocks_ - 1) {
    return getLastBlockLength();
  }
  else if (index < blocks_ - 1) {
    return getBlockLength();
  }
  else {
    return 0;
  }
}

bool BitfieldMan::hasMissingPiece(const unsigned char* peerBitfield,
                                  size_t length) const
{
  if (bitfieldLength_ != length) {
    return false;
  }
  bool retval = false;
  for (size_t i = 0; i < bitfieldLength_; ++i) {
    unsigned char temp = peerBitfield[i] & ~bitfield_[i];
    if (filterEnabled_) {
      temp &= filterBitfield_[i];
    }
    if (temp & 0xffu) {
      retval = true;
      break;
    }
  }
  return retval;
}

bool BitfieldMan::getFirstMissingUnusedIndex(size_t& index) const
{
  if (filterEnabled_) {
    return bitfield::getFirstSetBitIndex(
        index,
        ~array(bitfield_) & ~array(useBitfield_) & array(filterBitfield_),
        blocks_);
  }
  else {
    return bitfield::getFirstSetBitIndex(
        index, ~array(bitfield_) & ~array(useBitfield_), blocks_);
  }
}

size_t BitfieldMan::getFirstNMissingUnusedIndex(std::vector<size_t>& out,
                                                size_t n) const
{
  if (filterEnabled_) {
    return bitfield::getFirstNSetBitIndex(
        std::back_inserter(out), n,
        ~array(bitfield_) & ~array(useBitfield_) & array(filterBitfield_),
        blocks_);
  }
  else {
    return bitfield::getFirstNSetBitIndex(
        std::back_inserter(out), n, ~array(bitfield_) & ~array(useBitfield_),
        blocks_);
  }
}

bool BitfieldMan::getFirstMissingIndex(size_t& index) const
{
  if (filterEnabled_) {
    return bitfield::getFirstSetBitIndex(
        index, ~array(bitfield_) & array(filterBitfield_), blocks_);
  }
  else {
    return bitfield::getFirstSetBitIndex(index, ~array(bitfield_), blocks_);
  }
}

namespace {
template <typename Array>
size_t getStartIndex(size_t index, const Array& bitfield, size_t blocks)
{
  while (index < blocks && bitfield::test(bitfield, blocks, index)) {
    ++index;
  }
  if (blocks <= index) {
    return blocks;
  }
  else {
    return index;
  }
}
} // namespace

namespace {
template <typename Array>
size_t getEndIndex(size_t index, const Array& bitfield, size_t blocks)
{
  while (index < blocks && !bitfield::test(bitfield, blocks, index)) {
    ++index;
  }
  return index;
}
} // namespace

namespace {
template <typename Array>
bool getSparseMissingUnusedIndex(size_t& index, int32_t minSplitSize,
                                 const Array& bitfield,
                                 const unsigned char* useBitfield,
                                 int32_t blockLength, size_t blocks)
{
  BitfieldMan::Range maxRange;
  BitfieldMan::Range currentRange;
  size_t nextIndex = 0;
  while (nextIndex < blocks) {
    currentRange.startIndex = getStartIndex(nextIndex, bitfield, blocks);
    if (currentRange.startIndex == blocks) {
      break;
    }
    currentRange.endIndex =
        getEndIndex(currentRange.startIndex, bitfield, blocks);

    if (currentRange.startIndex > 0) {
      if (bitfield::test(useBitfield, blocks, currentRange.startIndex - 1)) {
        currentRange.startIndex = currentRange.getMidIndex();
      }
    }
    // If range is equal, choose a range where its startIndex-1 is
    // set.
    if (maxRange < currentRange ||
        (maxRange == currentRange && maxRange.startIndex > 0 &&
         currentRange.startIndex > 0 &&
         (!bitfield::test(bitfield, blocks, maxRange.startIndex - 1) ||
          bitfield::test(useBitfield, blocks, maxRange.startIndex - 1)) &&
         bitfield::test(bitfield, blocks, currentRange.startIndex - 1) &&
         !bitfield::test(useBitfield, blocks, currentRange.startIndex - 1))) {
      maxRange = currentRange;
    }
    nextIndex = currentRange.endIndex;
  }
  if (maxRange.getSize()) {
    if (maxRange.startIndex == 0) {
      index = 0;
      return true;
    }
    else {
      if ((!bitfield::test(useBitfield, blocks, maxRange.startIndex - 1) &&
           bitfield::test(bitfield, blocks, maxRange.startIndex - 1)) ||
          (static_cast<int64_t>(maxRange.endIndex - maxRange.startIndex) *
               blockLength >=
           minSplitSize)) {
        index = maxRange.startIndex;
        return true;
      }
      else {
        return false;
      }
    }
  }
  else {
    return false;
  }
}
} // namespace

bool BitfieldMan::getSparseMissingUnusedIndex(
    size_t& index, int32_t minSplitSize, const unsigned char* ignoreBitfield,
    size_t ignoreBitfieldLength) const
{
  if (filterEnabled_) {
    return aria2::getSparseMissingUnusedIndex(
        index, minSplitSize,
        array(ignoreBitfield) | ~array(filterBitfield_) | array(bitfield_) |
            array(useBitfield_),
        useBitfield_, blockLength_, blocks_);
  }
  else {
    return aria2::getSparseMissingUnusedIndex(
        index, minSplitSize,
        array(ignoreBitfield) | array(bitfield_) | array(useBitfield_),
        useBitfield_, blockLength_, blocks_);
  }
}

namespace {
template <typename Array>
bool getGeomMissingUnusedIndex(size_t& index, int32_t minSplitSize,
                               const Array& bitfield,
                               const unsigned char* useBitfield,
                               int32_t blockLength, size_t blocks, double base,
                               size_t offsetIndex)
{
  double start = 0;
  double end = 1;
  while (start + offsetIndex < blocks) {
    index = blocks;
    for (size_t i = start + offsetIndex,
                eoi = std::min(blocks, static_cast<size_t>(end + offsetIndex));
         i < eoi; ++i) {
      if (bitfield::test(useBitfield, blocks, i)) {
        break;
      }
      else if (!bitfield::test(bitfield, blocks, i)) {
        index = i;
        break;
      }
    }
    if (index < blocks) {
      return true;
    }
    else {
      start = end;
      end *= base;
    }
  }
  return getSparseMissingUnusedIndex(index, minSplitSize, bitfield, useBitfield,
                                     blockLength, blocks);
}
} // namespace

bool BitfieldMan::getGeomMissingUnusedIndex(size_t& index, int32_t minSplitSize,
                                            const unsigned char* ignoreBitfield,
                                            size_t ignoreBitfieldLength,
                                            double base,
                                            size_t offsetIndex) const
{
  if (filterEnabled_) {
    return aria2::getGeomMissingUnusedIndex(
        index, minSplitSize,
        array(ignoreBitfield) | ~array(filterBitfield_) | array(bitfield_) |
            array(useBitfield_),
        useBitfield_, blockLength_, blocks_, base, offsetIndex);
  }
  else {
    return aria2::getGeomMissingUnusedIndex(
        index, minSplitSize,
        array(ignoreBitfield) | array(bitfield_) | array(useBitfield_),
        useBitfield_, blockLength_, blocks_, base, offsetIndex);
  }
}

namespace {
template <typename Array>
bool getInorderMissingUnusedIndex(size_t& index, size_t startIndex,
                                  size_t lastIndex, int32_t minSplitSize,
                                  const Array& bitfield,
                                  const unsigned char* useBitfield,
                                  int32_t blockLength, size_t blocks)
{
  // We always return first piece if it is available.
  if (!bitfield::test(bitfield, blocks, startIndex) &&
      !bitfield::test(useBitfield, blocks, startIndex)) {
    index = startIndex;
    return true;
  }
  for (size_t i = startIndex + 1; i < lastIndex;) {
    if (!bitfield::test(bitfield, blocks, i) &&
        !bitfield::test(useBitfield, blocks, i)) {
      // If previous piece has already been retrieved, we can download
      // from this index.
      if (!bitfield::test(useBitfield, blocks, i - 1) &&
          bitfield::test(bitfield, blocks, i - 1)) {
        index = i;
        return true;
      }
      // Check free space of minSplitSize.  When checking this, we use
      // blocks instead of lastIndex.
      size_t j;
      for (j = i; j < blocks; ++j) {
        if (bitfield::test(bitfield, blocks, j) ||
            bitfield::test(useBitfield, blocks, j)) {
          break;
        }
        if (static_cast<int64_t>(j - i + 1) * blockLength >= minSplitSize) {
          index = j;
          return true;
        }
      }
      i = j + 1;
    }
    else {
      ++i;
    }
  }
  return false;
}
} // namespace

bool BitfieldMan::getInorderMissingUnusedIndex(
    size_t& index, int32_t minSplitSize, const unsigned char* ignoreBitfield,
    size_t ignoreBitfieldLength) const
{
  if (filterEnabled_) {
    return aria2::getInorderMissingUnusedIndex(
        index, 0, blocks_, minSplitSize,
        array(ignoreBitfield) | ~array(filterBitfield_) | array(bitfield_) |
            array(useBitfield_),
        useBitfield_, blockLength_, blocks_);
  }
  else {
    return aria2::getInorderMissingUnusedIndex(
        index, 0, blocks_, minSplitSize,
        array(ignoreBitfield) | array(bitfield_) | array(useBitfield_),
        useBitfield_, blockLength_, blocks_);
  }
}

bool BitfieldMan::getInorderMissingUnusedIndex(
    size_t& index, size_t startIndex, size_t endIndex, int32_t minSplitSize,
    const unsigned char* ignoreBitfield, size_t ignoreBitfieldLength) const
{
  endIndex = std::min(endIndex, blocks_);
  if (filterEnabled_) {
    return aria2::getInorderMissingUnusedIndex(
        index, startIndex, endIndex, minSplitSize,
        array(ignoreBitfield) | ~array(filterBitfield_) | array(bitfield_) |
            array(useBitfield_),
        useBitfield_, blockLength_, blocks_);
  }
  else {
    return aria2::getInorderMissingUnusedIndex(
        index, startIndex, endIndex, minSplitSize,
        array(ignoreBitfield) | array(bitfield_) | array(useBitfield_),
        useBitfield_, blockLength_, blocks_);
  }
}

namespace {
template <typename Array>
bool copyBitfield(unsigned char* dst, const Array& src, size_t blocks)
{
  unsigned char bits = 0;
  size_t len = (blocks + 7) / 8;
  for (size_t i = 0; i < len - 1; ++i) {
    dst[i] = src[i];
    bits |= dst[i];
  }
  dst[len - 1] = src[len - 1] & bitfield::lastByteMask(blocks);
  bits |= dst[len - 1];
  return bits != 0;
}
} // namespace

bool BitfieldMan::getAllMissingIndexes(unsigned char* misbitfield,
                                       size_t len) const
{
  assert(len == bitfieldLength_);
  if (filterEnabled_) {
    return copyBitfield(misbitfield, ~array(bitfield_) & array(filterBitfield_),
                        blocks_);
  }
  else {
    return copyBitfield(misbitfield, ~array(bitfield_), blocks_);
  }
}

bool BitfieldMan::getAllMissingIndexes(unsigned char* misbitfield, size_t len,
                                       const unsigned char* peerBitfield,
                                       size_t peerBitfieldLength) const
{
  assert(len == bitfieldLength_);
  if (bitfieldLength_ != peerBitfieldLength) {
    return false;
  }
  if (filterEnabled_) {
    return copyBitfield(misbitfield,
                        ~array(bitfield_) & array(peerBitfield) &
                            array(filterBitfield_),
                        blocks_);
  }
  else {
    return copyBitfield(misbitfield, ~array(bitfield_) & array(peerBitfield),
                        blocks_);
  }
}

bool BitfieldMan::getAllMissingUnusedIndexes(unsigned char* misbitfield,
                                             size_t len,
                                             const unsigned char* peerBitfield,
                                             size_t peerBitfieldLength) const
{
  assert(len == bitfieldLength_);
  if (bitfieldLength_ != peerBitfieldLength) {
    return false;
  }
  if (filterEnabled_) {
    return copyBitfield(misbitfield,
                        ~array(bitfield_) & ~array(useBitfield_) &
                            array(peerBitfield) & array(filterBitfield_),
                        blocks_);
  }
  else {
    return copyBitfield(misbitfield,
                        ~array(bitfield_) & ~array(useBitfield_) &
                            array(peerBitfield),
                        blocks_);
  }
}

size_t BitfieldMan::countMissingBlock() const { return cachedNumMissingBlock_; }

size_t BitfieldMan::countMissingBlockNow() const
{
  if (filterEnabled_) {
    return bitfield::countSetBit(filterBitfield_, blocks_) -
           bitfield::countSetBitSlow(array(bitfield_) & array(filterBitfield_),
                                     blocks_);
  }
  else {
    return blocks_ - bitfield::countSetBit(bitfield_, blocks_);
  }
}

size_t BitfieldMan::countFilteredBlockNow() const
{
  if (filterEnabled_) {
    return bitfield::countSetBit(filterBitfield_, blocks_);
  }
  else {
    return 0;
  }
}

bool BitfieldMan::setBitInternal(unsigned char* bitfield, size_t index, bool on)
{
  if (blocks_ <= index) {
    return false;
  }
  unsigned char mask = 128 >> (index % 8);
  if (on) {
    bitfield[index / 8] |= mask;
  }
  else {
    bitfield[index / 8] &= ~mask;
  }
  return true;
}

bool BitfieldMan::setUseBit(size_t index)
{
  return setBitInternal(useBitfield_, index, true);
}

bool BitfieldMan::unsetUseBit(size_t index)
{
  return setBitInternal(useBitfield_, index, false);
}

bool BitfieldMan::setBit(size_t index)
{
  bool b = setBitInternal(bitfield_, index, true);
  updateCache();
  return b;
}

bool BitfieldMan::unsetBit(size_t index)
{
  bool b = setBitInternal(bitfield_, index, false);
  updateCache();
  return b;
}

bool BitfieldMan::isFilteredAllBitSet() const
{
  if (filterEnabled_) {
    for (size_t i = 0; i < bitfieldLength_; ++i) {
      if ((bitfield_[i] & filterBitfield_[i]) != filterBitfield_[i]) {
        return false;
      }
    }
    return true;
  }
  else {
    return isAllBitSet();
  }
}

namespace {
bool testAllBitSet(const unsigned char* bitfield, size_t length, size_t blocks)
{
  if (length == 0) {
    return true;
  }
  for (size_t i = 0; i < length - 1; ++i) {
    if (bitfield[i] != 0xffu) {
      return false;
    }
  }
  return bitfield[length - 1] == bitfield::lastByteMask(blocks);
}
} // namespace

bool BitfieldMan::isAllBitSet() const
{
  return testAllBitSet(bitfield_, bitfieldLength_, blocks_);
}

bool BitfieldMan::isAllFilterBitSet() const
{
  if (!filterBitfield_) {
    return false;
  }
  return testAllBitSet(filterBitfield_, bitfieldLength_, blocks_);
}

bool BitfieldMan::isFilterBitSet(size_t index) const
{
  if (filterBitfield_) {
    return bitfield::test(filterBitfield_, blocks_, index);
  }
  else {
    return false;
  }
}

bool BitfieldMan::isBitSet(size_t index) const
{
  return bitfield::test(bitfield_, blocks_, index);
}

bool BitfieldMan::isUseBitSet(size_t index) const
{
  return bitfield::test(useBitfield_, blocks_, index);
}

void BitfieldMan::setBitfield(const unsigned char* bitfield,
                              size_t bitfieldLength)
{
  if (bitfieldLength_ != bitfieldLength) {
    return;
  }
  memcpy(bitfield_, bitfield, bitfieldLength_);
  memset(useBitfield_, 0, bitfieldLength_);
  updateCache();
}

void BitfieldMan::clearAllBit()
{
  memset(bitfield_, 0, bitfieldLength_);
  updateCache();
}

void BitfieldMan::setAllBit()
{
  for (size_t i = 0; i < blocks_; ++i) {
    setBitInternal(bitfield_, i, true);
  }
  updateCache();
}

void BitfieldMan::clearAllUseBit()
{
  memset(useBitfield_, 0, bitfieldLength_);
  updateCache();
}

void BitfieldMan::setAllUseBit()
{
  for (size_t i = 0; i < blocks_; ++i) {
    setBitInternal(useBitfield_, i, true);
  }
}

bool BitfieldMan::setFilterBit(size_t index)
{
  return setBitInternal(filterBitfield_, index, true);
}

void BitfieldMan::ensureFilterBitfield()
{
  if (!filterBitfield_) {
    filterBitfield_ = new unsigned char[bitfieldLength_];
    memset(filterBitfield_, 0, bitfieldLength_);
  }
}

void BitfieldMan::addFilter(int64_t offset, int64_t length)
{
  ensureFilterBitfield();
  if (length > 0) {
    size_t startBlock = offset / blockLength_;
    size_t endBlock = (offset + length - 1) / blockLength_;
    for (size_t i = startBlock; i <= endBlock && i < blocks_; i++) {
      setFilterBit(i);
    }
  }
  updateCache();
}

void BitfieldMan::removeFilter(int64_t offset, int64_t length)
{
  ensureFilterBitfield();
  if (length > 0) {
    size_t startBlock = offset / blockLength_;
    size_t endBlock = (offset + length - 1) / blockLength_;
    for (size_t i = startBlock; i <= endBlock && i < blocks_; i++) {
      setBitInternal(filterBitfield_, i, false);
    }
  }
  updateCache();
}

void BitfieldMan::addNotFilter(int64_t offset, int64_t length)
{
  ensureFilterBitfield();
  if (length > 0 && blocks_ > 0) {
    size_t startBlock = offset / blockLength_;
    if (blocks_ <= startBlock) {
      startBlock = blocks_;
    }
    size_t endBlock = (offset + length - 1) / blockLength_;
    for (size_t i = 0; i < startBlock; ++i) {
      setFilterBit(i);
    }
    for (size_t i = endBlock + 1; i < blocks_; ++i) {
      setFilterBit(i);
    }
  }
  updateCache();
}

void BitfieldMan::enableFilter()
{
  ensureFilterBitfield();
  filterEnabled_ = true;
  updateCache();
}

void BitfieldMan::disableFilter()
{
  filterEnabled_ = false;
  updateCache();
}

void BitfieldMan::clearFilter()
{
  if (filterBitfield_) {
    delete[] filterBitfield_;
    filterBitfield_ = nullptr;
  }
  filterEnabled_ = false;
  updateCache();
}

int64_t BitfieldMan::getFilteredTotalLengthNow() const
{
  if (!filterBitfield_) {
    return 0;
  }
  size_t filteredBlocks = bitfield::countSetBit(filterBitfield_, blocks_);
  if (filteredBlocks == 0) {
    return 0;
  }
  if (bitfield::test(filterBitfield_, blocks_, blocks_ - 1)) {
    return ((int64_t)filteredBlocks - 1) * blockLength_ + getLastBlockLength();
  }
  else {
    return ((int64_t)filteredBlocks) * blockLength_;
  }
}

namespace {
template <typename Array, typename CountFun>
int64_t computeCompletedLength(const Array& bitfield, const BitfieldMan* btman,
                               CountFun cntfun)
{
  size_t nbits = btman->countBlock();
  size_t completedBlocks = cntfun(bitfield, nbits);
  int64_t completedLength = 0;
  if (completedBlocks == 0) {
    completedLength = 0;
  }
  else {
    if (bitfield::test(bitfield, nbits, nbits - 1)) {
      completedLength =
          ((int64_t)completedBlocks - 1) * btman->getBlockLength() +
          btman->getLastBlockLength();
    }
    else {
      completedLength = ((int64_t)completedBlocks) * btman->getBlockLength();
    }
  }
  return completedLength;
}
} // namespace

int64_t BitfieldMan::getCompletedLength(bool useFilter) const
{
  if (useFilter && filterEnabled_) {
    auto arr = array(bitfield_) & array(filterBitfield_);
    return computeCompletedLength(arr, this,
                                  &bitfield::countSetBitSlow<decltype(arr)>);
  }
  else {
    return computeCompletedLength(bitfield_, this, &bitfield::countSetBit);
  }
}

int64_t BitfieldMan::getCompletedLengthNow() const
{
  return getCompletedLength(false);
}

int64_t BitfieldMan::getFilteredCompletedLengthNow() const
{
  return getCompletedLength(true);
}

void BitfieldMan::updateCache()
{
  cachedNumMissingBlock_ = countMissingBlockNow();
  cachedNumFilteredBlock_ = countFilteredBlockNow();
  cachedFilteredTotalLength_ = getFilteredTotalLengthNow();
  cachedCompletedLength_ = getCompletedLengthNow();
  cachedFilteredCompletedLength_ = getFilteredCompletedLengthNow();
}

bool BitfieldMan::isBitRangeSet(size_t startIndex, size_t endIndex) const
{
  for (size_t i = startIndex; i <= endIndex; ++i) {
    if (!isBitSet(i)) {
      return false;
    }
  }
  return true;
}

void BitfieldMan::unsetBitRange(size_t startIndex, size_t endIndex)
{
  for (size_t i = startIndex; i <= endIndex; ++i) {
    unsetBit(i);
  }
  updateCache();
}

void BitfieldMan::setBitRange(size_t startIndex, size_t endIndex)
{
  for (size_t i = startIndex; i <= endIndex; ++i) {
    setBit(i);
  }
  updateCache();
}

bool BitfieldMan::isBitSetOffsetRange(int64_t offset, int64_t length) const
{
  if (length <= 0) {
    return false;
  }
  if (totalLength_ <= offset) {
    return false;
  }
  if (totalLength_ < offset + length) {
    length = totalLength_ - offset;
  }
  size_t startBlock = offset / blockLength_;
  size_t endBlock = (offset + length - 1) / blockLength_;
  for (size_t i = startBlock; i <= endBlock; i++) {
    if (!isBitSet(i)) {
      return false;
    }
  }
  return true;
}

int64_t BitfieldMan::getOffsetCompletedLength(int64_t offset,
                                              int64_t length) const
{
  int64_t res = 0;
  if (length == 0 || totalLength_ <= offset) {
    return 0;
  }
  if (totalLength_ < offset + length) {
    length = totalLength_ - offset;
  }
  size_t start = offset / blockLength_;
  size_t end = (offset + length - 1) / blockLength_;
  if (start == end) {
    if (isBitSet(start)) {
      res = length;
    }
  }
  else {
    if (isBitSet(start)) {
      res += static_cast<int64_t>(start + 1) * blockLength_ - offset;
    }
    for (size_t i = start + 1; i <= end - 1; ++i) {
      if (isBitSet(i)) {
        res += blockLength_;
      }
    }
    if (isBitSet(end)) {
      res += offset + length - static_cast<int64_t>(end) * blockLength_;
    }
  }
  return res;
}

int64_t BitfieldMan::getMissingUnusedLength(size_t startingIndex) const
{
  if (blocks_ <= startingIndex) {
    return 0;
  }
  int64_t length = 0;
  for (size_t i = startingIndex; i < blocks_; ++i) {
    if (isBitSet(i) || isUseBitSet(i)) {
      break;
    }
    length += getBlockLength(i);
  }
  return length;
}

BitfieldMan::Range::Range(size_t startIndex, size_t endIndex)
    : startIndex(startIndex), endIndex(endIndex)
{
}

size_t BitfieldMan::Range::getSize() const { return endIndex - startIndex; }

size_t BitfieldMan::Range::getMidIndex() const
{
  return (endIndex - startIndex) / 2 + startIndex;
}

bool BitfieldMan::Range::operator<(const Range& range) const
{
  return getSize() < range.getSize();
}

bool BitfieldMan::Range::operator==(const Range& range) const
{
  return getSize() == range.getSize();
}

} // namespace aria2
