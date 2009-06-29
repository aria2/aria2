#include "RequestGroup.h"

#include <cppunit/extensions/HelperMacros.h>

#include "ServerHost.h"
#include "Option.h"
#include "DownloadContext.h"
#include "FileEntry.h"
#include "PieceStorage.h"
#include "DownloadResult.h"

namespace aria2 {

class RequestGroupTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestGroupTest);
  CPPUNIT_TEST(testRegisterSearchRemove);
  CPPUNIT_TEST(testGetFirstFilePath);
  CPPUNIT_TEST(testCreateDownloadResult);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Option> _option;
public:
  void setUp()
  {
    _option.reset(new Option());
  }

  void testRegisterSearchRemove();
  void testGetFirstFilePath();
  void testCreateDownloadResult();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestGroupTest );

void RequestGroupTest::testRegisterSearchRemove()
{
  RequestGroup rg(_option);
  SharedHandle<ServerHost> sv1(new ServerHost(1, "localhost1"));
  SharedHandle<ServerHost> sv2(new ServerHost(2, "localhost2"));
  SharedHandle<ServerHost> sv3(new ServerHost(3, "localhost3"));

  rg.registerServerHost(sv3);
  rg.registerServerHost(sv1);
  rg.registerServerHost(sv2);

  CPPUNIT_ASSERT(rg.searchServerHost(0).isNull());

  {
    SharedHandle<ServerHost> sv = rg.searchServerHost(1);
    CPPUNIT_ASSERT(!sv.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("localhost1"), sv->getHostname());
  }

  rg.removeServerHost(1);

  {
    SharedHandle<ServerHost> sv = rg.searchServerHost(1);
    CPPUNIT_ASSERT(sv.isNull());
  }
  {
    SharedHandle<ServerHost> sv = rg.searchServerHost(2);
    CPPUNIT_ASSERT(!sv.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("localhost2"), sv->getHostname());
  }
}

void RequestGroupTest::testGetFirstFilePath()
{
  SharedHandle<DownloadContext> ctx
    (new DownloadContext(1024, 1024, "/tmp/myfile"));

  RequestGroup group(_option);
  group.setDownloadContext(ctx);

  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/myfile"), group.getFirstFilePath());

  group.markInMemoryDownload();

  CPPUNIT_ASSERT_EQUAL(std::string("[MEMORY]myfile"), group.getFirstFilePath());
}

void RequestGroupTest::testCreateDownloadResult()
{
  SharedHandle<DownloadContext> ctx
    (new DownloadContext(1024, 1024*1024, "/tmp/myfile"));
  RequestGroup group(_option);
  group.setDownloadContext(ctx);
  group.initPieceStorage();
  {
    SharedHandle<DownloadResult> result = group.createDownloadResult();
  
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/myfile"),
			 result->fileEntries[0]->getPath());
    CPPUNIT_ASSERT_EQUAL((off_t)1024*1024,
			 result->fileEntries.back()->getLastOffset());
    CPPUNIT_ASSERT_EQUAL((uint64_t)0, result->sessionDownloadLength);
    CPPUNIT_ASSERT_EQUAL((int64_t)0, result->sessionTime);
    // result is UNKNOWN_ERROR if download has not completed and no specific
    // error has been reported
    CPPUNIT_ASSERT_EQUAL(downloadresultcode::UNKNOWN_ERROR, result->result);

    // if haltReason is set to RequestGroup::USER_REQUEST, download
    // result becomes IN_PROGRESS
    group.setHaltRequested(true, RequestGroup::USER_REQUEST);
    result = group.createDownloadResult();
    CPPUNIT_ASSERT_EQUAL(downloadresultcode::IN_PROGRESS, result->result);
  }
  {
    group.setLastUriResult
      ("http://second/file",downloadresultcode::RESOURCE_NOT_FOUND);
  
    SharedHandle<DownloadResult> result = group.createDownloadResult();

    CPPUNIT_ASSERT_EQUAL(downloadresultcode::RESOURCE_NOT_FOUND, result->result);
  }
  {
    group.getPieceStorage()->markAllPiecesDone();

    SharedHandle<DownloadResult> result = group.createDownloadResult();

    CPPUNIT_ASSERT_EQUAL(downloadresultcode::FINISHED, result->result);
  }
}

} // namespace aria2
