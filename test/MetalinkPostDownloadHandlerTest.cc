#include "MetalinkPostDownloadHandler.h"
#include "RequestGroup.h"
#include "Option.h"
#include <cppunit/extensions/HelperMacros.h>

class MetalinkPostDownloadHandlerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkPostDownloadHandlerTest);
  CPPUNIT_TEST(testCanHandle);
  CPPUNIT_TEST(testGetNextRequestGroups);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testCanHandle();
  void testGetNextRequestGroups();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MetalinkPostDownloadHandlerTest );

void MetalinkPostDownloadHandlerTest::testCanHandle()
{
  Option op;
  MetalinkPostDownloadHandler handler(&op);
  CPPUNIT_ASSERT(!handler.canHandle(".metalink!!"));
  CPPUNIT_ASSERT(handler.canHandle(".metalink"));
}

void MetalinkPostDownloadHandlerTest::testGetNextRequestGroups()
{
  Option op;
  MetalinkPostDownloadHandler handler(&op);
  RequestGroups groups = handler.getNextRequestGroups("test.xml");
  CPPUNIT_ASSERT_EQUAL((size_t)6/* 5 + 1 torrent file download */, groups.size());
}
