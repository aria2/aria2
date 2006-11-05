#include "DefaultBtAnnounce.h"
#include "DefaultBtContext.h"
#include "Option.h"
#include "Util.h"
#include "Exception.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class DefaultBtAnnounceTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtAnnounceTest);
  CPPUNIT_TEST(testIsDefaultAnnounceReady);
  CPPUNIT_TEST_SUITE_END();
private:
  BtContextHandle btContext;
  Option* option;
public:
  DefaultBtAnnounceTest():btContext(0) {}

  void setUp() {
    btContext = BtContextHandle(new DefaultBtContext());
    btContext->load("test.torrent");
    option = new Option();
  }

  void testIsDefaultAnnounceReady();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtAnnounceTest);

void DefaultBtAnnounceTest::testIsDefaultAnnounceReady() {
  DefaultBtAnnounce btAnnounce(btContext, option);

  CPPUNIT_ASSERT(btAnnounce.isDefaultAnnounceReady());
}
