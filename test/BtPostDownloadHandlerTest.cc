#include "BtPostDownloadHandler.h"
#include "BtContext.h"
#include "RequestGroup.h"
#include "Option.h"
#include <cppunit/extensions/HelperMacros.h>

class BtPostDownloadHandlerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtPostDownloadHandlerTest);
  CPPUNIT_TEST(testCanHandle);
  CPPUNIT_TEST(testGetNextRequestGroups);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testCanHandle();
  void testGetNextRequestGroups();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BtPostDownloadHandlerTest );

void BtPostDownloadHandlerTest::testCanHandle()
{
  Option op;
  BtPostDownloadHandler handler(&op);
  CPPUNIT_ASSERT(!handler.canHandle(".torrent!!"));
  CPPUNIT_ASSERT(handler.canHandle(".torrent"));
}

void BtPostDownloadHandlerTest::testGetNextRequestGroups()
{
  Option op;
  BtPostDownloadHandler handler(&op);
  RequestGroups groups = handler.getNextRequestGroups("test.torrent");
  CPPUNIT_ASSERT_EQUAL((size_t)1, groups.size());
  BtContextHandle btctx = groups.front()->getDownloadContext();
  CPPUNIT_ASSERT(!btctx.isNull());
  CPPUNIT_ASSERT_EQUAL(string("aria2-test"), btctx->getName());
}
