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
#include "array_fun.h"
#include "RecoverableException.h"
#include "util.h"

namespace aria2 {

class RequestGroupManTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestGroupManTest);
  CPPUNIT_TEST(testIsSameFileBeingDownloaded);
  CPPUNIT_TEST(testGetInitialCommands);
  CPPUNIT_TEST(testLoadServerStat);
  CPPUNIT_TEST(testSaveServerStat);
  CPPUNIT_TEST(testChangeReservedGroupPosition);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Option> option_;
public:
  void setUp()
  {
    RequestGroup::resetGIDCounter();
    option_.reset(new Option());
  }

  void testIsSameFileBeingDownloaded();
  void testGetInitialCommands();
  void testLoadServerStat();
  void testSaveServerStat();
  void testChangeReservedGroupPosition();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestGroupManTest );

void RequestGroupManTest::testIsSameFileBeingDownloaded()
{
  SharedHandle<RequestGroup> rg1(new RequestGroup(util::copy(option_)));
  SharedHandle<RequestGroup> rg2(new RequestGroup(util::copy(option_)));

  SharedHandle<DownloadContext> dctx1
    (new DownloadContext(0, 0, "aria2.tar.bz2"));
  SharedHandle<DownloadContext> dctx2
    (new DownloadContext(0, 0, "aria2.tar.bz2"));

  rg1->setDownloadContext(dctx1);
  rg2->setDownloadContext(dctx2);

  RequestGroupMan gm(std::vector<SharedHandle<RequestGroup> >(), 1,
                     option_.get());

  gm.addRequestGroup(rg1);
  gm.addRequestGroup(rg2);
  
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
  RequestGroupMan rm
    (std::vector<SharedHandle<RequestGroup> >(),0,option_.get());
  SharedHandle<ServerStat> ss_localhost(new ServerStat("localhost", "http"));
  rm.addServerStat(ss_localhost);
  File f(A2_TEST_OUT_DIR"/aria2_RequestGroupManTest_testSaveServerStat");
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
  File f(A2_TEST_OUT_DIR"/aria2_RequestGroupManTest_testLoadServerStat");
  std::ofstream o(f.getPath().c_str(), std::ios::binary);
  o << "host=localhost, protocol=http, dl_speed=0, last_updated=1219505257,"
    << "status=OK";
  o.close();

  RequestGroupMan rm
    (std::vector<SharedHandle<RequestGroup> >(),0,option_.get());
  std::cerr << "testLoadServerStat" << std::endl;
  CPPUNIT_ASSERT(rm.loadServerStat(f.getPath()));
  SharedHandle<ServerStat> ss_localhost = rm.findServerStat("localhost",
                                                            "http");
  CPPUNIT_ASSERT(ss_localhost);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), ss_localhost->getHostname());
}

void RequestGroupManTest::testChangeReservedGroupPosition()
{
  SharedHandle<RequestGroup> gs[] = {
    SharedHandle<RequestGroup>(new RequestGroup(util::copy(option_))),
    SharedHandle<RequestGroup>(new RequestGroup(util::copy(option_))),
    SharedHandle<RequestGroup>(new RequestGroup(util::copy(option_))),
    SharedHandle<RequestGroup>(new RequestGroup(util::copy(option_)))
  };
  std::vector<SharedHandle<RequestGroup> > groups(vbegin(gs), vend(gs));
  RequestGroupMan rm(groups, 0, option_.get());

  CPPUNIT_ASSERT_EQUAL
    ((size_t)0, rm.changeReservedGroupPosition(1, 0, RequestGroupMan::POS_SET));
  CPPUNIT_ASSERT_EQUAL
    ((size_t)1, rm.changeReservedGroupPosition(1, 1, RequestGroupMan::POS_SET));
  CPPUNIT_ASSERT_EQUAL
    ((size_t)3, rm.changeReservedGroupPosition(1, 10,RequestGroupMan::POS_SET));
  CPPUNIT_ASSERT_EQUAL
    ((size_t)0, rm.changeReservedGroupPosition(1,-10,RequestGroupMan::POS_SET));
  
  CPPUNIT_ASSERT_EQUAL
    ((size_t)1, rm.changeReservedGroupPosition(2, 0, RequestGroupMan::POS_CUR));
  CPPUNIT_ASSERT_EQUAL
    ((size_t)2, rm.changeReservedGroupPosition(2, 1, RequestGroupMan::POS_CUR));
  CPPUNIT_ASSERT_EQUAL
    ((size_t)1, rm.changeReservedGroupPosition(2, -1,RequestGroupMan::POS_CUR));
  CPPUNIT_ASSERT_EQUAL
    ((size_t)0, rm.changeReservedGroupPosition(2,-10,RequestGroupMan::POS_CUR));
  CPPUNIT_ASSERT_EQUAL
    ((size_t)1, rm.changeReservedGroupPosition(2, 1, RequestGroupMan::POS_CUR));
  CPPUNIT_ASSERT_EQUAL
    ((size_t)3, rm.changeReservedGroupPosition(2, 10,RequestGroupMan::POS_CUR));
  CPPUNIT_ASSERT_EQUAL
    ((size_t)1, rm.changeReservedGroupPosition(2, -2,RequestGroupMan::POS_CUR));

  CPPUNIT_ASSERT_EQUAL
    ((size_t)3, rm.changeReservedGroupPosition(4, 0, RequestGroupMan::POS_END));
  CPPUNIT_ASSERT_EQUAL
    ((size_t)2, rm.changeReservedGroupPosition(4, -1,RequestGroupMan::POS_END));
  CPPUNIT_ASSERT_EQUAL
    ((size_t)0, rm.changeReservedGroupPosition(4,-10,RequestGroupMan::POS_END));
  CPPUNIT_ASSERT_EQUAL
    ((size_t)3, rm.changeReservedGroupPosition(4, 10,RequestGroupMan::POS_END));

  CPPUNIT_ASSERT_EQUAL((size_t)4, rm.getReservedGroups().size());

  try {
    rm.changeReservedGroupPosition(5, 0, RequestGroupMan::POS_CUR);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // success
  }
}

} // namespace aria2
