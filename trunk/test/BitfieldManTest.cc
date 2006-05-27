#include "BitfieldMan.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BitfieldManTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BitfieldManTest);
  CPPUNIT_TEST(testGetBlockSize);
  CPPUNIT_TEST(testGetFirstMissingUnusedIndex);
  CPPUNIT_TEST(testIsAllBitSet);
  CPPUNIT_TEST(testFilter);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testGetBlockSize();
  void testGetFirstMissingUnusedIndex();
  void testIsAllBitSet();
  void testFilter();
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

  for(int i = 0; i < bt1.countBlock(); i++) {
    CPPUNIT_ASSERT(bt1.setBit(i));
  }
  CPPUNIT_ASSERT(bt1.isAllBitSet());
}

void BitfieldManTest::testFilter() {
  // set random seed here in order to get same random numbers.
  srandom(100);
  BitfieldMan btman(2, 32);

  // test offset=4, length=12
  btman.addFilter(4, 12);
  btman.enableFilter();
  unsigned char peerBt[2];
  memset(peerBt, 0xff, sizeof(peerBt));

  int index;
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(3, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(4, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(2, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(6, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(5, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(7, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(-1, index);
  CPPUNIT_ASSERT_EQUAL((long long int)12, btman.getFilteredTotalLength());

  // test offset=5, length=2
  btman.clearAllBit();
  btman.clearFilter();
  btman.addFilter(5, 2);
  btman.enableFilter();
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  btman.setBit(index);
  CPPUNIT_ASSERT_EQUAL(3, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  btman.setBit(index);
  CPPUNIT_ASSERT_EQUAL(2, index);
  index = btman.getMissingUnusedIndex(peerBt, sizeof(peerBt));
  btman.setUseBit(index);
  CPPUNIT_ASSERT_EQUAL(-1, index);
  CPPUNIT_ASSERT_EQUAL((long long int)4, btman.getFilteredTotalLength());
  CPPUNIT_ASSERT(btman.isAllBitSet());

  BitfieldMan btman2(2, 31);
  btman2.addFilter(0, 31);
  btman2.enableFilter();
  CPPUNIT_ASSERT_EQUAL((long long int)31, btman2.getFilteredTotalLength());
}
