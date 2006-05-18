#include "HaveNoneMessage.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class HaveNoneMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HaveNoneMessageTest);
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


CPPUNIT_TEST_SUITE_REGISTRATION(HaveNoneMessageTest);

void HaveNoneMessageTest::testCreate() {
  char msg[5];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 15);
  PeerMessage* pm = HaveNoneMessage::create(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL(15, pm->getId());

  // case: payload size is wrong
  try {
    char msg[6];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 2, 15);
    HaveNoneMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    char msg[5];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 16);
    HaveNoneMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void HaveNoneMessageTest::testGetMessage() {
  HaveNoneMessage msg;
  char data[5];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 1, 15);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 5) == 0);
}
