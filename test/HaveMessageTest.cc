#include "HaveMessage.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class HaveMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HaveMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreate();
  void testGetMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(HaveMessageTest);

void HaveMessageTest::testCreate() {
  char msg[9];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, 4);
  PeerMessageUtil::setIntParam(&msg[5], 12345);
  HaveMessage* pm = HaveMessage::create(&msg[4], 5);
  CPPUNIT_ASSERT_EQUAL(4, pm->getId());
  CPPUNIT_ASSERT_EQUAL(12345, pm->getIndex());

  // case: payload size is wrong
  try {
    char msg[10];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 6, 4);
    HaveMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    char msg[9];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, 5);
    HaveMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void HaveMessageTest::testGetMessage() {
  HaveMessage msg;
  msg.setIndex(12345);
  char data[9];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 5, 4);
  PeerMessageUtil::setIntParam(&data[5], 12345);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 9) == 0);
}
