#include "download_handlers.h"

#include <cppunit/extensions/HelperMacros.h>

#include "RequestGroup.h"
#include "Option.h"
#include "DownloadContext.h"
#include "MemoryBufferPreDownloadHandler.h"
#include "FileEntry.h"
#include "RequestGroupCriteria.h"

namespace aria2 {

class DownloadHandlersTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DownloadHandlersTest);
  CPPUNIT_TEST(testGetMemoryPreDownloadHandler);
#ifdef ENABLE_METALINK
  CPPUNIT_TEST(testGetMetalinkPreDownloadHandler_extension);
  CPPUNIT_TEST(testGetMetalinkPreDownloadHandler_contentType);
#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT
  CPPUNIT_TEST(testGetBtPreDownloadHandler_extension);
  CPPUNIT_TEST(testGetBtPreDownloadHandler_contentType);
#endif // ENABLE_BITTORRENT

  CPPUNIT_TEST_SUITE_END();

private:
  std::shared_ptr<Option> option_;

public:
  void setUp() { option_ = std::make_shared<Option>(); }
  void testGetMemoryPreDownloadHandler();
#ifdef ENABLE_METALINK
  void testGetMetalinkPreDownloadHandler_extension();
  void testGetMetalinkPreDownloadHandler_contentType();
#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT
  void testGetBtPreDownloadHandler_extension();
  void testGetBtPreDownloadHandler_contentType();
#endif // ENABLE_BITTORRENT
};

CPPUNIT_TEST_SUITE_REGISTRATION(DownloadHandlersTest);

void DownloadHandlersTest::testGetMemoryPreDownloadHandler()
{
  CPPUNIT_ASSERT(
      download_handlers::getMemoryPreDownloadHandler()->canHandle(nullptr));
}

#ifdef ENABLE_METALINK

void DownloadHandlersTest::testGetMetalinkPreDownloadHandler_extension()
{
  auto dctx = std::make_shared<DownloadContext>(0, 0, "test.metalink");
  RequestGroup rg(GroupId::create(), option_);
  rg.setDownloadContext(dctx);

  auto handler = download_handlers::getMetalinkPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->getFirstFileEntry()->setPath("test.metalink2");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

void DownloadHandlersTest::testGetMetalinkPreDownloadHandler_contentType()
{
  auto dctx = std::make_shared<DownloadContext>(0, 0, "test");
  dctx->getFirstFileEntry()->setContentType("application/metalink+xml");
  RequestGroup rg(GroupId::create(), option_);
  rg.setDownloadContext(dctx);

  auto handler = download_handlers::getMetalinkPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->getFirstFileEntry()->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT

void DownloadHandlersTest::testGetBtPreDownloadHandler_extension()
{
  auto dctx =
      std::make_shared<DownloadContext>(0, 0, A2_TEST_DIR "/test.torrent");
  RequestGroup rg(GroupId::create(), option_);
  rg.setDownloadContext(dctx);

  auto handler = download_handlers::getBtPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->getFirstFileEntry()->setPath(A2_TEST_DIR "/test.torrent2");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

void DownloadHandlersTest::testGetBtPreDownloadHandler_contentType()
{
  auto dctx = std::make_shared<DownloadContext>(0, 0, "test");
  dctx->getFirstFileEntry()->setContentType("application/x-bittorrent");
  RequestGroup rg(GroupId::create(), option_);
  rg.setDownloadContext(dctx);

  auto handler = download_handlers::getBtPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->getFirstFileEntry()->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

#endif // ENABLE_BITTORRENT

} // namespace aria2
