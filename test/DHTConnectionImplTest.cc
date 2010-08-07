#include "DHTConnectionImpl.h"

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "SocketCore.h"
#include "A2STR.h"

namespace aria2 {

class DHTConnectionImplTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTConnectionImplTest);
  CPPUNIT_TEST(testWriteAndReadData);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testWriteAndReadData();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTConnectionImplTest);

void DHTConnectionImplTest::testWriteAndReadData()
{
  try {
    DHTConnectionImpl con1(AF_INET);
    uint16_t con1port = 0;
    CPPUNIT_ASSERT(con1.bind(con1port, A2STR::NIL));

    DHTConnectionImpl con2(AF_INET);
    uint16_t con2port = 0;
    CPPUNIT_ASSERT(con2.bind(con2port, A2STR::NIL));

    std::string message1 = "hello world.";
    // hostname should be "localhost", not 127.0.0.1. Test failed on Mac OSX10.5
    con1.sendMessage(reinterpret_cast<const unsigned char*>(message1.c_str()),
                     message1.size(), "localhost", con2port);

    unsigned char readbuffer[100];
    std::string remoteHost;
    uint16_t remotePort;
    {
      while(!con2.getSocket()->isReadable(0));
      ssize_t rlength = con2.receiveMessage(readbuffer, sizeof(readbuffer), remoteHost, remotePort);
      CPPUNIT_ASSERT_EQUAL((ssize_t)message1.size(), rlength);
      CPPUNIT_ASSERT_EQUAL(message1,
                           std::string(&readbuffer[0], &readbuffer[rlength]));
    }
  } catch(Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

} // namespace aria2
