#include "HaveAllMessage.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class HaveAllMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HaveAllMessageTest);
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


CPPUNIT_TEST_SUITE_REGISTRATION(HaveAllMessageTest);

void HaveAllMessageTest::testCreate() {
  char msg[5];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 14);
  PeerMessage* pm = HaveAllMessage::create(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL(14, pm->getId());

  // case: payload size is wrong
  try {
    char msg[6];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 2, 14);
    HaveAllMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    char msg[5];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 15);
    HaveAllMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void HaveAllMessageTest::testGetMessage() {
  HaveAllMessage msg;
  char data[5];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 1, 14);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 5) == 0);
}
