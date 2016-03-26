#include "SegList.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class SegListTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SegListTest);
  CPPUNIT_TEST(testNext);
  CPPUNIT_TEST(testPeek);
  CPPUNIT_TEST(testClear);
  CPPUNIT_TEST(testNormalize);
  CPPUNIT_TEST_SUITE_END();

public:
  void testNext();
  void testPeek();
  void testClear();
  void testNormalize();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SegListTest);

void SegListTest::testNext()
{
  SegList<int> sgl;
  sgl.add(-500, -498);
  sgl.add(5, 10);
  sgl.add(1, 5);
  for (int i = -500; i < -498; ++i) {
    CPPUNIT_ASSERT(sgl.hasNext());
    CPPUNIT_ASSERT_EQUAL(i, sgl.next());
  }
  for (int i = 5; i < 10; ++i) {
    CPPUNIT_ASSERT(sgl.hasNext());
    CPPUNIT_ASSERT_EQUAL(i, sgl.next());
  }
  for (int i = 1; i < 5; ++i) {
    CPPUNIT_ASSERT(sgl.hasNext());
    CPPUNIT_ASSERT_EQUAL(i, sgl.next());
  }
  CPPUNIT_ASSERT(!sgl.hasNext());
  CPPUNIT_ASSERT_EQUAL(0, sgl.next());
}

void SegListTest::testPeek()
{
  SegList<int> sgl;
  sgl.add(1, 3);
  sgl.add(4, 5);
  CPPUNIT_ASSERT_EQUAL(1, sgl.peek());
  CPPUNIT_ASSERT_EQUAL(1, sgl.peek());
  CPPUNIT_ASSERT_EQUAL(1, sgl.next());
  CPPUNIT_ASSERT_EQUAL(2, sgl.peek());
  CPPUNIT_ASSERT_EQUAL(2, sgl.next());
  CPPUNIT_ASSERT_EQUAL(4, sgl.peek());
  CPPUNIT_ASSERT_EQUAL(4, sgl.next());
  CPPUNIT_ASSERT(!sgl.hasNext());
}

void SegListTest::testClear()
{
  SegList<int> sgl;
  sgl.add(1, 3);
  CPPUNIT_ASSERT_EQUAL(1, sgl.next());
  sgl.clear();
  CPPUNIT_ASSERT(!sgl.hasNext());
  sgl.add(2, 3);
  CPPUNIT_ASSERT_EQUAL(2, sgl.next());
}

void SegListTest::testNormalize()
{
  SegList<int> sgl;
  sgl.add(10, 15);
  sgl.add(0, 1);
  sgl.add(1, 5);
  sgl.add(14, 16);
  sgl.add(2, 4);
  sgl.add(20, 21);
  sgl.normalize();
  for (int i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(sgl.hasNext());
    CPPUNIT_ASSERT_EQUAL(i, sgl.next());
  }
  for (int i = 10; i < 16; ++i) {
    CPPUNIT_ASSERT(sgl.hasNext());
    CPPUNIT_ASSERT_EQUAL(i, sgl.next());
  }
  CPPUNIT_ASSERT(sgl.hasNext());
  CPPUNIT_ASSERT_EQUAL(20, sgl.next());
  CPPUNIT_ASSERT(!sgl.hasNext());
}

} // namespace aria2
