#include "Triplet.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class TripletTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TripletTest);
  CPPUNIT_TEST(testLess);
  CPPUNIT_TEST(testTupleGet);
  CPPUNIT_TEST(testTupleNthType);
  CPPUNIT_TEST(testTuple2Pair);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testLess();
  void testTupleGet();
  void testTupleNthType();
  void testTuple2Pair();
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

void TripletTest::testTupleGet()
{
  Triplet<int, double, std::string> x(1, 3.14, "foo");
  CPPUNIT_ASSERT_EQUAL(1, (TupleGet<1>::get(x)));
  CPPUNIT_ASSERT_EQUAL((double)3.14, (TupleGet<2>::get(x)));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), (TupleGet<3>::get(x)));
}

void TripletTest::testTupleNthType()
{
  TupleNthType<Triplet<int, double, std::string>, 1>::type x = 1;
  CPPUNIT_ASSERT_EQUAL(1, x);
  TupleNthType<Triplet<int, double, std::string>, 2>::type y = 3.14;
  CPPUNIT_ASSERT_EQUAL((double)3.14, y);
  TupleNthType<Triplet<int, double, std::string>, 3>::type z = "foo";
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), z);
}

void TripletTest::testTuple2Pair()
{
  Triplet<int, double, std::string> x(1, 3.14, "foo");
  std::pair<int, double> p1 = Tuple2Pair<1, 2>()(x);
  CPPUNIT_ASSERT_EQUAL(1, p1.first);
  CPPUNIT_ASSERT_EQUAL((double)3.14, p1.second);

  std::pair<double, std::string> p2 = Tuple2Pair<2, 3>()(x);
  CPPUNIT_ASSERT_EQUAL((double)3.14, p2.first);
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), p2.second);
}

} // namespace aria2
