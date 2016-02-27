#include "array_fun.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace aria2::expr;

namespace aria2 {

class array_funTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(array_funTest);
  CPPUNIT_TEST(testArray_negate);
  CPPUNIT_TEST(testArray_and);
  CPPUNIT_TEST(testArrayLength);
  CPPUNIT_TEST(testArrayWrapper);
  CPPUNIT_TEST_SUITE_END();

public:
  void testBit_negate();
  void testBit_and();
  void testArray_negate();
  void testArray_and();
  void testArrayLength();
  void testArrayWrapper();

  struct X {
    int m;
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(array_funTest);

void array_funTest::testArray_negate()
{
  unsigned char a[] = {0xaa, 0x55};
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x55, (~array(a))[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0xaa, (~array((unsigned char*)a))[1]);

  CPPUNIT_ASSERT_EQUAL((unsigned char)0xaa, (~~array(a))[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x55, (~~array(a))[1]);
}

void array_funTest::testArray_and()
{
  unsigned char a1[] = {0xaa, 0x55};
  unsigned char a2[] = {0x1a, 0x25};
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x0a, (array(a1) & array(a2))[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x05, (array(a1) & array(a2))[1]);

  CPPUNIT_ASSERT_EQUAL((unsigned char)0xa0, (array(a1) & ~array(a2))[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x50, (array(a1) & ~array(a2))[1]);

  CPPUNIT_ASSERT_EQUAL((unsigned char)0xa0, (~array(a2) & array(a1))[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x50, (~array(a2) & array(a1))[1]);

  CPPUNIT_ASSERT_EQUAL((unsigned char)0x45, (~array(a1) & ~array(a2))[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0x8a, (~array(a1) & ~array(a2))[1]);
}

void array_funTest::testArrayLength()
{
  int64_t ia[] = {1, 2, 3, 4, 5};
  CPPUNIT_ASSERT_EQUAL((size_t)5, arraySize(ia));
  // This causes compile error under clang and gcc v3.4.3 opensolaris
  // 5.11
  // int64_t zeroLengthArray[] = {};
  // CPPUNIT_ASSERT_EQUAL((size_t)0, arraySize(zeroLengthArray));
}

namespace {
void arrayPtrCast(struct array_funTest::X* x) {}
} // namespace

namespace {
void arrayPtrConstCast(const struct array_funTest::X* x) {}
} // namespace

namespace {
void arrayWrapperConst(const array_wrapper<int, 10>& array)
{
  CPPUNIT_ASSERT_EQUAL(9, array[9]);
}
} // namespace

void array_funTest::testArrayWrapper()
{
  array_wrapper<int, 10> a1;
  CPPUNIT_ASSERT_EQUAL((size_t)10, a1.size());
  for (size_t i = 0; i < a1.size(); ++i) {
    a1[i] = i;
  }
  CPPUNIT_ASSERT_EQUAL(9, a1[9]);
  array_wrapper<int, 10> a2 = a1;
  CPPUNIT_ASSERT_EQUAL(9, a2[9]);

  arrayWrapperConst(a2);

  array_wrapper<struct X, 10> x1;
  arrayPtrCast(x1);
  arrayPtrConstCast(x1);
}

} // namespace aria2
