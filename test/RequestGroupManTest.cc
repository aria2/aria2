#include "RequestGroupMan.h"

#include <fstream>

#include <cppunit/extensions/HelperMacros.h>

#include "prefs.h"
#include "DownloadContext.h"
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
  SharedHandle<Option> _option;
public:
  void setUp()
  {
    _option.reset(new Option());
  }

  void testIsSameFileBeingDownloaded();
  void testGetInitialCommands();
  void testLoadServerStat();
  void testSaveServerStat();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestGroupManTest );

void RequestGroupManTest::testIsSameFileBeingDownloaded()
{
  SharedHandle<RequestGroup> rg1(new RequestGroup(_option));
  SharedHandle<RequestGroup> rg2(new RequestGroup(_option));

  SharedHandle<DownloadContext> dctx1
    (new DownloadContext(0, 0, "aria2.tar.bz2"));
  SharedHandle<DownloadContext> dctx2
    (new DownloadContext(0, 0, "aria2.tar.bz2"));

  rg1->setDownloadContext(dctx1);
  rg2->setDownloadContext(dctx2);

  RequestGroups rgs;
  rgs.push_back(rg1);
  rgs.push_back(rg2);

  RequestGroupMan gm(rgs, 1, _option.get());
  
  CPPUNIT_ASSERT(gm.isSameFileBeingDownloaded(rg1.get()));

  dctx2->getFirstFileEntry()->setPath("aria2.tar.gz");

  CPPUNIT_ASSERT(!gm.isSameFileBeingDownloaded(rg1.get()));

}

void RequestGroupManTest::testGetInitialCommands()
{
  // TODO implement later
}

void RequestGroupManTest::testSaveServerStat()
{
  RequestGroupMan rm(std::deque<SharedHandle<RequestGroup> >(),0,_option.get());
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

  RequestGroupMan rm(std::deque<SharedHandle<RequestGroup> >(),0,_option.get());
  std::cerr << "testLoadServerStat" << std::endl;
  CPPUNIT_ASSERT(rm.loadServerStat(f.getPath()));
  SharedHandle<ServerStat> ss_localhost = rm.findServerStat("localhost",
							    "http");
  CPPUNIT_ASSERT(!ss_localhost.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), ss_localhost->getHostname());
}

} // namespace aria2
