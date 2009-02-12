#include "array_fun.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class array_funTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(array_funTest);
  CPPUNIT_TEST(testBit_negate);
  CPPUNIT_TEST(testBit_and);
  CPPUNIT_TEST(testArray_negate);
  CPPUNIT_TEST(testArray_and);
  CPPUNIT_TEST(testArrayLength);
  CPPUNIT_TEST(testArrayPtr);
  CPPUNIT_TEST_SUITE_END();

public:
  void testBit_negate();
  void testBit_and();
  void testArray_negate();
  void testArray_and();
  void testArrayLength();
  void testArrayPtr();

  struct X{
    int m;
  };

};


CPPUNIT_TEST_SUITE_REGISTRATION(array_funTest);

void array_funTest::testBit_negate()
{
  unsigned char b = 0xaa;
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x55, bit_negate<unsigned char>()(b));
}

void array_funTest::testBit_and()
{
  unsigned char b = 0xaa;
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x0a, bit_and<unsigned char>()(b, 0x0a));
}

void array_funTest::testArray_negate()
{
  unsigned char a[] = { 0xaa, 0x55 };
  array_fun<unsigned char> f = array_negate((unsigned char*)a);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x55, f[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0xaa, f[1]);

  array_fun<unsigned char> ff = array_negate(f);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0xaa, ff[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x55, ff[1]);
}

void array_funTest::testArray_and()
{
  unsigned char a1[] = { 0xaa, 0x55 };
  unsigned char a2[] = { 0x1a, 0x25 };
  array_fun<unsigned char> f = array_and((unsigned char*)a1, (unsigned char*)a2);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x0a, f[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x05, f[1]);

  array_fun<unsigned char> f2 = array_and((unsigned char*)a1, array_negate(a2));
  CPPUNIT_ASSERT_EQUAL((unsigned char)0xa0, f2[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x50, f2[1]);

  array_fun<unsigned char> f3 = array_and(array_negate(a2), (unsigned char*)a1);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0xa0, f3[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x50, f3[1]);

  array_fun<unsigned char> f4 = array_and(array_negate(a1), array_negate(a2));
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x45, f4[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x8a, f4[1]);
}

void array_funTest::testArrayLength()
{
  int64_t ia[] = { 1, 2, 3, 4, 5 };
  int64_t zeroLengthArray[] = {};

  CPPUNIT_ASSERT_EQUAL((size_t)5, arrayLength(ia));
  CPPUNIT_ASSERT_EQUAL((size_t)0, arrayLength(zeroLengthArray));
}

// Check operator[] in const context.
static void arrayPtrConst(const array_ptr<struct array_funTest::X>& ax)
{
  CPPUNIT_ASSERT_EQUAL(100, ax[3].m);
  CPPUNIT_ASSERT_EQUAL(99, ax[2].m);
}

static void arrayPtrCast(struct array_funTest::X* x) {}

static void arrayPtrConstCast(const struct array_funTest::X* x) {}

void array_funTest::testArrayPtr()
{
  array_ptr<struct X> ax(new struct X[10]);
  ax[3].m = 100;
  ax[2].m = 99;
  CPPUNIT_ASSERT_EQUAL(100, ax[3].m);
  CPPUNIT_ASSERT_EQUAL(99, ax[2].m);
  arrayPtrConst(ax);

  arrayPtrCast(ax);
  arrayPtrConstCast(ax);
}

} // namespace aria2
