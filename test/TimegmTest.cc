#include "timegm.h"

#include <cstring>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class TimegmTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TimegmTest);
  CPPUNIT_TEST(testTimegm);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testTimegm();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TimegmTest);

namespace {
void setTime(struct tm* tm, int yr, int mon, int day, int h, int m, int s)
{
  tm->tm_year = yr - 1900;
  tm->tm_mon = mon-1;
  tm->tm_mday = day;
  tm->tm_hour = h;
  tm->tm_min = m;
  tm->tm_sec = s;
}
} // namespace

void TimegmTest::testTimegm()
{
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  setTime(&tm, 1970, 1, 1, 0, 0, 0);
  CPPUNIT_ASSERT_EQUAL((time_t)0, timegm(&tm));
  setTime(&tm, 2000, 1, 2, 1, 2, 3);
  CPPUNIT_ASSERT_EQUAL((time_t)946774923, timegm(&tm));
  setTime(&tm, 2000, 2, 2, 1, 2, 3);
  CPPUNIT_ASSERT_EQUAL((time_t)949453323, timegm(&tm));
  setTime(&tm, 2015, 10, 21, 10, 19, 30);
  CPPUNIT_ASSERT_EQUAL((time_t)1445422770, timegm(&tm));
  setTime(&tm, 1970, 13, 1, 0, 0, 0);
  CPPUNIT_ASSERT_EQUAL((time_t)-1, timegm(&tm));
  setTime(&tm, 2039, 1, 1, 0, 0, 0);
  if(sizeof(time_t) == 4) {
    CPPUNIT_ASSERT_EQUAL((time_t)-1, timegm(&tm));
  } else if(sizeof(time_t) == 8) {
    CPPUNIT_ASSERT_EQUAL((time_t)2177452800LL, timegm(&tm));
  }
}

} // namespace aria2
