#include "TimeA2.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"

namespace aria2 {

class TimeTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TimeTest);
  CPPUNIT_TEST(testParseRFC1123);
  CPPUNIT_TEST(testParseRFC850);
  CPPUNIT_TEST(testParseRFC850Ext);
  CPPUNIT_TEST(testParseAsctime);
  CPPUNIT_TEST(testParseHTTPDate);
  CPPUNIT_TEST(testOperatorLess);
  CPPUNIT_TEST(testElapsed);
  CPPUNIT_TEST(testToHTTPDate);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testParseRFC1123();
  void testParseRFC850();
  void testParseRFC850Ext();
  void testParseAsctime();
  void testParseHTTPDate();
  void testOperatorLess();
  void testElapsed();
  void testToHTTPDate();
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

void TimeTest::testParseAsctime()
{
  Time t1 = Time::parseAsctime("Sun Sep  6 15:26:33 2008");
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
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793,
                       Time::parseHTTPDate
                       ("Sun Sep  6 15:26:33 2008").getTime());
  CPPUNIT_ASSERT(Time::parseHTTPDate
                 ("Sat, 2008-09-06 15:26:33 GMT").bad());
}

void TimeTest::testOperatorLess()
{
  CPPUNIT_ASSERT(Time(1) < Time(2));
  CPPUNIT_ASSERT(!(Time(1) < Time(1)));
  CPPUNIT_ASSERT(!(Time(2) < Time(1)));

  struct timeval tv1;
  tv1.tv_sec = 0;
  tv1.tv_usec = 1;
  struct timeval tv2;
  tv2.tv_sec = 1;
  tv2.tv_usec = 0;
  CPPUNIT_ASSERT(Time(tv1) < Time(tv2));

  tv2.tv_sec = 0;
  CPPUNIT_ASSERT(Time(tv2) < Time(tv1));
}

void TimeTest::testToHTTPDate()
{
  Time t(1220714793);
  CPPUNIT_ASSERT_EQUAL(std::string("Sat, 06 Sep 2008 15:26:33 GMT"),
                       t.toHTTPDate());
}

void TimeTest::testElapsed()
{
  struct timeval now;
  gettimeofday(&now, 0);
  {
    struct timeval tv = now;
    CPPUNIT_ASSERT(!Time(tv).elapsed(1));
  }
  {
    struct timeval tv;
    suseconds_t usec = now.tv_usec+500000;
    if(usec > 999999) {
      tv.tv_sec = now.tv_sec+usec/1000000;
      tv.tv_usec = usec%1000000;
    } else {
      tv.tv_sec = now.tv_sec;
      tv.tv_usec = usec;
    }
    CPPUNIT_ASSERT(!Time(tv).elapsed(1));
  }
  {
    struct timeval tv = { now.tv_sec-1, now.tv_usec };
    CPPUNIT_ASSERT(Time(tv).elapsed(1));
  }
}

} // namespace aria2
