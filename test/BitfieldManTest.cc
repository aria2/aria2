#include "BitfieldMan.h"
#include "FixedNumberRandomizer.h"
#include "BitfieldManFactory.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BitfieldManTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BitfieldManTest);
  CPPUNIT_TEST(testGetBlockSize);
  CPPUNIT_TEST(testGetFirstMissingUnusedIndex);
  CPPUNIT_TEST(testIsAllBitSet);
  CPPUNIT_TEST(testFilter);
  CPPUNIT_TEST(testGetMissingIndex);
  CPPUNIT_TEST(testGetSparceMissingUnusedIndex);
  CPPUNIT_TEST(testIsBitSetOffsetRange);
  CPPUNIT_TEST(testGetMissingUnusedLength);
  CPPUNIT_TEST(testSetBitRange);
  CPPUNIT_TEST_SUITE_END();
private:
  RandomizerHandle fixedNumberRandomizer;

public:
  BitfieldManTest():fixedNumberRandomizer(0) {
    FixedNumberRandomizer* randomizer = new FixedNumberRandomizer();
    randomizer->setFixedNumber(0);
    this->fixedNumberRandomizer = randomizer;
  }

  void setUp() {
  }

  void testGetBlockSize();
  void testGetFirstMissingUnusedIndex();
  void testIsAllBitSet();
  void testFilter();
  void testGetMissingIndex();
  void testGetSparceMissingUnusedIndex();
  void testIsBitSetOffsetRange();
  void testGetMissingUnusedLength();
  void testSetBitRange();
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

void BitfieldManTest::testGetFirstMissingUnusedIndex() {
  BitfieldMan bt1(1024, 1024*10);
  unsigned char bitfield[2];
  memset(bitfield, 0xff, sizeof(bitfield));

  CPPUNIT_ASSERT_EQUAL(0, bt1.getFirstMissingUnusedIndex(bitfield, sizeof(bitfield)));
  CPPUNIT_ASSERT(bt1.setUseBit(0));
  CPPUNIT_ASSERT_EQUAL(1, bt1.getFirstMissingUnusedIndex(bitfield, sizeof(bitfield)));
  CPPUNIT_ASSERT(bt1.unsetUseBit(0));
  CPPUNIT_ASSERT_EQUAL(0, bt1.getFirstMissingUnusedIndex(bitfield, sizeof(bitfield)));
  CPPUNIT_ASSERT(bt1.setBit(0));
  CPPUNIT_ASSERT_EQUAL(1, bt1.getFirstMissingUnusedIndex(bitfield, sizeof(bitfield)));

  for(int i = 0; i < 8; i++) {
    CPPUNIT_ASSERT(bt1.setBit(i));
  }
  CPPUNIT_ASSERT_EQUAL(8, bt1.getFirstMissingUnusedIndex(bitfield, sizeof(bitfield)));

  CPPUNIT_ASSERT_EQUAL(8, bt1.getFirstMissingUnusedIndex());
  CPPUNIT_ASSERT(bt1.setUseBit(8));
  CPPUNIT_ASSERT_EQUAL(9, bt1.getFirstMissingUnusedIndex());
}

void BitfieldManTest::testIsAllBitSet() {
  BitfieldMan bt1(1024, 1024*10);
  CPPUNIT_ASSERT(!bt1.isAllBitSet());
  bt1.setBit(1);
  CPPUNIT_ASSERT(!bt1.isAllBitSet());
  
  for(int i = 0; i < 8; i++) {
    CPPUNIT_ASSERT(bt1.setBit(i));
  }
  CPPUNIT_ASSERT(!bt1.isAllBitSet());

  for(int32_t i = 0; i < bt1.countBlock(); i++) {
    CPPUNIT_ASSERT(bt1.setBit(i));
  }
  CPPUNIT_ASSERT(bt1.isAllBitSet());
}

