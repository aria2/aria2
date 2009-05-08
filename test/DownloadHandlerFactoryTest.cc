#include "DownloadHandlerFactory.h"

#include <cppunit/extensions/HelperMacros.h>

#include "RequestGroup.h"
#include "Option.h"
#include "SingleFileDownloadContext.h"
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
  SharedHandle<Option> _option;
public:
  void setUp()
  {
    _option.reset(new Option());
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
  SharedHandle<SingleFileDownloadContext> dctx
    (new SingleFileDownloadContext(0, 0, "test.metalink"));
  RequestGroup rg(_option, std::deque<std::string>());
  rg.setDownloadContext(dctx);

  SharedHandle<PreDownloadHandler> handler = DownloadHandlerFactory::getMetalinkPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->setFilename("test.metalink2");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

void DownloadHandlerFactoryTest::testGetMetalinkPreDownloadHandler_contentType()
{
  SharedHandle<SingleFileDownloadContext> dctx
    (new SingleFileDownloadContext(0, 0, "test"));
  dctx->setContentType("application/metalink+xml");
  RequestGroup rg(_option, std::deque<std::string>());
  rg.setDownloadContext(dctx);

  SharedHandle<PreDownloadHandler> handler = DownloadHandlerFactory::getMetalinkPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT

void DownloadHandlerFactoryTest::testGetBtPreDownloadHandler_extension()
{
  SharedHandle<SingleFileDownloadContext> dctx
    (new SingleFileDownloadContext(0, 0, "test.torrent"));
  RequestGroup rg(_option, std::deque<std::string>());
  rg.setDownloadContext(dctx);

  SharedHandle<PreDownloadHandler> handler = DownloadHandlerFactory::getBtPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->setFilename("test.torrent2");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

void DownloadHandlerFactoryTest::testGetBtPreDownloadHandler_contentType()
{
  SharedHandle<SingleFileDownloadContext> dctx
    (new SingleFileDownloadContext(0, 0, "test"));
  dctx->setContentType("application/x-bittorrent");
  RequestGroup rg(_option, std::deque<std::string>());
  rg.setDownloadContext(dctx);

  SharedHandle<PreDownloadHandler> handler = DownloadHandlerFactory::getBtPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

#endif // ENABLE_BITTORRENT

} // namespace aria2
