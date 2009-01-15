#include "RequestGroup.h"

#include <cppunit/extensions/HelperMacros.h>

#include "ServerHost.h"
#include "Option.h"
#include "SingleFileDownloadContext.h"
#include "FileEntry.h"
#include "PieceStorage.h"

namespace aria2 {

class RequestGroupTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestGroupTest);
  CPPUNIT_TEST(testRegisterSearchRemove);
  CPPUNIT_TEST(testRemoveURIWhoseHostnameIs);
  CPPUNIT_TEST(testGetFilePath);
  CPPUNIT_TEST(testCreateDownloadResult);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testRegisterSearchRemove();
  void testRemoveURIWhoseHostnameIs();
  void testGetFilePath();
  void testCreateDownloadResult();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestGroupTest );

void RequestGroupTest::testRegisterSearchRemove()
{
  Option op;
  RequestGroup rg(&op, std::deque<std::string>());
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

void RequestGroupTest::testRemoveURIWhoseHostnameIs()
{
  const char* uris[] = { "http://localhost/aria2.zip",
			 "ftp://localhost/aria2.zip",
			 "http://mirror/aria2.zip" };
  Option op;
  RequestGroup rg(&op, std::deque<std::string>(&uris[0], &uris[3]));
  rg.removeURIWhoseHostnameIs("localhost");
  CPPUNIT_ASSERT_EQUAL((size_t)1, rg.getRemainingUris().size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/aria2.zip"),
		       rg.getRemainingUris()[0]);
}

void RequestGroupTest::testGetFilePath()
{
  SharedHandle<SingleFileDownloadContext> ctx
    (new SingleFileDownloadContext(1024, 1024, "myfile"));
  ctx->setDir("/tmp");
  Option op;
  std::deque<std::string> uris;

  RequestGroup group(&op, uris);
  group.setDownloadContext(ctx);

  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/myfile"), group.getFilePath());

  group.markInMemoryDownload();

  CPPUNIT_ASSERT_EQUAL(std::string("[MEMORY]myfile"), group.getFilePath());
}

void RequestGroupTest::testCreateDownloadResult()
{
  SharedHandle<SingleFileDownloadContext> ctx
    (new SingleFileDownloadContext(1024, 1024*1024, "myfile"));
  ctx->setDir("/tmp");
  Option op;
  std::deque<std::string> uris;
  uris.push_back("http://first/file");
  uris.push_back("http://second/file");

  RequestGroup group(&op, uris);
  group.setDownloadContext(ctx);
  group.initPieceStorage();
  {
    SharedHandle<DownloadResult> result = group.createDownloadResult();
  
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/myfile"), result->filePath);
    CPPUNIT_ASSERT_EQUAL((uint64_t)1024*1024, result->totalLength);
    CPPUNIT_ASSERT_EQUAL(std::string("http://first/file"), result->uri);
    CPPUNIT_ASSERT_EQUAL((size_t)2, result->numUri);
    CPPUNIT_ASSERT_EQUAL((uint64_t)0, result->sessionDownloadLength);
    CPPUNIT_ASSERT_EQUAL((int64_t)0, result->sessionTime);
    // result is UNKNOWN_ERROR if download has not completed and no specific
    // error has been reported
    CPPUNIT_ASSERT_EQUAL(DownloadResult::UNKNOWN_ERROR, result->result);
  }
  {
    group.addURIResult("http://first/file", DownloadResult::TIME_OUT);
    group.addURIResult("http://second/file",DownloadResult::RESOURCE_NOT_FOUND);
  
    SharedHandle<DownloadResult> result = group.createDownloadResult();

    CPPUNIT_ASSERT_EQUAL(DownloadResult::RESOURCE_NOT_FOUND, result->result);
  }
  {
    group.getPieceStorage()->markAllPiecesDone();

    SharedHandle<DownloadResult> result = group.createDownloadResult();

    CPPUNIT_ASSERT_EQUAL(DownloadResult::FINISHED, result->result);
  }
}

} // namespace aria2
