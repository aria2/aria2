#include "BtKeepAliveMessage.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class BtKeepAliveMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtKeepAliveMessageTest);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreateMessage();
  void testToString();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtKeepAliveMessageTest);

void BtKeepAliveMessageTest::testCreateMessage() {
  char data[4];
  memset(data, 0, sizeof(data));
  BtKeepAliveMessage message;
  CPPUNIT_ASSERT_EQUAL((uint8_t)99, message.getId());
  CPPUNIT_ASSERT_EQUAL((size_t)4, message.getMessageLength());
  unsigned char* rawmsg = message.createMessage();
  CPPUNIT_ASSERT(memcmp(rawmsg, data, 4) == 0);
  delete [] rawmsg;
}

void BtKeepAliveMessageTest::testToString() {
  BtKeepAliveMessage msg;
  CPPUNIT_ASSERT_EQUAL(std::string("keep alive"), msg.toString());
}

} // namespace aria2
