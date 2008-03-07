#include "DHTConnectionImpl.h"
#include "Exception.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

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
    DHTConnectionImpl con1;
    /*uint16_t con1port =*/ con1.bind(0);
    DHTConnectionImpl con2;
    uint16_t con2port = con2.bind(0);

    std::string message1 = "hello world.";
    con1.sendMessage(reinterpret_cast<const unsigned char*>(message1.c_str()),
		     message1.size(), "127.0.0.1", con2port);

    unsigned char readbuffer[100];
    std::string remoteHost;
    uint16_t remotePort;
    {
      ssize_t rlength = con2.receiveMessage(readbuffer, sizeof(readbuffer), remoteHost, remotePort);
      CPPUNIT_ASSERT_EQUAL((ssize_t)message1.size(), rlength);
      CPPUNIT_ASSERT_EQUAL(message1,
			   std::string(&readbuffer[0], &readbuffer[rlength]));
    }
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    delete e;
    CPPUNIT_FAIL("exception thrown");
  }
}

} // namespace aria2
