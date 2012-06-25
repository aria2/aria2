#include "RequestGroup.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Option.h"
#include "DownloadContext.h"
#include "FileEntry.h"
#include "PieceStorage.h"
#include "DownloadResult.h"

namespace aria2 {

class RequestGroupTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestGroupTest);
  CPPUNIT_TEST(testGetFirstFilePath);
  CPPUNIT_TEST(testCreateDownloadResult);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Option> option_;
public:
  void setUp()
  {
    option_.reset(new Option());
  }

  void testGetFirstFilePath();
  void testCreateDownloadResult();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestGroupTest );

void RequestGroupTest::testGetFirstFilePath()
{
  SharedHandle<DownloadContext> ctx
    (new DownloadContext(1024, 1024, "/tmp/myfile"));

  RequestGroup group(option_);
  group.setDownloadContext(ctx);

  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/myfile"), group.getFirstFilePath());

  group.markInMemoryDownload();

  CPPUNIT_ASSERT_EQUAL(std::string("[MEMORY]myfile"), group.getFirstFilePath());
}

void RequestGroupTest::testCreateDownloadResult()
{
  SharedHandle<DownloadContext> ctx
    (new DownloadContext(1024, 1024*1024, "/tmp/myfile"));
  RequestGroup group(option_);
  group.setDownloadContext(ctx);
  group.initPieceStorage();
  {
    SharedHandle<DownloadResult> result = group.createDownloadResult();
  
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/myfile"),
                         result->fileEntries[0]->getPath());
    CPPUNIT_ASSERT_EQUAL((int64_t)1024*1024,
                         result->fileEntries.back()->getLastOffset());
    CPPUNIT_ASSERT_EQUAL((uint64_t)0, result->sessionDownloadLength);
    CPPUNIT_ASSERT_EQUAL((int64_t)0, result->sessionTime);
    // result is UNKNOWN_ERROR if download has not completed and no specific
    // error has been reported
    CPPUNIT_ASSERT_EQUAL(error_code::UNKNOWN_ERROR, result->result);

    // if haltReason is set to RequestGroup::USER_REQUEST, download
    // result will become REMOVED.
    group.setHaltRequested(true, RequestGroup::USER_REQUEST);
    result = group.createDownloadResult();
    CPPUNIT_ASSERT_EQUAL(error_code::REMOVED, result->result);
    // if haltReason is set to RequestGroup::SHUTDOWN_SIGNAL, download
    // result will become IN_PROGRESS.
    group.setHaltRequested(true, RequestGroup::SHUTDOWN_SIGNAL);
    result = group.createDownloadResult();
    CPPUNIT_ASSERT_EQUAL(error_code::IN_PROGRESS, result->result);
  }
  {
    group.setLastErrorCode(error_code::RESOURCE_NOT_FOUND);
  
    SharedHandle<DownloadResult> result = group.createDownloadResult();

    CPPUNIT_ASSERT_EQUAL(error_code::RESOURCE_NOT_FOUND, result->result);
  }
  {
    group.getPieceStorage()->markAllPiecesDone();

    SharedHandle<DownloadResult> result = group.createDownloadResult();

    CPPUNIT_ASSERT_EQUAL(error_code::FINISHED, result->result);
  }
}

} // namespace aria2
