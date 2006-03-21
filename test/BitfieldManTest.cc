#include "BitfieldMan.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BitfieldManTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BitfieldManTest);
  CPPUNIT_TEST(testGetBlockSize);
  CPPUNIT_TEST(testGetFirstMissingUnusedIndex);
  CPPUNIT_TEST(testIsAllBitSet);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testGetBlockSize();
  void testGetFirstMissingUnusedIndex();
  void testIsAllBitSet();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BitfieldManTest );

void BitfieldManTest::testGetBlockSize() {
  BitfieldMan bt1(1024, 1024*10);
  CPPUNIT_ASSERT_EQUAL(1024, bt1.getBlockSize(9));

  BitfieldMan bt2(1024, 1024*10+1);
  CPPUNIT_ASSERT_EQUAL(1024, bt2.getBlockSize(9));
  CPPUNIT_ASSERT_EQUAL(1, bt2.getBlockSize(10));
  CPPUNIT_ASSERT_EQUAL(0, bt2.getBlockSize(11));
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

  for(int i = 0; i < bt1.getBlocks(); i++) {
    CPPUNIT_ASSERT(bt1.setBit(i));
  }
  CPPUNIT_ASSERT(bt1.isAllBitSet());
}
