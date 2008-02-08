#include "BtPostDownloadHandler.h"
#include "BtContext.h"
#include "RequestGroup.h"
#include "Option.h"
#include "SingleFileDownloadContext.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class BtPostDownloadHandlerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtPostDownloadHandlerTest);
  CPPUNIT_TEST(testCanHandle_extension);
  CPPUNIT_TEST(testCanHandle_contentType);
  CPPUNIT_TEST(testGetNextRequestGroups);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testCanHandle_extension();
  void testCanHandle_contentType();
  void testGetNextRequestGroups();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BtPostDownloadHandlerTest );

void BtPostDownloadHandlerTest::testCanHandle_extension()
{
  Option op;
  SharedHandle<SingleFileDownloadContext> dctx = new SingleFileDownloadContext(0, 0, "test.torrent");
  RequestGroup rg(&op, std::deque<std::string>());
  rg.setDownloadContext(dctx);

  BtPostDownloadHandler handler;

  CPPUNIT_ASSERT(handler.canHandle(&rg));

  dctx->setFilename("test.torrent2");
  CPPUNIT_ASSERT(!handler.canHandle(&rg));
}

void BtPostDownloadHandlerTest::testCanHandle_contentType()
{
  Option op;
  SharedHandle<SingleFileDownloadContext> dctx = new SingleFileDownloadContext(0, 0, "test");
  dctx->setContentType("application/x-bittorrent");
  RequestGroup rg(&op, std::deque<std::string>());
  rg.setDownloadContext(dctx);

  BtPostDownloadHandler handler;

  CPPUNIT_ASSERT(handler.canHandle(&rg));

  dctx->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler.canHandle(&rg));
}

void BtPostDownloadHandlerTest::testGetNextRequestGroups()
{
  Option op;
  SharedHandle<SingleFileDownloadContext> dctx = new SingleFileDownloadContext(0, 0, "test.torrent");
  RequestGroup rg(&op, std::deque<std::string>());
  rg.setDownloadContext(dctx);
  rg.initPieceStorage();

  BtPostDownloadHandler handler;
  std::deque<SharedHandle<RequestGroup> > groups = handler.getNextRequestGroups(&rg);
  CPPUNIT_ASSERT_EQUAL((size_t)1, groups.size());
  SharedHandle<BtContext> btctx = groups.front()->getDownloadContext();
  CPPUNIT_ASSERT(!btctx.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-test"), btctx->getName());
}

} // namespace aria2
