#include "TimeA2.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"

namespace aria2 {

class TimeTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TimeTest);
  CPPUNIT_TEST(testParseRFC1123);
  CPPUNIT_TEST(testParseRFC850);
  CPPUNIT_TEST(testParseRFC850Ext);
  CPPUNIT_TEST(testParseAsctime);
  CPPUNIT_TEST(testParseHTTPDate);
  CPPUNIT_TEST(testOperatorLess);
  CPPUNIT_TEST(testToHTTPDate);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testParseRFC1123();
  void testParseRFC1123Alt();
  void testParseRFC850();
  void testParseRFC850Ext();
  void testParseAsctime();
  void testParseHTTPDate();
  void testOperatorLess();
  void testToHTTPDate();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TimeTest);

void TimeTest::testParseRFC1123()
{
  Time t1 = Time::parseRFC1123("Sat, 06 Sep 2008 15:26:33 GMT");
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793, t1.getTimeFromEpoch());
}

void TimeTest::testParseRFC1123Alt()
{
  Time t1 = Time::parseRFC1123Alt("Sat, 06 Sep 2008 15:26:33 +0000");
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793, t1.getTimeFromEpoch());
}

void TimeTest::testParseRFC850()
{
  Time t1 = Time::parseRFC850("Saturday, 06-Sep-08 15:26:33 GMT");
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793, t1.getTimeFromEpoch());
}

void TimeTest::testParseRFC850Ext()
{
  Time t1 = Time::parseRFC850Ext("Saturday, 06-Sep-2008 15:26:33 GMT");
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793, t1.getTimeFromEpoch());
}

void TimeTest::testParseAsctime()
{
  Time t1 = Time::parseAsctime("Sun Sep  6 15:26:33 2008");
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793, t1.getTimeFromEpoch());
}

void TimeTest::testParseHTTPDate()
{
  CPPUNIT_ASSERT_EQUAL(
      (time_t)1220714793,
      Time::parseHTTPDate("Sat, 06 Sep 2008 15:26:33 GMT").getTimeFromEpoch());
  CPPUNIT_ASSERT_EQUAL(
      (time_t)1220714793,
      Time::parseHTTPDate("Sat, 06-Sep-2008 15:26:33 GMT").getTimeFromEpoch());
  CPPUNIT_ASSERT_EQUAL(
      (time_t)1220714793,
      Time::parseHTTPDate("Sat, 06-Sep-08 15:26:33 GMT").getTimeFromEpoch());
  CPPUNIT_ASSERT_EQUAL(
      (time_t)1220714793,
      Time::parseHTTPDate("Sun Sep  6 15:26:33 2008").getTimeFromEpoch());
  CPPUNIT_ASSERT(Time::parseHTTPDate("Sat, 2008-09-06 15:26:33 GMT").bad());
}

void TimeTest::testOperatorLess()
{
  CPPUNIT_ASSERT(Time(1) < Time(2));
  CPPUNIT_ASSERT(!(Time(1) < Time(1)));
  CPPUNIT_ASSERT(!(Time(2) < Time(1)));
}

void TimeTest::testToHTTPDate()
{
// This test disabled for MinGW32, because the garbage will be
// displayed and it hides real errors.
#ifndef __MINGW32__
  Time t(1220714793);
  CPPUNIT_ASSERT_EQUAL(std::string("Sat, 06 Sep 2008 15:26:33 GMT"),
                       t.toHTTPDate());
#endif // !__MINGW32__
}

} // namespace aria2
