#include "BitfieldMan.h"

#include <cstring>
#include <vector>

#include <cppunit/extensions/HelperMacros.h>

#include "bitfield.h"
#include "array_fun.h"

namespace aria2 {

class BitfieldManTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BitfieldManTest);
  CPPUNIT_TEST(testGetBlockSize);
  CPPUNIT_TEST(testGetFirstMissingUnusedIndex);
  CPPUNIT_TEST(testGetFirstMissingIndex);
  CPPUNIT_TEST(testIsAllBitSet);
  CPPUNIT_TEST(testFilter);
  CPPUNIT_TEST(testIsFilterBitSet);
  CPPUNIT_TEST(testAddFilter_zeroLength);
  CPPUNIT_TEST(testAddNotFilter);
  CPPUNIT_TEST(testAddNotFilter_zeroLength);
  CPPUNIT_TEST(testAddNotFilter_overflow);
  CPPUNIT_TEST(testGetSparseMissingUnusedIndex);
  CPPUNIT_TEST(testGetSparseMissingUnusedIndex_setBit);
  CPPUNIT_TEST(testGetSparseMissingUnusedIndex_withMinSplitSize);
  CPPUNIT_TEST(testIsBitSetOffsetRange);
  CPPUNIT_TEST(testGetOffsetCompletedLength);
  CPPUNIT_TEST(testGetMissingUnusedLength);
  CPPUNIT_TEST(testSetBitRange);
  CPPUNIT_TEST(testGetAllMissingIndexes);
  CPPUNIT_TEST(testGetAllMissingIndexes_noarg);
  CPPUNIT_TEST(testGetAllMissingIndexes_checkLastByte);
  CPPUNIT_TEST(testGetAllMissingUnusedIndexes);
  CPPUNIT_TEST(testCountFilteredBlock);
  CPPUNIT_TEST(testCountMissingBlock);
  CPPUNIT_TEST(testZeroLengthFilter);
  CPPUNIT_TEST(testGetFirstNMissingUnusedIndex);
  CPPUNIT_TEST(testGetInorderMissingUnusedIndex);
  CPPUNIT_TEST(testGetGeomMissingUnusedIndex);
  CPPUNIT_TEST_SUITE_END();
public:
  void testGetBlockSize();
  void testGetFirstMissingUnusedIndex();
  void testGetFirstMissingIndex();
  void testGetAllMissingIndexes();
  void testGetAllMissingIndexes_noarg();
  void testGetAllMissingIndexes_checkLastByte();
  void testGetAllMissingUnusedIndexes();
  
  void testIsAllBitSet();
  void testFilter();
  void testIsFilterBitSet();
  void testAddFilter_zeroLength();
  void testAddNotFilter();
  void testAddNotFilter_zeroLength();
  void testAddNotFilter_overflow();
  void testGetSparseMissingUnusedIndex();
  void testGetSparseMissingUnusedIndex_setBit();
  void testGetSparseMissingUnusedIndex_withMinSplitSize();
  void testIsBitSetOffsetRange();
  void testGetOffsetCompletedLength();
  void testGetMissingUnusedLength();
  void testSetBitRange();
  void testCountFilteredBlock();
  void testCountMissingBlock();
  void testZeroLengthFilter();
  void testGetFirstNMissingUnusedIndex();
  void testGetInorderMissingUnusedIndex();
  void testGetGeomMissingUnusedIndex();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BitfieldManTest );

void BitfieldManTest::testGetBlockSize() {
  BitfieldMan bt1(1024, 1024*10);
  CPPUNIT_ASSERT_EQUAL(1024, bt1.getBlockLength(9));

  BitfieldMan bt2(1024, 1024*10+1);
  CPPUNIT_ASSERT_EQUAL(1024, bt2.getBlockLength(9));
  CPPUNIT_ASSERT_EQUAL(1, bt2.getBlockLength(10));
  CPPUNIT_ASSERT_EQUAL(0, bt2.getBlockLength(11));
}

