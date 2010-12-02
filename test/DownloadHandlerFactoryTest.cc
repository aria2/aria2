#include "DownloadHandlerFactory.h"

#include <cppunit/extensions/HelperMacros.h>

#include "RequestGroup.h"
#include "Option.h"
#include "DownloadContext.h"
#include "MemoryBufferPreDownloadHandler.h"
#include "FileEntry.h"

namespace aria2 {

class DownloadHandlerFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DownloadHandlerFactoryTest);
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
  SharedHandle<Option> option_;
public:
  void setUp()
  {
    option_.reset(new Option());
  }
#ifdef ENABLE_METALINK
  void testGetMetalinkPreDownloadHandler_extension();
  void testGetMetalinkPreDownloadHandler_contentType();
#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT
  void testGetBtPreDownloadHandler_extension();
  void testGetBtPreDownloadHandler_contentType();
#endif // ENABLE_BITTORRENT

};


CPPUNIT_TEST_SUITE_REGISTRATION( DownloadHandlerFactoryTest );

#ifdef ENABLE_METALINK

void DownloadHandlerFactoryTest::testGetMetalinkPreDownloadHandler_extension()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext(0, 0, "test.metalink"));
  RequestGroup rg(option_);
  rg.setDownloadContext(dctx);

  SharedHandle<PreDownloadHandler> handler = DownloadHandlerFactory::getMetalinkPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->getFirstFileEntry()->setPath("test.metalink2");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

void DownloadHandlerFactoryTest::testGetMetalinkPreDownloadHandler_contentType()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext(0, 0, "test"));
  dctx->getFirstFileEntry()->setContentType("application/metalink+xml");
  RequestGroup rg(option_);
  rg.setDownloadContext(dctx);

  SharedHandle<PreDownloadHandler> handler = DownloadHandlerFactory::getMetalinkPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->getFirstFileEntry()->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT

void DownloadHandlerFactoryTest::testGetBtPreDownloadHandler_extension()
{
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(0, 0, A2_TEST_DIR"/test.torrent"));
  RequestGroup rg(option_);
  rg.setDownloadContext(dctx);

  SharedHandle<PreDownloadHandler> handler = DownloadHandlerFactory::getBtPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->getFirstFileEntry()->setPath(A2_TEST_DIR"/test.torrent2");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

void DownloadHandlerFactoryTest::testGetBtPreDownloadHandler_contentType()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext(0, 0, "test"));
  dctx->getFirstFileEntry()->setContentType("application/x-bittorrent");
  RequestGroup rg(option_);
  rg.setDownloadContext(dctx);

  SharedHandle<PreDownloadHandler> handler = DownloadHandlerFactory::getBtPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->getFirstFileEntry()->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

#endif // ENABLE_BITTORRENT

} // namespace aria2
