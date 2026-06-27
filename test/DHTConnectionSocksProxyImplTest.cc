#include "DHTConnectionSocksProxyImpl.h"

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "SocketCore.h"
#include "A2STR.h"

namespace aria2 {

class DHTConnectionSocksProxyImplTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTConnectionSocksProxyImplTest);
  CPPUNIT_TEST(testWriteAndReadData);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testWriteAndReadData();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DHTConnectionSocksProxyImplTest);

void DHTConnectionSocksProxyImplTest::testWriteAndReadData()
{
  try {
    DHTConnectionSocksProxyImpl con1(AF_INET);
    uint16_t con1port = 0;
    CPPUNIT_ASSERT(con1.bind(con1port, "127.0.0.1"));

    DHTConnectionImpl con2(AF_INET);
    uint16_t con2port = 0;
    CPPUNIT_ASSERT(con2.bind(con2port, "127.0.0.1"));

    // TODO: Requires a SOCKS5 proxy server for tests
    return;
    CPPUNIT_ASSERT(
        con1.startProxy("localhost", 10801, "", "", "127.0.0.1", con1port));

    std::string message1 = "hello world.";
    con1.sendMessage(reinterpret_cast<const unsigned char*>(message1.c_str()),
                     message1.size(), "127.0.0.1", con2port);

    unsigned char readbuffer[100];
    std::string remoteHost;
    uint16_t remotePort;
    {
      while (!con2.getSocket()->isReadable(0))
        ;
      ssize_t rlength = con2.receiveMessage(readbuffer, sizeof(readbuffer),
                                            remoteHost, remotePort);
      CPPUNIT_ASSERT_EQUAL((ssize_t)message1.size(), rlength);
      CPPUNIT_ASSERT_EQUAL(message1,
                           std::string(&readbuffer[0], &readbuffer[rlength]));
    }

    std::string message2 = "hello world too.";
    con2.sendMessage(reinterpret_cast<const unsigned char*>(message2.c_str()),
                     message2.size(), remoteHost, remotePort);

    {
      std::string remoteHost;
      uint16_t remotePort;
      while (!con1.getSocket()->isReadable(0))
        ;
      ssize_t rlength = con1.receiveMessage(readbuffer, sizeof(readbuffer),
                                            remoteHost, remotePort);
      CPPUNIT_ASSERT_EQUAL((ssize_t)message2.size(), rlength);
      CPPUNIT_ASSERT_EQUAL(message2,
                           std::string(&readbuffer[0], &readbuffer[rlength]));
    }
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

} // namespace aria2
