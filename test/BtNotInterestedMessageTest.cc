#include "BtNotInterestedMessage.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtNotInterestedMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtNotInterestedMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testOnSendComplete);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreate();
  void testGetMessage();
  void testDoReceivedAction();
  void testOnSendComplete();
  void testToString();
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtNotInterestedMessageTest);

void BtNotInterestedMessageTest::testCreate() {
  unsigned char msg[5];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 3);
  BtNotInterestedMessageHandle pm = BtNotInterestedMessage::create(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL(3, pm->getId());

  // case: payload size is wrong
  try {
    unsigned char msg[6];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 2, 3);
    BtNotInterestedMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[5];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 4);
    BtNotInterestedMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void BtNotInterestedMessageTest::testGetMessage() {
  BtNotInterestedMessage msg;
  char data[5];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 1, 3);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 5) == 0);
}

void BtNotInterestedMessageTest::testDoReceivedAction() {
  PeerHandle peer = new Peer("host", 6969, 16*1024, 256*1024);
  peer->peerInterested = true;
  BtNotInterestedMessage msg;
  msg.setPeer(peer);
  CPPUNIT_ASSERT(peer->peerInterested);
  msg.doReceivedAction();
  CPPUNIT_ASSERT(!peer->peerInterested);
}

void BtNotInterestedMessageTest::testOnSendComplete() {
  PeerHandle peer = new Peer("host", 6969, 16*1024, 256*1024);
  peer->amInterested = true;
  BtNotInterestedMessage msg;
  msg.setPeer(peer);
  CPPUNIT_ASSERT(peer->amInterested);
  msg.onSendComplete();
  CPPUNIT_ASSERT(!peer->amInterested);
}

void BtNotInterestedMessageTest::testToString() {
  BtNotInterestedMessage msg;
  CPPUNIT_ASSERT_EQUAL(string("not interested"), msg.toString());
}