void BitfieldManTest::testGetFirstMissingUnusedIndex()
{
  {
    BitfieldMan bt1(1024, 1024*10);
    {
      size_t index;
      CPPUNIT_ASSERT(bt1.getFirstMissingUnusedIndex(index));
      CPPUNIT_ASSERT_EQUAL((size_t)0, index);
    }
    bt1.setUseBit(0);
    {
      size_t index;
      CPPUNIT_ASSERT(bt1.getFirstMissingUnusedIndex(index));
      CPPUNIT_ASSERT_EQUAL((size_t)1, index);
    }
    bt1.unsetUseBit(0);
    bt1.setBit(0);
    {
      size_t index;
      CPPUNIT_ASSERT(bt1.getFirstMissingUnusedIndex(index));
      CPPUNIT_ASSERT_EQUAL((size_t)1, index);
    }
    bt1.setAllBit();
    {
      size_t index;
      CPPUNIT_ASSERT(!bt1.getFirstMissingUnusedIndex(index));
    }
  }
  {
    BitfieldMan bt1(1024, 1024*10);

    bt1.addFilter(1024, 1024*10);
    bt1.enableFilter();
    {
      size_t index;
      CPPUNIT_ASSERT(bt1.getFirstMissingUnusedIndex(index));
      CPPUNIT_ASSERT_EQUAL((size_t)1, index);
    }
    bt1.setUseBit(1);
    {
      size_t index;
      CPPUNIT_ASSERT(bt1.getFirstMissingUnusedIndex(index));
      CPPUNIT_ASSERT_EQUAL((size_t)2, index);
    }
    bt1.setBit(2);
    {
      size_t index;
      CPPUNIT_ASSERT(bt1.getFirstMissingUnusedIndex(index));
      CPPUNIT_ASSERT_EQUAL((size_t)3, index);
    }
  }
}

void BitfieldManTest::testGetFirstMissingIndex()
{
  {
    BitfieldMan bt1(1024, 1024*10);
    {
      size_t index;
      CPPUNIT_ASSERT(bt1.getFirstMissingIndex(index));
      CPPUNIT_ASSERT_EQUAL((size_t)0, index);
    }
    bt1.setUseBit(0);
    {
      size_t index;
      CPPUNIT_ASSERT(bt1.getFirstMissingIndex(index));
      CPPUNIT_ASSERT_EQUAL((size_t)0, index);
    }
    bt1.unsetUseBit(0);
    bt1.setBit(0);
    {
      size_t index;
      CPPUNIT_ASSERT(bt1.getFirstMissingIndex(index));
      CPPUNIT_ASSERT_EQUAL((size_t)1, index);
    }
    bt1.setAllBit();
    {
      size_t index;
      CPPUNIT_ASSERT(!bt1.getFirstMissingIndex(index));
    }
  }
  {
    BitfieldMan bt1(1024, 1024*10);

    bt1.addFilter(1024, 1024*10);
    bt1.enableFilter();
    {
      size_t index;
      CPPUNIT_ASSERT(bt1.getFirstMissingIndex(index));
      CPPUNIT_ASSERT_EQUAL((size_t)1, index);
    }
    bt1.setUseBit(1);
    {
      size_t index;
      CPPUNIT_ASSERT(bt1.getFirstMissingIndex(index));
      CPPUNIT_ASSERT_EQUAL((size_t)1, index);
    }
    bt1.setBit(1);
    {
      size_t index;
      CPPUNIT_ASSERT(bt1.getFirstMissingIndex(index));
      CPPUNIT_ASSERT_EQUAL((size_t)2, index);
    }
  }
}

void BitfieldManTest::testIsAllBitSet() {
  BitfieldMan bt1(1024, 1024*10);
  CPPUNIT_ASSERT(!bt1.isAllBitSet());
  bt1.setBit(1);
  CPPUNIT_ASSERT(!bt1.isAllBitSet());
  
  for(size_t i = 0; i < 8; i++) {
    CPPUNIT_ASSERT(bt1.setBit(i));
  }
  CPPUNIT_ASSERT(!bt1.isAllBitSet());

  for(size_t i = 0; i < bt1.countBlock(); i++) {
    CPPUNIT_ASSERT(bt1.setBit(i));
  }
  CPPUNIT_ASSERT(bt1.isAllBitSet());

  BitfieldMan btzero(1024, 0);
  CPPUNIT_ASSERT(btzero.isAllBitSet());
}

