#include "DownloadHandlerFactory.h"
#include "RequestGroup.h"
#include "Option.h"
#include "SingleFileDownloadContext.h"
#include "MemoryBufferPreDownloadHandler.h"
#include <cppunit/extensions/HelperMacros.h>

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
  SingleFileDownloadContextHandle dctx = new SingleFileDownloadContext(0, 0, "test.metalink");
  RequestGroup rg(&op, Strings());
  rg.setDownloadContext(dctx);

  PreDownloadHandlerHandle handler = DownloadHandlerFactory::getMetalinkPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->setFilename("test.metalink2");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

void DownloadHandlerFactoryTest::testGetMetalinkPreDownloadHandler_contentType()
{
  Option op;
  SingleFileDownloadContextHandle dctx = new SingleFileDownloadContext(0, 0, "test");
  dctx->setContentType("application/metalink+xml");
  RequestGroup rg(&op, Strings());
  rg.setDownloadContext(dctx);

  PreDownloadHandlerHandle handler = DownloadHandlerFactory::getMetalinkPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

void DownloadHandlerFactoryTest::testGetBtPreDownloadHandler_extension()
{
  Option op;
  SingleFileDownloadContextHandle dctx = new SingleFileDownloadContext(0, 0, "test.torrent");
  RequestGroup rg(&op, Strings());
  rg.setDownloadContext(dctx);

  PreDownloadHandlerHandle handler = DownloadHandlerFactory::getBtPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->setFilename("test.torrent2");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}

void DownloadHandlerFactoryTest::testGetBtPreDownloadHandler_contentType()
{
  Option op;
  SingleFileDownloadContextHandle dctx = new SingleFileDownloadContext(0, 0, "test");
  dctx->setContentType("application/x-bittorrent");
  RequestGroup rg(&op, Strings());
  rg.setDownloadContext(dctx);

  PreDownloadHandlerHandle handler = DownloadHandlerFactory::getBtPreDownloadHandler();

  CPPUNIT_ASSERT(handler->canHandle(&rg));

  dctx->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler->canHandle(&rg));
}
