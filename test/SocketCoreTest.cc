#include "SocketCore.h"
#include "Exception.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class SocketCoreTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SocketCoreTest);
  CPPUNIT_TEST(testWriteAndReadDatagram);
  CPPUNIT_TEST(testGetSocketError);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testWriteAndReadDatagram();
  void testGetSocketError();
};


CPPUNIT_TEST_SUITE_REGISTRATION(SocketCoreTest);

void SocketCoreTest::testWriteAndReadDatagram()
{
  try {
    SocketCore s(SOCK_DGRAM);
    s.bind(0);
    SocketCore c(SOCK_DGRAM);
    c.bind(0);

    std::pair<std::string, uint16_t> svaddr;
    s.getAddrInfo(svaddr);

    std::string message1 = "hello world.";
    c.writeData(message1.c_str(), message1.size(), "localhost", svaddr.second);
    std::string message2 = "chocolate coated pie";
    c.writeData(message2.c_str(), message2.size(), "localhost", svaddr.second);

    char readbuffer[100];
    std::pair<std::string, uint16_t> peer;
    {
      ssize_t rlength = s.readDataFrom(readbuffer, sizeof(readbuffer), peer);
      // commented out because ip address may vary
      //CPPUNIT_ASSERT_EQUAL(std::std::string("127.0.0.1"), peer.first);
      CPPUNIT_ASSERT_EQUAL((ssize_t)message1.size(), rlength);
      readbuffer[rlength] = '\0';
      CPPUNIT_ASSERT_EQUAL(message1, std::string(readbuffer));
    }
    {
      ssize_t rlength = s.readDataFrom(readbuffer, sizeof(readbuffer), peer);
      CPPUNIT_ASSERT_EQUAL((ssize_t)message2.size(), rlength);
      readbuffer[rlength] = '\0';
      CPPUNIT_ASSERT_EQUAL(message2, std::string(readbuffer));
    }
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
    CPPUNIT_FAIL("exception thrown");
  }
}

void SocketCoreTest::testGetSocketError()
{
  SocketCore s;
  s.bind(0);
  // See there is no error at this point
  CPPUNIT_ASSERT_EQUAL(std::string(""), s.getSocketError());
}

} // namespace aria2