void BitfieldManTest::testFilter()
{
  BitfieldMan btman(2, 32);
  // test offset=4, length=12
  btman.addFilter(4, 12);
  btman.enableFilter();
  std::vector<size_t> out;
  CPPUNIT_ASSERT_EQUAL((size_t)6, btman.getFirstNMissingUnusedIndex(out, 32));
  const size_t ans[] = { 2, 3, 4, 5, 6, 7 };
  for(size_t i = 0; i < A2_ARRAY_LEN(ans); ++i) {
    CPPUNIT_ASSERT_EQUAL(ans[i], out[i]);
  }
  CPPUNIT_ASSERT_EQUAL((int64_t)12ULL, btman.getFilteredTotalLength());

  // test offset=5, length=2
  out.clear();
  btman.clearAllBit();
  btman.clearAllUseBit();
  btman.clearFilter();
  btman.addFilter(5, 2);
  btman.enableFilter();
  CPPUNIT_ASSERT_EQUAL((size_t)2, btman.getFirstNMissingUnusedIndex(out, 32));
  CPPUNIT_ASSERT_EQUAL((size_t)2, out[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)3, out[1]);
  btman.setBit(2);
  btman.setBit(3);
  CPPUNIT_ASSERT_EQUAL((int64_t)4ULL, btman.getFilteredTotalLength());
  CPPUNIT_ASSERT(btman.isFilteredAllBitSet());

  BitfieldMan btman2(2, 31);
  btman2.addFilter(0, 31);
  btman2.enableFilter();
  CPPUNIT_ASSERT_EQUAL((int64_t)31ULL, btman2.getFilteredTotalLength());
}

void BitfieldManTest::testIsFilterBitSet()
{
  BitfieldMan btman(2, 32);
  CPPUNIT_ASSERT(!btman.isFilterBitSet(0));
  btman.addFilter(0, 2);
  CPPUNIT_ASSERT(btman.isFilterBitSet(0));
  CPPUNIT_ASSERT(!btman.isFilterBitSet(1));
  btman.addFilter(2, 4);
  CPPUNIT_ASSERT(btman.isFilterBitSet(1));
}

void BitfieldManTest::testAddFilter_zeroLength()
{
  BitfieldMan bits(1024, 1024*1024);
  bits.addFilter(2048, 0);
  bits.enableFilter();
  CPPUNIT_ASSERT_EQUAL((size_t)0, bits.countMissingBlock());
  CPPUNIT_ASSERT(bits.isFilteredAllBitSet());
}

void BitfieldManTest::testAddNotFilter() {
  BitfieldMan btman(2, 32);

  btman.addNotFilter(3, 6);
  CPPUNIT_ASSERT(bitfield::test(btman.getFilterBitfield(), 16, 0));
  for(size_t i = 1; i < 5; ++i) {
    CPPUNIT_ASSERT(!bitfield::test(btman.getFilterBitfield(), 16, i));
  }
  for(size_t i = 5; i < 16; ++i) {
    CPPUNIT_ASSERT(bitfield::test(btman.getFilterBitfield(), 16, i));
  }
}

void BitfieldManTest::testAddNotFilter_zeroLength() {
  BitfieldMan btman(2, 6);
  btman.addNotFilter(2, 0);
  CPPUNIT_ASSERT(!bitfield::test(btman.getFilterBitfield(), 3, 0));
  CPPUNIT_ASSERT(!bitfield::test(btman.getFilterBitfield(), 3, 1));
  CPPUNIT_ASSERT(!bitfield::test(btman.getFilterBitfield(), 3, 2));
}

