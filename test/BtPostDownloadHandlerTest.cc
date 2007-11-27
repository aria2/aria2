#include "BtPostDownloadHandler.h"
#include "BtContext.h"
#include "RequestGroup.h"
#include "Option.h"
#include "SingleFileDownloadContext.h"
#include <cppunit/extensions/HelperMacros.h>

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
  SingleFileDownloadContextHandle dctx = new SingleFileDownloadContext(0, 0, "test.torrent");
  RequestGroup rg(&op, Strings());
  rg.setDownloadContext(dctx);

  BtPostDownloadHandler handler;

  CPPUNIT_ASSERT(handler.canHandle(&rg));

  dctx->setFilename("test.torrent2");
  CPPUNIT_ASSERT(!handler.canHandle(&rg));
}

void BtPostDownloadHandlerTest::testCanHandle_contentType()
{
  Option op;
  SingleFileDownloadContextHandle dctx = new SingleFileDownloadContext(0, 0, "test");
  dctx->setContentType("application/x-bittorrent");
  RequestGroup rg(&op, Strings());
  rg.setDownloadContext(dctx);

  BtPostDownloadHandler handler;

  CPPUNIT_ASSERT(handler.canHandle(&rg));

  dctx->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler.canHandle(&rg));
}

void BtPostDownloadHandlerTest::testGetNextRequestGroups()
{
  Option op;
  SingleFileDownloadContextHandle dctx = new SingleFileDownloadContext(0, 0, "test.torrent");
  RequestGroup rg(&op, Strings());
  rg.setDownloadContext(dctx);
  rg.initPieceStorage();

  BtPostDownloadHandler handler;
  RequestGroups groups = handler.getNextRequestGroups(&rg);
  CPPUNIT_ASSERT_EQUAL((size_t)1, groups.size());
  BtContextHandle btctx = groups.front()->getDownloadContext();
  CPPUNIT_ASSERT(!btctx.isNull());
  CPPUNIT_ASSERT_EQUAL(string("aria2-test"), btctx->getName());
}
