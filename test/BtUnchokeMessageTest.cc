#include "BtUnchokeMessage.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtUnchokeMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtUnchokeMessageTest);
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

CPPUNIT_TEST_SUITE_REGISTRATION(BtUnchokeMessageTest);

void BtUnchokeMessageTest::testCreate() {
  unsigned char msg[5];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 1);
  BtUnchokeMessageHandle pm = BtUnchokeMessage::create(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, pm->getId());

  // case: payload size is wrong
  try {
    unsigned char msg[6];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 2, 1);
    BtUnchokeMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[5];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 2);
    BtUnchokeMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void BtUnchokeMessageTest::testGetMessage() {
  BtUnchokeMessage msg;
  unsigned char data[5];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 1, 1);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 5) == 0);
}

void BtUnchokeMessageTest::testDoReceivedAction() {
  PeerHandle peer = new Peer("host", 6969, 16*1024, 256*1024);
  peer->peerChoking = true;
  BtUnchokeMessage msg;
  msg.setPeer(peer);

  CPPUNIT_ASSERT(peer->peerChoking);
  msg.doReceivedAction();
  CPPUNIT_ASSERT(!peer->peerChoking);
}

void BtUnchokeMessageTest::testOnSendComplete() {
  PeerHandle peer = new Peer("host", 6969, 16*1024, 256*1024);
  peer->amChoking = true;
  BtUnchokeMessage msg;
  msg.setPeer(peer);

  CPPUNIT_ASSERT(peer->amChoking);
  msg.onSendComplete();
  CPPUNIT_ASSERT(!peer->amChoking);
}

void BtUnchokeMessageTest::testToString() {
  BtUnchokeMessage msg;
  CPPUNIT_ASSERT_EQUAL(string("unchoke"), msg.toString());
}