void BitfieldManTest::testAddNotFilter_overflow() {
  BitfieldMan btman(2, 6);
  btman.addNotFilter(6, 100);
  CPPUNIT_ASSERT(bitfield::test(btman.getFilterBitfield(), 3, 0));
  CPPUNIT_ASSERT(bitfield::test(btman.getFilterBitfield(), 3, 1));
  CPPUNIT_ASSERT(bitfield::test(btman.getFilterBitfield(), 3, 2));
}

// TODO1.5 add test using ignoreBitfield
void BitfieldManTest::testGetSparseMissingUnusedIndex() {
  BitfieldMan bitfield(1024*1024, 10*1024*1024);
  const size_t length = 2;
  unsigned char ignoreBitfield[length];
  memset(ignoreBitfield, 0, sizeof(ignoreBitfield));
  size_t minSplitSize = 1024*1024;
  size_t index;
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)0, index);
  bitfield.setUseBit(0);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)5, index);
  bitfield.setUseBit(5);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)3, index);
  bitfield.setUseBit(3);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)8, index);
  bitfield.setUseBit(8);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)2, index);
  bitfield.setUseBit(2);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)1, index);
  bitfield.setUseBit(1);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)4, index);
  bitfield.setUseBit(4);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)7, index);
  bitfield.setUseBit(7);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)6, index);
  bitfield.setUseBit(6);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)9, index);
  bitfield.setUseBit(9);
  CPPUNIT_ASSERT(!bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                       ignoreBitfield, length));
}

void BitfieldManTest::testGetSparseMissingUnusedIndex_setBit() {
  BitfieldMan bitfield(1024*1024, 10*1024*1024);
  const size_t length = 2;
  unsigned char ignoreBitfield[length];
  memset(ignoreBitfield, 0, sizeof(ignoreBitfield));
  size_t minSplitSize = 1024*1024;
  size_t index;
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)0, index);
  bitfield.setBit(0);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)1, index);
  bitfield.setBit(1);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)2, index);
  bitfield.setBit(2);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)3, index);
  bitfield.setBit(3);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)4, index);
  bitfield.setBit(4);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)5, index);
  bitfield.setBit(5);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)6, index);
  bitfield.setBit(6);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)7, index);
  bitfield.setBit(7);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)8, index);
  bitfield.setBit(8);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)9, index);
  bitfield.setBit(9);
  CPPUNIT_ASSERT(!bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                       ignoreBitfield, length));
}

void BitfieldManTest::testGetSparseMissingUnusedIndex_withMinSplitSize()
{
  BitfieldMan bitfield(1024*1024, 10*1024*1024);
  const size_t length = 2;
  unsigned char ignoreBitfield[length];
  memset(ignoreBitfield, 0, sizeof(ignoreBitfield));
  size_t minSplitSize = 2*1024*1024;
  size_t index;
  bitfield.setUseBit(1);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)6, index);
  bitfield.setBit(6);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)7, index);
  bitfield.setUseBit(7);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)4, index);
  bitfield.setBit(4);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)0, index);
  bitfield.setBit(0);
  CPPUNIT_ASSERT(bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                      ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)5, index);
  bitfield.setBit(5);  
  CPPUNIT_ASSERT(!bitfield.getSparseMissingUnusedIndex(index, minSplitSize,
                                                       ignoreBitfield, length));
}

void BitfieldManTest::testIsBitSetOffsetRange()
{
  int64_t totalLength = 4ULL*1024*1024*1024;
  int32_t pieceLength = 4*1024*1024;
  BitfieldMan bitfield(pieceLength, totalLength);
  bitfield.setAllBit();

  CPPUNIT_ASSERT(!bitfield.isBitSetOffsetRange(0, 0));
  CPPUNIT_ASSERT(!bitfield.isBitSetOffsetRange(totalLength, 100));
  CPPUNIT_ASSERT(!bitfield.isBitSetOffsetRange(totalLength+1, 100));

  CPPUNIT_ASSERT(bitfield.isBitSetOffsetRange(0, totalLength));
  CPPUNIT_ASSERT(bitfield.isBitSetOffsetRange(0, totalLength+1));

  bitfield.clearAllBit();

  bitfield.setBit(100);
  bitfield.setBit(101);
  
  CPPUNIT_ASSERT(bitfield.isBitSetOffsetRange(pieceLength*100, pieceLength*2));
  CPPUNIT_ASSERT(!bitfield.isBitSetOffsetRange(pieceLength*100-10, pieceLength*2));
  CPPUNIT_ASSERT(!bitfield.isBitSetOffsetRange(pieceLength*100, pieceLength*2+1));
    
  bitfield.clearAllBit();

  bitfield.setBit(100);
  bitfield.setBit(102);

  CPPUNIT_ASSERT(!bitfield.isBitSetOffsetRange(pieceLength*100, pieceLength*3));
}

