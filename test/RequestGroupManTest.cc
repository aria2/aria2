#include "RequestGroupMan.h"
#include "CUIDCounter.h"
#include "prefs.h"
#include "SingleFileDownloadContext.h"
#include "RequestGroup.h"
#include "Option.h"
#include "DownloadResult.h"
#include "FileEntry.h"
#include "ServerStatMan.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class RequestGroupManTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestGroupManTest);
  CPPUNIT_TEST(testIsSameFileBeingDownloaded);
  CPPUNIT_TEST(testGetInitialCommands);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp()
  {
    SharedHandle<CUIDCounter> counter(new CUIDCounter());
    SingletonHolder<SharedHandle<CUIDCounter> >::instance(counter);
  }

  void testIsSameFileBeingDownloaded();
  void testGetInitialCommands();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestGroupManTest );

void RequestGroupManTest::testIsSameFileBeingDownloaded()
{
  Option option;

  std::deque<std::string> uris;
  uris.push_back("http://localhost/aria2.tar.bz2");
  SharedHandle<RequestGroup> rg1(new RequestGroup(&option, uris));
  SharedHandle<RequestGroup> rg2(new RequestGroup(&option, uris));

  SharedHandle<SingleFileDownloadContext> dctx1
    (new SingleFileDownloadContext(0, 0, "aria2.tar.bz2"));
  SharedHandle<SingleFileDownloadContext> dctx2
    (new SingleFileDownloadContext(0, 0, "aria2.tar.bz2"));

  rg1->setDownloadContext(dctx1);
  rg2->setDownloadContext(dctx2);

  RequestGroups rgs;
  rgs.push_back(rg1);
  rgs.push_back(rg2);

  RequestGroupMan gm(rgs, 1, &option);
  
  CPPUNIT_ASSERT(gm.isSameFileBeingDownloaded(rg1.get()));

  dctx2->setFilename("aria2.tar.gz");

  CPPUNIT_ASSERT(!gm.isSameFileBeingDownloaded(rg1.get()));

}

void RequestGroupManTest::testGetInitialCommands()
{
  // TODO implement later
}

} // namespace aria2
