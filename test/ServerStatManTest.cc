#include "ServerStatMan.h"
#include "ServerStat.h"
#include "Exception.h"
#include "Util.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class ServerStatManTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ServerStatManTest);
  CPPUNIT_TEST(testAddAndFind);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testAddAndFind();
};


CPPUNIT_TEST_SUITE_REGISTRATION(ServerStatManTest);

void ServerStatManTest::testAddAndFind()
{
  SharedHandle<ServerStat> localhost_http(new ServerStat("localhost", "http"));
  SharedHandle<ServerStat> localhost_ftp(new ServerStat("localhost", "ftp"));
  SharedHandle<ServerStat> mirror(new ServerStat("mirror", "http"));

  ServerStatMan ssm;
  CPPUNIT_ASSERT(ssm.add(localhost_http));
  CPPUNIT_ASSERT(!ssm.add(localhost_http));
  CPPUNIT_ASSERT(ssm.add(localhost_ftp));
  CPPUNIT_ASSERT(ssm.add(mirror));

  {
    SharedHandle<ServerStat> r = ssm.find("localhost", "http");
    CPPUNIT_ASSERT(!r.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("localhost"), r->getHostname());
    CPPUNIT_ASSERT_EQUAL(std::string("http"), r->getProtocol());
  }
  {
    SharedHandle<ServerStat> r = ssm.find("mirror", "ftp");
    CPPUNIT_ASSERT(r.isNull());
  }
}

} // namespace aria2