void BitfieldManTest::testGetOffsetCompletedLength()
{
  BitfieldMan bt(1024, 1024*20);
  // 00000|00000|00000|00000
  CPPUNIT_ASSERT_EQUAL((int64_t)0, bt.getOffsetCompletedLength(0, 1024));
  CPPUNIT_ASSERT_EQUAL((int64_t)0, bt.getOffsetCompletedLength(0, 0));
  for(size_t i = 2; i <= 4; ++i) {
    bt.setBit(i);
  }
  // 00111|00000|00000|00000
  CPPUNIT_ASSERT_EQUAL((int64_t)3072, bt.getOffsetCompletedLength(2048, 3072));
  CPPUNIT_ASSERT_EQUAL((int64_t)3071, bt.getOffsetCompletedLength(2047, 3072));
  CPPUNIT_ASSERT_EQUAL((int64_t)3071, bt.getOffsetCompletedLength(2049, 3072));
  CPPUNIT_ASSERT_EQUAL((int64_t)0, bt.getOffsetCompletedLength(2048, 0));
  CPPUNIT_ASSERT_EQUAL((int64_t)1, bt.getOffsetCompletedLength(2048, 1));
  CPPUNIT_ASSERT_EQUAL((int64_t)0, bt.getOffsetCompletedLength(2047, 1));
  CPPUNIT_ASSERT_EQUAL((int64_t)3072, bt.getOffsetCompletedLength(0, 1024*20));
  CPPUNIT_ASSERT_EQUAL((int64_t)3072,
                       bt.getOffsetCompletedLength(0, 1024*20+10));
  CPPUNIT_ASSERT_EQUAL((int64_t)0, bt.getOffsetCompletedLength(1024*20, 1));
}

void BitfieldManTest::testGetMissingUnusedLength()
{
  int64_t totalLength = 1024*10+10;
  size_t blockLength = 1024;

  BitfieldMan bf(blockLength, totalLength);

  // from index 0 and all blocks are unused and not acquired.
  CPPUNIT_ASSERT_EQUAL(totalLength, bf.getMissingUnusedLength(0));

  // from index 10 and all blocks are unused and not acquired.
  CPPUNIT_ASSERT_EQUAL((int64_t)10ULL, bf.getMissingUnusedLength(10));

  // from index 11
  CPPUNIT_ASSERT_EQUAL((int64_t)0ULL, bf.getMissingUnusedLength(11));

  // from index 12
  CPPUNIT_ASSERT_EQUAL((int64_t)0ULL, bf.getMissingUnusedLength(12));

  // from index 0 and 5th block is used.
  bf.setUseBit(5);
  CPPUNIT_ASSERT_EQUAL((int64_t)(5LL*blockLength), bf.getMissingUnusedLength(0));

  // from index 0 and 4th block is acquired.
  bf.setBit(4);
  CPPUNIT_ASSERT_EQUAL((int64_t)(4LL*blockLength), bf.getMissingUnusedLength(0));

  // from index 1
  CPPUNIT_ASSERT_EQUAL((int64_t)(3LL*blockLength), bf.getMissingUnusedLength(1));
}

