#include "MetalinkPostDownloadHandler.h"
#include "RequestGroup.h"
#include "Option.h"
#include "SingleFileDownloadContext.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class MetalinkPostDownloadHandlerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkPostDownloadHandlerTest);
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


CPPUNIT_TEST_SUITE_REGISTRATION( MetalinkPostDownloadHandlerTest );

void MetalinkPostDownloadHandlerTest::testCanHandle_extension()
{
  Option op;
  SharedHandle<SingleFileDownloadContext> dctx = new SingleFileDownloadContext(0, 0, "test.metalink");
  RequestGroup rg(&op, std::deque<std::string>());
  rg.setDownloadContext(dctx);

  MetalinkPostDownloadHandler handler;

  CPPUNIT_ASSERT(handler.canHandle(&rg));

  dctx->setFilename("test.metalink2");
  CPPUNIT_ASSERT(!handler.canHandle(&rg));
}

void MetalinkPostDownloadHandlerTest::testCanHandle_contentType()
{
  Option op;
  SharedHandle<SingleFileDownloadContext> dctx = new SingleFileDownloadContext(0, 0, "test");
  dctx->setContentType("application/metalink+xml");
  RequestGroup rg(&op, std::deque<std::string>());
  rg.setDownloadContext(dctx);

  MetalinkPostDownloadHandler handler;

  CPPUNIT_ASSERT(handler.canHandle(&rg));

  dctx->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler.canHandle(&rg));
}

void MetalinkPostDownloadHandlerTest::testGetNextRequestGroups()
{
  Option op;
  SharedHandle<SingleFileDownloadContext> dctx = new SingleFileDownloadContext(0, 0, "test.xml");
  RequestGroup rg(&op, std::deque<std::string>());
  rg.setDownloadContext(dctx);
  rg.initPieceStorage();

  MetalinkPostDownloadHandler handler;
  RequestGroups groups = handler.getNextRequestGroups(&rg);
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL((size_t)6/* 5 + 1 torrent file download */, groups.size());
#else
  CPPUNIT_ASSERT_EQUAL((size_t)5, groups.size());
#endif // ENABLE_BITTORRENT
}

} // namespace aria2
