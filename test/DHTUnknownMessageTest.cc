#include "DHTUnknownMessage.h"
#include "DHTNode.h"
#include "Exception.h"
#include <cppunit/extensions/HelperMacros.h>

class DHTUnknownMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTUnknownMessageTest);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testToString();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTUnknownMessageTest);

void DHTUnknownMessageTest::testToString()
{
  DHTNodeHandle localNode = new DHTNode();
  string ipaddr = "192.168.0.1";
  uint16_t port = 6881;

  {
    // data.size() > 8
    string data = "chocolate";
    DHTUnknownMessage msg(localNode, remoteNode, data.c_str(), data.size(),
			  ipaddr, port);

    CPPUNIT_ASSERT_EQUAL(string("dht unknown Remote:192.168.0.1:6881 length=9, first 8 bytes(hex)=63686f636f6c617465"), msg.toString());
  }
  {
    // data.size() == 3
    string data = "foo";
    DHTUnknownMessage msg(localNode, remoteNode, data.c_str(), data.size(),
			  ipaddr, port);

    CPPUNIT_ASSERT_EQUAL(string("dht unknown Remote:192.168.0.1:6881 length=9, first 8 bytes(hex)=66666f"), msg.toString());
  }
}
