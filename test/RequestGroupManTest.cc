#include "RequestGroupMan.h"
#include "ConsoleDownloadEngine.h"
#include "CUIDCounter.h"
#include "prefs.h"
#include "RequestFactory.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class RequestGroupManTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestGroupManTest);
  CPPUNIT_TEST(testIsSameFileBeingDownloaded);
  CPPUNIT_TEST(testGetInitialCommands);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp()
  {
    CUIDCounterHandle counter = new CUIDCounter();
    CUIDCounterSingletonHolder::instance(counter);
  }

  void testIsSameFileBeingDownloaded();
  void testGetInitialCommands();
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

void RequestGroupManTest::testGetInitialCommands()
{
  Option option;
  option.put(PREF_SPLIT, "1");
  option.put(PREF_TIMEOUT, "10");

  RequestFactoryHandle requestFactory = new RequestFactory();
  requestFactory->setOption(&option);
  RequestFactorySingletonHolder::instance(requestFactory);

  RequestGroupMan gm;

  RequestGroupHandle rg1 = new RequestGroup("aria2.tar.bz2.metalink",
					    &option);
  RequestGroupHandle rg2 = new RequestGroup("http://localhost/aria2.tar.bz2",
					    &option);

  gm.addRequestGroup(rg1);
  gm.addRequestGroup(rg2);

  ConsoleDownloadEngine e;
  e.option = &option;
  Commands commands = gm.getInitialCommands(&e);
  CPPUNIT_ASSERT_EQUAL((size_t)1, commands.size());
}
