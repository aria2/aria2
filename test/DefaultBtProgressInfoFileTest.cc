#include "DefaultBtProgressInfoFile.h"
#include "DefaultBtContext.h"
#include "Option.h"
#include "Util.h"
#include "Exception.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class DefaultBtProgressInfoFileTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtProgressInfoFileTest);
  CPPUNIT_TEST(testSave);
  CPPUNIT_TEST_SUITE_END();
private:
  BtContextHandle btContext;
  Option* option;
public:
  DefaultBtProgressInfoFileTest():btContext(0) {}

  void setUp() {
    btContext = BtContextHandle(new DefaultBtContext());
    btContext->load("test.torrent");
    option = new Option();
  }

  void testSave();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtProgressInfoFileTest);

void DefaultBtProgressInfoFileTest::testSave() {
}
