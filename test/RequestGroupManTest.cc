#include "RequestGroupMan.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class RequestGroupManTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestGroupManTest);
  CPPUNIT_TEST(testIsSameFileBeingDownloaded);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testIsSameFileBeingDownloaded();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestGroupManTest );

void RequestGroupManTest::testIsSameFileBeingDownloaded()
{
  Option option;
  RequestGroupMan gm;

  RequestGroupHandle rg1 = new RequestGroup("http://localhost/aria2.tar.bz2",
					    &option);
  RequestGroupHandle rg2 = new RequestGroup("http://localhost/aria2.tar.bz2",
					    &option);

  gm.addRequestGroup(rg1);
  gm.addRequestGroup(rg2);

  rg1->initSegmentMan();
  rg2->initSegmentMan();

  rg1->getSegmentMan()->filename = "aria2.tar.bz2";
  rg2->getSegmentMan()->filename = "aria2.tar.bz2";

  CPPUNIT_ASSERT(gm.isSameFileBeingDownloaded(rg1.get()));

  rg2->getSegmentMan()->filename = "aria2-0.10.2.tar.bz2";

  CPPUNIT_ASSERT(!gm.isSameFileBeingDownloaded(rg1.get()));

}
