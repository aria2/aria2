#include "DownloadHandlerFactory.h"
#include "RequestGroup.h"
#include "Option.h"
#include "SingleFileDownloadContext.h"
#include "MemoryBufferPreDownloadHandler.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DownloadHandlerFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DownloadHandlerFactoryTest);
  CPPUNIT_TEST(testGetMetalinkPreDownloadHandler_extension);
  CPPUNIT_TEST(testGetMetalinkPreDownloadHandler_contentType);
  CPPUNIT_TEST(testGetBtPreDownloadHandler_extension);
  CPPUNIT_TEST(testGetBtPreDownloadHandler_contentType);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testGetMetalinkPreDownloadHandler_extension();
  void testGetMetalinkPreDownloadHandler_contentType();
  void testGetBtPreDownloadHandler_extension();
  void testGetBtPreDownloadHandler_contentType();
};


CPPUNIT_TEST_SUITE_REGISTRATION( DownloadHandlerFactoryTest );

void DownloadHandlerFactoryTest::testGetMetalinkPreDownloadHandler_extension()
{
  Option op;
  SharedHandle<SingleFileDownloadContext> dctx = new SingleFileDownloadContext(0, 0, "test.metalink");
  RequestGroup rg(&op, std::deque<std::string>());
  rg.setDownloadContext(dctx);

  SharedHandle<PreDownloadHandler> handler = DownloadHandlerFactory::getMetalinkPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->setFilename("test.metalink2");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

void DownloadHandlerFactoryTest::testGetMetalinkPreDownloadHandler_contentType()
{
  Option op;
  SharedHandle<SingleFileDownloadContext> dctx = new SingleFileDownloadContext(0, 0, "test");
  dctx->setContentType("application/metalink+xml");
  RequestGroup rg(&op, std::deque<std::string>());
  rg.setDownloadContext(dctx);

  SharedHandle<PreDownloadHandler> handler = DownloadHandlerFactory::getMetalinkPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

void DownloadHandlerFactoryTest::testGetBtPreDownloadHandler_extension()
{
  Option op;
  SharedHandle<SingleFileDownloadContext> dctx = new SingleFileDownloadContext(0, 0, "test.torrent");
  RequestGroup rg(&op, std::deque<std::string>());
  rg.setDownloadContext(dctx);

  SharedHandle<PreDownloadHandler> handler = DownloadHandlerFactory::getBtPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->setFilename("test.torrent2");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

void DownloadHandlerFactoryTest::testGetBtPreDownloadHandler_contentType()
{
  Option op;
  SharedHandle<SingleFileDownloadContext> dctx = new SingleFileDownloadContext(0, 0, "test");
  dctx->setContentType("application/x-bittorrent");
  RequestGroup rg(&op, std::deque<std::string>());
  rg.setDownloadContext(dctx);

  SharedHandle<PreDownloadHandler> handler = DownloadHandlerFactory::getBtPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

} // namespace aria2
