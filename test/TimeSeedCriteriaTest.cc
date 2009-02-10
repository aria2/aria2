#include "TimeSeedCriteria.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Util.h"

namespace aria2 {

class TimeSeedCriteriaTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TimeSeedCriteriaTest);
  CPPUNIT_TEST(testEvaluate);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testEvaluate();
};


CPPUNIT_TEST_SUITE_REGISTRATION(TimeSeedCriteriaTest);

void TimeSeedCriteriaTest::testEvaluate() {
  TimeSeedCriteria cri(1);
  // Seel 2seconds. 1 seconds are not enough in some systems.
  Util::sleep(2);
  CPPUNIT_ASSERT(cri.evaluate());
  cri.reset();
  cri.setDuration(10);
  CPPUNIT_ASSERT(!cri.evaluate());
}

} // namespace aria2
