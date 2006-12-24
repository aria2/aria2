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
};


CPPUNIT_TEST_SUITE_REGISTRATION( BitfieldManTest );

void BitfieldManTest::testGetBlockSize() {
  BitfieldMan bt1(1024, 1024*10);
  CPPUNIT_ASSERT_EQUAL((uint32_t)1024, bt1.getBlockLength(9));

  BitfieldMan bt2(1024, 1024*10+1);
  CPPUNIT_ASSERT_EQUAL((uint32_t)1024, bt2.getBlockLength(9));
  CPPUNIT_ASSERT_EQUAL((uint32_t)1, bt2.getBlockLength(10));
  CPPUNIT_ASSERT_EQUAL((uint32_t)0, bt2.getBlockLength(11));
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

  for(uint32_t i = 0; i < bt1.countBlock(); i++) {
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
  CPPUNIT_ASSERT_EQUAL((uint64_t)12, btman.getFilteredTotalLength());

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
  CPPUNIT_ASSERT_EQUAL((uint64_t)4, btman.getFilteredTotalLength());
  CPPUNIT_ASSERT(btman.isAllBitSet());

  BitfieldMan btman2(2, 31);
  btman2.addFilter(0, 31);
  btman2.enableFilter();
  CPPUNIT_ASSERT_EQUAL((uint64_t)31, btman2.getFilteredTotalLength());

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
