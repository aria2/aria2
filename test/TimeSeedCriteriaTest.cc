#include "TimeSeedCriteria.h"

#include <cppunit/extensions/HelperMacros.h>

#include "util.h"
#include "wallclock.h"

namespace aria2 {

class TimeSeedCriteriaTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TimeSeedCriteriaTest);
  CPPUNIT_TEST(testEvaluate);
  CPPUNIT_TEST_SUITE_END();

public:
  void testEvaluate();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TimeSeedCriteriaTest);

void TimeSeedCriteriaTest::testEvaluate()
{
  TimeSeedCriteria cri(1_s);
  global::wallclock().reset();
  global::wallclock().advance(2_s);
  CPPUNIT_ASSERT(cri.evaluate());
  cri.reset();
  cri.setDuration(10_s);
  CPPUNIT_ASSERT(!cri.evaluate());
}

} // namespace aria2
