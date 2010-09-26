#include "Triplet.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class TripletTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TripletTest);
  CPPUNIT_TEST(testLess);
  CPPUNIT_TEST(testTripletGet);
  CPPUNIT_TEST(testTripletNthType);
  CPPUNIT_TEST(testTriplet2Pair);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testLess();
  void testTripletGet();
  void testTripletNthType();
  void testTriplet2Pair();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TripletTest);

void TripletTest::testLess()
{
  Triplet<int, int, int> tri1(0, 1, 1);
  Triplet<int, int, int> tri2(1, 0, 0);
  CPPUNIT_ASSERT(!(tri1 < tri1));
  CPPUNIT_ASSERT(tri1 < tri2);
  CPPUNIT_ASSERT(!(tri2 < tri1));

  Triplet<int, int, int> tri3(0, 0, 1);
  Triplet<int, int, int> tri4(0, 1, 0);
  CPPUNIT_ASSERT(tri3 < tri4);
  CPPUNIT_ASSERT(!(tri4 < tri3));

  Triplet<int, int, int> tri5(0, 0, 0);
  Triplet<int, int, int> tri6(0, 0, 1);
  CPPUNIT_ASSERT(tri5 < tri6);
  CPPUNIT_ASSERT(!(tri6 < tri5));
}

void TripletTest::testTripletGet()
{
  Triplet<int, double, std::string> x(1, 3.14, "foo");
  CPPUNIT_ASSERT_EQUAL(1, (TripletGet<1>::get(x)));
  CPPUNIT_ASSERT_EQUAL((double)3.14, (TripletGet<2>::get(x)));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), (TripletGet<3>::get(x)));
}

void TripletTest::testTripletNthType()
{
  TripletNthType<Triplet<int, double, std::string>, 1>::type x = 1;
  CPPUNIT_ASSERT_EQUAL(1, x);
  TripletNthType<Triplet<int, double, std::string>, 2>::type y = 3.14;
  CPPUNIT_ASSERT_EQUAL((double)3.14, y);
  TripletNthType<Triplet<int, double, std::string>, 3>::type z = "foo";
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), z);
}

void TripletTest::testTriplet2Pair()
{
  Triplet<int, double, std::string> x(1, 3.14, "foo");
  std::pair<int, double> p1 = Triplet2Pair<1, 2>()(x);
  CPPUNIT_ASSERT_EQUAL(1, p1.first);
  CPPUNIT_ASSERT_EQUAL((double)3.14, p1.second);

  std::pair<double, std::string> p2 = Triplet2Pair<2, 3>()(x);
  CPPUNIT_ASSERT_EQUAL((double)3.14, p2.first);
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), p2.second);
}

} // namespace aria2
