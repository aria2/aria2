#include "NotInterestedMessage.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class NotInterestedMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(NotInterestedMessageTest);
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


CPPUNIT_TEST_SUITE_REGISTRATION(NotInterestedMessageTest);

void NotInterestedMessageTest::testCreate() {
  char msg[5];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 3);
  PeerMessage* pm = NotInterestedMessage::create(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL(3, pm->getId());

  // case: payload size is wrong
  try {
    char msg[6];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 2, 3);
    NotInterestedMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    char msg[5];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 4);
    NotInterestedMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void NotInterestedMessageTest::testGetMessage() {
  NotInterestedMessage msg;
  char data[5];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 1, 3);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 5) == 0);
}