void BitfieldManTest::testFilter() {
  BitfieldMan btman(2, 32);
  btman.setRandomizer(fixedNumberRandomizer);

  // test offset=4, length=12
  btman.addFilter(4, 12);
  btman.enableFilter();
  unsigned char peerBt[2];
  memset(peerBt, 0xff, sizeof(peerBt));

  int index;
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(2, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(3, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(4, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(5, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(6, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(7, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(-1, index);
  CPPUNIT_ASSERT_EQUAL((int64_t)12, btman.getFilteredTotalLength());

  // test offset=5, length=2
  btman.clearAllBit();
  btman.clearAllUseBit();
  btman.clearFilter();
  btman.addFilter(5, 2);
  btman.enableFilter();
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  btman.setBit(index);
  CPPUNIT_ASSERT_EQUAL(2, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  btman.setBit(index);
  CPPUNIT_ASSERT_EQUAL(3, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(-1, index);
  CPPUNIT_ASSERT_EQUAL((int64_t)4, btman.getFilteredTotalLength());
  CPPUNIT_ASSERT(btman.isFilteredAllBitSet());

  BitfieldMan btman2(2, 31);
  btman2.addFilter(0, 31);
  btman2.enableFilter();
  CPPUNIT_ASSERT_EQUAL((int64_t)31, btman2.getFilteredTotalLength());

}

void BitfieldManTest::testGetMissingIndex() {
  BitfieldMan bt1(1024, 1024*256);
  bt1.setRandomizer(fixedNumberRandomizer);

  unsigned char bitArray[] = {
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
  };
  CPPUNIT_ASSERT_EQUAL(0, bt1.getMissingIndex(bitArray, 32));

  unsigned char bitArray2[] = {
    0x0f, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
  };

  CPPUNIT_ASSERT_EQUAL(4, bt1.getMissingIndex(bitArray2, 32));

  unsigned char bitArray3[] = {
    0x00, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
  };

  CPPUNIT_ASSERT_EQUAL(8, bt1.getMissingIndex(bitArray3, 32));

  unsigned char bitArray4[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };

  CPPUNIT_ASSERT_EQUAL(-1, bt1.getMissingIndex(bitArray4, 32));

}

void BitfieldManTest::testGetSparceMissingUnusedIndex() {
  BitfieldMan bitfield(1024*1024, 10*1024*1024);

  CPPUNIT_ASSERT_EQUAL(0, bitfield.getSparseMissingUnusedIndex());
  bitfield.setBit(0);
  CPPUNIT_ASSERT_EQUAL(5, bitfield.getSparseMissingUnusedIndex());
  bitfield.setUseBit(5);
  CPPUNIT_ASSERT_EQUAL(3, bitfield.getSparseMissingUnusedIndex());
  bitfield.setBit(3);
  CPPUNIT_ASSERT_EQUAL(8, bitfield.getSparseMissingUnusedIndex());
  bitfield.setBit(8);
  CPPUNIT_ASSERT_EQUAL(2, bitfield.getSparseMissingUnusedIndex());
  bitfield.setBit(2);
  CPPUNIT_ASSERT_EQUAL(7, bitfield.getSparseMissingUnusedIndex());
  bitfield.setBit(7);
  CPPUNIT_ASSERT_EQUAL(1, bitfield.getSparseMissingUnusedIndex());
  bitfield.setBit(1);
  CPPUNIT_ASSERT_EQUAL(4, bitfield.getSparseMissingUnusedIndex());
  bitfield.setBit(4);
  CPPUNIT_ASSERT_EQUAL(6, bitfield.getSparseMissingUnusedIndex());
  bitfield.setBit(6);
  CPPUNIT_ASSERT_EQUAL(9, bitfield.getSparseMissingUnusedIndex());
  bitfield.setBit(9);
  CPPUNIT_ASSERT_EQUAL(-1, bitfield.getSparseMissingUnusedIndex());
}

void BitfieldManTest::testIsBitSetOffsetRange()
{
  int64_t totalLength = (int64_t)4*1024*1024*1024;
  int32_t pieceLength = 4*1024*1024;
  BitfieldMan bitfield(pieceLength, totalLength);
  bitfield.setAllBit();

  CPPUNIT_ASSERT(!bitfield.isBitSetOffsetRange(0, 0));
  CPPUNIT_ASSERT(!bitfield.isBitSetOffsetRange(0, -1));
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

void BitfieldManTest::testGetMissingUnusedLength()
{
  int64_t totalLength = 1024*10+10;
  int32_t blockLength = 1024;

  BitfieldMan bf(blockLength, totalLength);

  // from index 0 and all blocks are unused and not acquired.
  CPPUNIT_ASSERT_EQUAL((int64_t)totalLength, bf.getMissingUnusedLength(0));

  // from index 10 and all blocks are unused and not acquired.
  CPPUNIT_ASSERT_EQUAL((int64_t)10, bf.getMissingUnusedLength(10));

  // from index -1
  CPPUNIT_ASSERT_EQUAL((int64_t)0, bf.getMissingUnusedLength(-1));

  // from index 11
  CPPUNIT_ASSERT_EQUAL((int64_t)0, bf.getMissingUnusedLength(11));

  // from index 12
  CPPUNIT_ASSERT_EQUAL((int64_t)0, bf.getMissingUnusedLength(12));

  // from index 0 and 5th block is used.
  bf.setUseBit(5);
  CPPUNIT_ASSERT_EQUAL((int64_t)5*blockLength, bf.getMissingUnusedLength(0));

  // from index 0 and 4th block is acquired.
  bf.setBit(4);
  CPPUNIT_ASSERT_EQUAL((int64_t)4*blockLength, bf.getMissingUnusedLength(0));

  // from index 1
  CPPUNIT_ASSERT_EQUAL((int64_t)3*blockLength, bf.getMissingUnusedLength(1));
}

void BitfieldManTest::testSetBitRange()
{
  int32_t blockLength = 1024*1024;
  int64_t totalLength = 10*blockLength;

  BitfieldMan bf(blockLength, totalLength);

  bf.setBitRange(0, 4);

  for(int32_t i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(bf.isBitSet(i));
  }
  for(int32_t i = 5; i < 10; ++i) {
    CPPUNIT_ASSERT(!bf.isBitSet(i));
  }
  CPPUNIT_ASSERT_EQUAL(int64_t(5*blockLength), bf.getCompletedLength());
}