void BitfieldManTest::testSetBitRange()
{
  size_t blockLength = 1024*1024;
  int64_t totalLength = 10*blockLength;

  BitfieldMan bf(blockLength, totalLength);

  bf.setBitRange(0, 4);

  for(size_t i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(bf.isBitSet(i));
  }
  for(size_t i = 5; i < 10; ++i) {
    CPPUNIT_ASSERT(!bf.isBitSet(i));
  }
  CPPUNIT_ASSERT_EQUAL((int64_t)(5LL*blockLength), bf.getCompletedLength());
}

void BitfieldManTest::testGetAllMissingIndexes_noarg()
{
  size_t blockLength = 16*1024;
  int64_t totalLength = 1024*1024;
  size_t nbits = (totalLength+blockLength-1)/blockLength;
  BitfieldMan bf(blockLength, totalLength);
  unsigned char misbitfield[8];
  CPPUNIT_ASSERT(bf.getAllMissingIndexes(misbitfield, sizeof(misbitfield)));
  CPPUNIT_ASSERT_EQUAL((size_t)64, bitfield::countSetBit(misbitfield, nbits));

  for(size_t i = 0; i < 63; ++i) {
    bf.setBit(i);
  }
  CPPUNIT_ASSERT(bf.getAllMissingIndexes(misbitfield, sizeof(misbitfield)));
  CPPUNIT_ASSERT_EQUAL((size_t)1, bitfield::countSetBit(misbitfield, nbits));
  CPPUNIT_ASSERT(bitfield::test(misbitfield, nbits, 63));
}

// See garbage bits of last byte are 0
void BitfieldManTest::testGetAllMissingIndexes_checkLastByte()
{
  size_t blockLength = 16*1024;
  int64_t totalLength = blockLength*2;
  size_t nbits = (totalLength+blockLength-1)/blockLength;
  BitfieldMan bf(blockLength, totalLength);
  unsigned char misbitfield[1];
  CPPUNIT_ASSERT(bf.getAllMissingIndexes(misbitfield, sizeof(misbitfield)));
  CPPUNIT_ASSERT_EQUAL((size_t)2, bitfield::countSetBit(misbitfield, nbits));
  CPPUNIT_ASSERT(bitfield::test(misbitfield, nbits, 0));
  CPPUNIT_ASSERT(bitfield::test(misbitfield, nbits, 1));
}

void BitfieldManTest::testGetAllMissingIndexes()
{
  size_t blockLength = 16*1024;
  int64_t totalLength = 1024*1024;
  size_t nbits = (totalLength+blockLength-1)/blockLength;
  BitfieldMan bf(blockLength, totalLength);
  BitfieldMan peerBf(blockLength, totalLength);
  peerBf.setAllBit();
  unsigned char misbitfield[8];

  CPPUNIT_ASSERT(bf.getAllMissingIndexes(misbitfield, sizeof(misbitfield),
                                         peerBf.getBitfield(),
                                         peerBf.getBitfieldLength()));
  CPPUNIT_ASSERT_EQUAL((size_t)64, bitfield::countSetBit(misbitfield, nbits));
  for(size_t i = 0; i < 62; ++i) {
    bf.setBit(i);
  }
  peerBf.unsetBit(62);

  CPPUNIT_ASSERT(bf.getAllMissingIndexes(misbitfield, sizeof(misbitfield),
                                         peerBf.getBitfield(),
                                         peerBf.getBitfieldLength()));
  CPPUNIT_ASSERT_EQUAL((size_t)1, bitfield::countSetBit(misbitfield, nbits));
  CPPUNIT_ASSERT(bitfield::test(misbitfield, nbits, 63));
}

