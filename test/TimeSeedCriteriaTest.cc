#include "TimeSeedCriteria.h"
#include "Util.h"
#include <cppunit/extensions/HelperMacros.h>

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
  Util::sleep(1);
  CPPUNIT_ASSERT(cri.evaluate());
  cri.reset();
  cri.setDuration(10);
  CPPUNIT_ASSERT(!cri.evaluate());
}

} // namespace aria2
