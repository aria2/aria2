#include "RequestGroupMan.h"

#include <fstream>

#include <cppunit/extensions/HelperMacros.h>

#include "prefs.h"
#include "SingleFileDownloadContext.h"
#include "RequestGroup.h"
#include "Option.h"
#include "DownloadResult.h"
#include "FileEntry.h"
#include "ServerStatMan.h"
#include "ServerStat.h"
#include "File.h"

namespace aria2 {

class RequestGroupManTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestGroupManTest);
  CPPUNIT_TEST(testIsSameFileBeingDownloaded);
  CPPUNIT_TEST(testGetInitialCommands);
  CPPUNIT_TEST(testLoadServerStat);
  CPPUNIT_TEST(testSaveServerStat);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void testIsSameFileBeingDownloaded();
  void testGetInitialCommands();
  void testLoadServerStat();
  void testSaveServerStat();
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

void RequestGroupManTest::testSaveServerStat()
{
  Option option;
  RequestGroupMan rm(std::deque<SharedHandle<RequestGroup> >(), 0, &option);
  SharedHandle<ServerStat> ss_localhost(new ServerStat("localhost", "http"));
  rm.addServerStat(ss_localhost);
  File f("/tmp/aria2_RequestGroupManTest_testSaveServerStat");
  if(f.exists()) {
    f.remove();
  }
  CPPUNIT_ASSERT(rm.saveServerStat(f.getPath()));
  CPPUNIT_ASSERT(f.isFile());

  f.remove();
  CPPUNIT_ASSERT(f.mkdirs());
  CPPUNIT_ASSERT(!rm.saveServerStat(f.getPath()));
}

void RequestGroupManTest::testLoadServerStat()
{
  File f("/tmp/aria2_RequestGroupManTest_testLoadServerStat");
  std::ofstream o(f.getPath().c_str(), std::ios::binary);
  o << "host=localhost, protocol=http, dl_speed=0, last_updated=1219505257,"
    << "status=OK";
  o.close();

  Option option;
  RequestGroupMan rm(std::deque<SharedHandle<RequestGroup> >(), 0, &option);
  std::cerr << "testLoadServerStat" << std::endl;
  CPPUNIT_ASSERT(rm.loadServerStat(f.getPath()));
  SharedHandle<ServerStat> ss_localhost = rm.findServerStat("localhost",
							    "http");
  CPPUNIT_ASSERT(!ss_localhost.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), ss_localhost->getHostname());
}

} // namespace aria2
