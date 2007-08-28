#include "RequestGroup.h"
#include "prefs.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class RequestGroupTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestGroupTest);
  CPPUNIT_TEST(testTryAutoFileRenaming);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testTryAutoFileRenaming();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestGroupTest );

void RequestGroupTest::testTryAutoFileRenaming()
{
  Option op;
  op.put(PREF_DIR, ".");
  RequestGroup rg("http://localhost/RequestGroupTest.cc", &op);
  rg.setUserDefinedFilename("RequestGroupTest.cc");
  rg.initSegmentMan();
  CPPUNIT_ASSERT(rg.tryAutoFileRenaming());
  CPPUNIT_ASSERT_EQUAL(string("./RequestGroupTest.cc.1"), rg.getFilePath());
}
