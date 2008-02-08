#include "RequestGroup.h"
#include "ServerHost.h"
#include "Option.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class RequestGroupTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestGroupTest);
  CPPUNIT_TEST(testRegisterSearchRemove);
  CPPUNIT_TEST(testRemoveURIWhoseHostnameIs);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testRegisterSearchRemove();
  void testRemoveURIWhoseHostnameIs();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestGroupTest );

void RequestGroupTest::testRegisterSearchRemove()
{
  Option op;
  RequestGroup rg(&op, std::deque<std::string>());
  SharedHandle<ServerHost> sv1 = new ServerHost(1, "localhost1");
  SharedHandle<ServerHost> sv2 = new ServerHost(2, "localhost2");
  SharedHandle<ServerHost> sv3 = new ServerHost(3, "localhost3");

  rg.registerServerHost(sv3);
  rg.registerServerHost(sv1);
  rg.registerServerHost(sv2);

  CPPUNIT_ASSERT(rg.searchServerHost(0).isNull());

  {
    SharedHandle<ServerHost> sv = rg.searchServerHost(1);
    CPPUNIT_ASSERT(!sv.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("localhost1"), sv->getHostname());
  }

  rg.removeServerHost(1);

  {
    SharedHandle<ServerHost> sv = rg.searchServerHost(1);
    CPPUNIT_ASSERT(sv.isNull());
  }
  {
    SharedHandle<ServerHost> sv = rg.searchServerHost(2);
    CPPUNIT_ASSERT(!sv.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("localhost2"), sv->getHostname());
  }
}

void RequestGroupTest::testRemoveURIWhoseHostnameIs()
{
  const char* uris[] = { "http://localhost/aria2.zip",
			 "ftp://localhost/aria2.zip",
			 "http://mirror/aria2.zip" };
  Option op;
  RequestGroup rg(&op, std::deque<std::string>(&uris[0], &uris[3]));
  rg.removeURIWhoseHostnameIs("localhost");
  CPPUNIT_ASSERT_EQUAL((size_t)1, rg.getRemainingUris().size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/aria2.zip"),
		       rg.getRemainingUris()[0]);
}

} // namespace aria2
