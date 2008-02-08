#include "SpeedCalc.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class SpeedCalcTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SpeedCalcTest);
  CPPUNIT_TEST(testUpdate);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testUpdate();
};


CPPUNIT_TEST_SUITE_REGISTRATION( SpeedCalcTest );

void SpeedCalcTest::testUpdate() {
  SpeedCalc calc;
  calc.update(1000);
}

} // namespace aria2
