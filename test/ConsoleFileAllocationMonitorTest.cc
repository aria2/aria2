#include "ConsoleFileAllocationMonitor.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class ConsoleFileAllocationMonitorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ConsoleFileAllocationMonitorTest);
  CPPUNIT_TEST(testShowProgress);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testShowProgress();
};


CPPUNIT_TEST_SUITE_REGISTRATION( ConsoleFileAllocationMonitorTest );

void ConsoleFileAllocationMonitorTest::testShowProgress() {
  ConsoleFileAllocationMonitor monitor;
  monitor.setMinValue(0);
  monitor.setMaxValue(1000000000);
  monitor.setCurrentValue(0);

  cout << endl;
  for(uint64_t i = monitor.getMinValue(); i <= monitor.getMaxValue(); i += 1234343) {
    monitor.setCurrentValue(i);
    monitor.showProgress();
  }
  monitor.setCurrentValue(monitor.getMaxValue());
  monitor.showProgress();
}