void BitfieldManTest::testGetAllMissingUnusedIndexes()
{
  size_t blockLength = 16*1024;
  int64_t totalLength = 1024*1024;
  size_t nbits = (totalLength+blockLength-1)/blockLength;
  BitfieldMan bf(blockLength, totalLength);
  BitfieldMan peerBf(blockLength, totalLength);
  peerBf.setAllBit();
  unsigned char misbitfield[8];

  CPPUNIT_ASSERT(bf.getAllMissingUnusedIndexes(misbitfield,
                                               sizeof(misbitfield),
                                               peerBf.getBitfield(),
                                               peerBf.getBitfieldLength()));
  CPPUNIT_ASSERT_EQUAL((size_t)64, bitfield::countSetBit(misbitfield, nbits));

  for(size_t i = 0; i < 61; ++i) {
    bf.setBit(i);
  }
  bf.setUseBit(61);
  peerBf.unsetBit(62);
  CPPUNIT_ASSERT(bf.getAllMissingUnusedIndexes(misbitfield,
                                               sizeof(misbitfield),
                                               peerBf.getBitfield(),
                                               peerBf.getBitfieldLength()));
  CPPUNIT_ASSERT_EQUAL((size_t)1, bitfield::countSetBit(misbitfield, nbits));
  CPPUNIT_ASSERT(bitfield::test(misbitfield, nbits, 63));
}

void BitfieldManTest::testCountFilteredBlock()
{
  BitfieldMan bt(1024, 1024*256);
  CPPUNIT_ASSERT_EQUAL((size_t)256, bt.countBlock());
  CPPUNIT_ASSERT_EQUAL((size_t)0, bt.countFilteredBlock());
  bt.addFilter(1024, 1024*256);
  bt.enableFilter();
  CPPUNIT_ASSERT_EQUAL((size_t)256, bt.countBlock());
  CPPUNIT_ASSERT_EQUAL((size_t)255, bt.countFilteredBlock());
  bt.disableFilter();
  CPPUNIT_ASSERT_EQUAL((size_t)256, bt.countBlock());
  CPPUNIT_ASSERT_EQUAL((size_t)0, bt.countFilteredBlock());
}

void BitfieldManTest::testCountMissingBlock()
{
  BitfieldMan bt(1024, 1024*10);
  CPPUNIT_ASSERT_EQUAL((size_t)10, bt.countMissingBlock());
  bt.setBit(1);
  CPPUNIT_ASSERT_EQUAL((size_t)9, bt.countMissingBlock());
  bt.setAllBit();
  CPPUNIT_ASSERT_EQUAL((size_t)0, bt.countMissingBlock());
}

void BitfieldManTest::testZeroLengthFilter()
{
  BitfieldMan bt(1024, 1024*10);
  bt.enableFilter();
  CPPUNIT_ASSERT_EQUAL((size_t)0, bt.countMissingBlock());
}

void BitfieldManTest::testGetFirstNMissingUnusedIndex()
{
  BitfieldMan bt(1024, 1024*10);
  bt.setUseBit(1);
  bt.setBit(5);
  std::vector<size_t> out;
  CPPUNIT_ASSERT_EQUAL((size_t)8, bt.getFirstNMissingUnusedIndex(out, 256));
  CPPUNIT_ASSERT_EQUAL((size_t)8, out.size());
  const size_t ans[] = {0, 2, 3, 4, 6, 7, 8, 9};
  for(size_t i = 0; i < out.size(); ++i) {
    CPPUNIT_ASSERT_EQUAL(ans[i], out[i]);
  }
  out.clear();
  CPPUNIT_ASSERT_EQUAL((size_t)3, bt.getFirstNMissingUnusedIndex(out, 3));
  CPPUNIT_ASSERT_EQUAL((size_t)3, out.size());
  for(size_t i = 0; i < out.size(); ++i) {
    CPPUNIT_ASSERT_EQUAL(ans[i], out[i]);
  }  
  CPPUNIT_ASSERT_EQUAL((size_t)0, bt.getFirstNMissingUnusedIndex(out, 0));
  bt.setAllBit();
  CPPUNIT_ASSERT_EQUAL((size_t)0, bt.getFirstNMissingUnusedIndex(out, 10));
  bt.clearAllBit();
  out.clear();
  bt.addFilter(1024*9, 1024);
  bt.enableFilter();
  CPPUNIT_ASSERT_EQUAL((size_t)1, bt.getFirstNMissingUnusedIndex(out, 256));
  CPPUNIT_ASSERT_EQUAL((size_t)1, out.size());
  CPPUNIT_ASSERT_EQUAL((size_t)9, out[0]);
}

