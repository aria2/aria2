#include "SocketCore.h"
#include "Exception.h"
#include <cppunit/extensions/HelperMacros.h>

class SocketCoreTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SocketCoreTest);
  CPPUNIT_TEST(testWriteAndReadDatagram);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testWriteAndReadDatagram();
};


CPPUNIT_TEST_SUITE_REGISTRATION(SocketCoreTest);

void SocketCoreTest::testWriteAndReadDatagram()
{
  try {
    SocketCore s(SOCK_DGRAM);
    s.bind(0);
    SocketCore c(SOCK_DGRAM);
    c.bind(0);

    pair<string, int32_t> svaddr;
    s.getAddrInfo(svaddr);

    string message1 = "hello world.";
    c.writeData(message1.c_str(), message1.size(), "localhost", svaddr.second);
    string message2 = "chocolate coated pie";
    c.writeData(message2.c_str(), message2.size(), "localhost", svaddr.second);

    char readbuffer[100];
    pair<string, uint16_t> peer;
    {
      ssize_t rlength = s.readDataFrom(readbuffer, sizeof(readbuffer), peer);
      CPPUNIT_ASSERT_EQUAL(string("127.0.0.1"), peer.first);
      CPPUNIT_ASSERT_EQUAL((ssize_t)message1.size(), rlength);
      readbuffer[rlength] = '\0';
      CPPUNIT_ASSERT_EQUAL(message1, string(readbuffer));
    }
    {
      ssize_t rlength = s.readDataFrom(readbuffer, sizeof(readbuffer));
      CPPUNIT_ASSERT_EQUAL((ssize_t)message2.size(), rlength);
      readbuffer[rlength] = '\0';
      CPPUNIT_ASSERT_EQUAL(message2, string(readbuffer));
    }
  } catch(Exception* e) {
    cerr << *e << endl;
    delete e;
    CPPUNIT_FAIL("exception thrown");
  }
}
