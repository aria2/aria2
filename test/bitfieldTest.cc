#include "bitfield.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class bitfieldTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(bitfieldTest);
  CPPUNIT_TEST(testTest);
  CPPUNIT_TEST(testCountBit32);
  CPPUNIT_TEST(testCountSetBit);
  CPPUNIT_TEST(testLastByteMask);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void testTest();
  void testCountBit32();
  void testCountSetBit();
  void testLastByteMask();
};


CPPUNIT_TEST_SUITE_REGISTRATION( bitfieldTest );

void bitfieldTest::testTest()
{
  unsigned char bitfield[] = { 0xaa };

  CPPUNIT_ASSERT(bitfield::test(bitfield, 8, 0));
  CPPUNIT_ASSERT(!bitfield::test(bitfield, 8, 1));
}

void bitfieldTest::testCountBit32()
{
  CPPUNIT_ASSERT_EQUAL((size_t)32, bitfield::countBit32(UINT32_MAX));
  CPPUNIT_ASSERT_EQUAL((size_t)8, bitfield::countBit32(255));
}

void bitfieldTest::testCountSetBit()
{
  unsigned char bitfield[] = { 0xff, 0xff, 0xff, 0xf0, 0xff, 0x01 };

  CPPUNIT_ASSERT_EQUAL((size_t)37, bitfield::countSetBit(bitfield, 48));
  CPPUNIT_ASSERT_EQUAL((size_t)36, bitfield::countSetBit(bitfield, 47));
  CPPUNIT_ASSERT_EQUAL((size_t)28, bitfield::countSetBit(bitfield, 32));
}

void bitfieldTest::testLastByteMask()
{
  CPPUNIT_ASSERT_EQUAL((unsigned int)128,
		       (unsigned int)bitfield::lastByteMask(9));
  CPPUNIT_ASSERT_EQUAL((unsigned int)240,
		       (unsigned int)bitfield::lastByteMask(12));
  CPPUNIT_ASSERT_EQUAL((unsigned int)255,
		       (unsigned int)bitfield::lastByteMask(16));
}

} // namespace aria2