void BitfieldManTest::testGetInorderMissingUnusedIndex()
{
  BitfieldMan bt(1024, 1024*20);
  const size_t length = 3;
  unsigned char ignoreBitfield[length];
  memset(ignoreBitfield, 0, sizeof(ignoreBitfield));
  size_t minSplitSize = 1024;
  size_t index;
  // 00000|00000|00000|00000
  CPPUNIT_ASSERT
    (bt.getInorderMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)0, index);
  bt.setUseBit(0);
  // 10000|00000|00000|00000
  CPPUNIT_ASSERT
    (bt.getInorderMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)1, index);
  minSplitSize = 1024*2;
  CPPUNIT_ASSERT
    (bt.getInorderMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)2, index);
  bt.unsetUseBit(0);
  bt.setBit(0);
  CPPUNIT_ASSERT
    (bt.getInorderMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)1, index);
  bt.setAllBit();
  bt.unsetBit(10);
  // 11111|11111|01111|11111
  CPPUNIT_ASSERT
    (bt.getInorderMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)10, index);
  bt.setUseBit(10);
  CPPUNIT_ASSERT
    (!bt.getInorderMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length));
  bt.unsetUseBit(10);
  bt.setAllBit();
  // 11111|11111|11111|11111
  CPPUNIT_ASSERT
    (!bt.getInorderMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length));
  bt.clearAllBit();
  // 00000|00000|00000|00000
  for(int i = 0; i <= 1; ++i) {
    bitfield::flipBit(ignoreBitfield, length, i);
  }
  CPPUNIT_ASSERT
    (bt.getInorderMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)2, index);
  bt.addFilter(1024*3, 1024*3);
  bt.enableFilter();
  CPPUNIT_ASSERT
    (bt.getInorderMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length));
  CPPUNIT_ASSERT_EQUAL((size_t)3, index);
}

void BitfieldManTest::testGetGeomMissingUnusedIndex()
{
  BitfieldMan bt(1024, 1024*20);
  const size_t length = 3;
  unsigned char ignoreBitfield[length];
  memset(ignoreBitfield, 0, sizeof(ignoreBitfield));
  size_t minSplitSize = 1024;
  size_t index;
  // 00000|00000|00000|00000
  CPPUNIT_ASSERT
    (bt.getGeomMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length, 2, 0));
  CPPUNIT_ASSERT_EQUAL((size_t)0, index);
  bt.setUseBit(0);
  // 10000|00000|00000|00000
  CPPUNIT_ASSERT
    (bt.getGeomMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length, 2, 0));
  CPPUNIT_ASSERT_EQUAL((size_t)1, index);
  bt.setUseBit(1);
  // 11000|00000|00000|00000
  CPPUNIT_ASSERT
    (bt.getGeomMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length, 2, 0));
  CPPUNIT_ASSERT_EQUAL((size_t)2, index);
  bt.setUseBit(2);
  // 11100|00000|00000|00000
  CPPUNIT_ASSERT
    (bt.getGeomMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length, 2, 0));
  CPPUNIT_ASSERT_EQUAL((size_t)4, index);
  bt.setUseBit(4);
  // 11110|00000|00000|00000
  CPPUNIT_ASSERT
    (bt.getGeomMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length, 2, 0));
  CPPUNIT_ASSERT_EQUAL((size_t)8, index);
  bt.setUseBit(8);
  // 11110|00010|00000|00000
  CPPUNIT_ASSERT
    (bt.getGeomMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length, 2, 0));
  CPPUNIT_ASSERT_EQUAL((size_t)16, index);
  bt.setUseBit(16);
  // 11110|00010|00000|01000
  CPPUNIT_ASSERT
    (bt.getGeomMissingUnusedIndex
     (index, minSplitSize, ignoreBitfield, length, 2, 0));
  CPPUNIT_ASSERT_EQUAL((size_t)12, index);
  bt.setUseBit(12);
}

} // namespace aria2
