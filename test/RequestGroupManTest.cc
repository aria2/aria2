#include "RequestGroupMan.h"
#include "CUIDCounter.h"
#include "prefs.h"
#include "RequestFactory.h"
#include "SingleFileDownloadContext.h"
#include "RequestGroup.h"
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

  Strings uris;
  uris.push_back("http://localhost/aria2.tar.bz2");
  RequestGroupHandle rg1 = new RequestGroup(&option, uris);
  RequestGroupHandle rg2 = new RequestGroup(&option, uris);

  SingleFileDownloadContextHandle dctx1 =
    new SingleFileDownloadContext(0, 0, "aria2.tar.bz2");
  SingleFileDownloadContextHandle dctx2 =
    new SingleFileDownloadContext(0, 0, "aria2.tar.bz2");

  rg1->setDownloadContext(dctx1);
  rg2->setDownloadContext(dctx2);

  RequestGroups rgs;
  rgs.push_back(rg1);
  rgs.push_back(rg2);

  RequestGroupMan gm(rgs, 1);
  
  CPPUNIT_ASSERT(gm.isSameFileBeingDownloaded(rg1.get()));

  dctx2->setFilename("aria2.tar.gz");

  CPPUNIT_ASSERT(!gm.isSameFileBeingDownloaded(rg1.get()));

}

void RequestGroupManTest::testGetInitialCommands()
{
  // TODO implement later
  /*
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
  */
}
