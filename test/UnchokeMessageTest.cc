#include "UnchokeMessage.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class UnchokeMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UnchokeMessageTest);
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


CPPUNIT_TEST_SUITE_REGISTRATION(UnchokeMessageTest);

void UnchokeMessageTest::testCreate() {
  char msg[5];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 1);
  PeerMessage* pm = UnchokeMessage::create(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL(1, pm->getId());

  // case: payload size is wrong
  try {
    char msg[6];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 2, 1);
    UnchokeMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    char msg[5];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 2);
    UnchokeMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void UnchokeMessageTest::testGetMessage() {
  UnchokeMessage msg;
  char data[5];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 1, 1);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 5) == 0);
}
