#include "BtKeepAliveMessage.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class BtKeepAliveMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtKeepAliveMessageTest);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testGetMessage();
  void testToString();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtKeepAliveMessageTest);

void BtKeepAliveMessageTest::testGetMessage() {
  char msg[4];
  memset(msg, 0, sizeof(msg));
  BtKeepAliveMessage message;
  CPPUNIT_ASSERT_EQUAL((uint8_t)99, message.getId());
  CPPUNIT_ASSERT_EQUAL((size_t)4, message.getMessageLength());
  CPPUNIT_ASSERT(memcmp(msg, message.getMessage(), 4) == 0);
}

void BtKeepAliveMessageTest::testToString() {
  BtKeepAliveMessage msg;
  CPPUNIT_ASSERT_EQUAL(std::string("keep alive"), msg.toString());
}

} // namespace aria2
