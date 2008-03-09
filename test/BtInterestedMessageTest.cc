#include "BtInterestedMessage.h"
#include "PeerMessageUtil.h"
#include "Peer.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class BtInterestedMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtInterestedMessageTest);
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


CPPUNIT_TEST_SUITE_REGISTRATION(BtInterestedMessageTest);

void BtInterestedMessageTest::testCreate() {
  unsigned char msg[5];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 2);
  BtInterestedMessageHandle pm = BtInterestedMessage::create(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL((uint8_t)2, pm->getId());

  // case: payload size is wrong
  try {
    unsigned char msg[6];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 2, 2);
    BtInterestedMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[5];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 3);
    BtInterestedMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
}

void BtInterestedMessageTest::testGetMessage() {
  BtInterestedMessage msg;
  unsigned char data[5];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 1, 2);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 5) == 0);
}

void BtInterestedMessageTest::testDoReceivedAction() {
  BtInterestedMessage msg;
  SharedHandle<Peer> peer = new Peer("host", 6969);
  peer->allocateSessionResource(1024, 1024*1024);
  msg.setPeer(peer);
  CPPUNIT_ASSERT(!peer->peerInterested());
  msg.doReceivedAction();
  CPPUNIT_ASSERT(peer->peerInterested());
}

void BtInterestedMessageTest::testOnSendComplete() {
  BtInterestedMessage msg;
  SharedHandle<Peer> peer = new Peer("host", 6969);
  peer->allocateSessionResource(1024, 1024*1024);
  msg.setPeer(peer);
  CPPUNIT_ASSERT(!peer->amInterested());
  msg.onSendComplete();
  CPPUNIT_ASSERT(peer->amInterested());
}

void BtInterestedMessageTest::testToString() {
  BtInterestedMessage msg;
  CPPUNIT_ASSERT_EQUAL(std::string("interested"), msg.toString());
}

} // namespace aria2
