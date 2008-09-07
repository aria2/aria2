#include "TimeA2.h"
#include "Exception.h"
#include "Util.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class TimeTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TimeTest);
  CPPUNIT_TEST(testParseRFC1123);
  CPPUNIT_TEST(testParseRFC850);
  CPPUNIT_TEST(testParseRFC850Ext);
  CPPUNIT_TEST(testParseHTTPDate);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testParseRFC1123();
  void testParseRFC850();
  void testParseRFC850Ext();
  void testParseHTTPDate();
};


CPPUNIT_TEST_SUITE_REGISTRATION(TimeTest);

void TimeTest::testParseRFC1123()
{
  Time t1 = Time::parseRFC1123("Sat, 06 Sep 2008 15:26:33 GMT");
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793, t1.getTime());
}

void TimeTest::testParseRFC850()
{
  Time t1 = Time::parseRFC850("Saturday, 06-Sep-08 15:26:33 GMT");
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793, t1.getTime());
}

void TimeTest::testParseRFC850Ext()
{
  Time t1 = Time::parseRFC850Ext("Saturday, 06-Sep-2008 15:26:33 GMT");
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793, t1.getTime());
}

void TimeTest::testParseHTTPDate()
{
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793,
		       Time::parseHTTPDate
		       ("Sat, 06 Sep 2008 15:26:33 GMT").getTime());
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793,
		       Time::parseHTTPDate
		       ("Sat, 06-Sep-2008 15:26:33 GMT").getTime());
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793,
		       Time::parseHTTPDate
		       ("Sat, 06-Sep-08 15:26:33 GMT").getTime());
  CPPUNIT_ASSERT_EQUAL((time_t)-1,
		       Time::parseHTTPDate
		       ("Sat, 2008-09-06 15:26:33 GMT").getTime());
}

} // namespace aria2
