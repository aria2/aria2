#include "BtUnchokeMessage.h"
#include "bittorrent_helper.h"
#include "Peer.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class BtUnchokeMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtUnchokeMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testOnSendComplete);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreate();
  void testCreateMessage();
  void testDoReceivedAction();
  void testOnSendComplete();
  void testToString();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtUnchokeMessageTest);

void BtUnchokeMessageTest::testCreate() {
  unsigned char msg[5];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 1);
  SharedHandle<BtUnchokeMessage> pm = BtUnchokeMessage::create(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, pm->getId());

  // case: payload size is wrong
  try {
    unsigned char msg[6];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 2, 1);
    BtUnchokeMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[5];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 2);
    BtUnchokeMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
}

void BtUnchokeMessageTest::testCreateMessage() {
  BtUnchokeMessage msg;
  unsigned char data[5];
  bittorrent::createPeerMessageString(data, sizeof(data), 1, 1);
  unsigned char* rawmsg = msg.createMessage();
  CPPUNIT_ASSERT(memcmp(rawmsg, data, 5) == 0);
  delete [] rawmsg;
}

void BtUnchokeMessageTest::testDoReceivedAction() {
  SharedHandle<Peer> peer(new Peer("host", 6969));
  peer->allocateSessionResource(1024, 1024*1024);
  peer->peerChoking(true);
  BtUnchokeMessage msg;
  msg.setPeer(peer);

  CPPUNIT_ASSERT(peer->peerChoking());
  msg.doReceivedAction();
  CPPUNIT_ASSERT(!peer->peerChoking());
}

void BtUnchokeMessageTest::testOnSendComplete() {
  SharedHandle<Peer> peer(new Peer("host", 6969));
  peer->allocateSessionResource(1024, 1024*1024);
  peer->amChoking(true);
  BtUnchokeMessage msg;
  msg.setPeer(peer);

  CPPUNIT_ASSERT(peer->amChoking());
  msg.onSendComplete();
  CPPUNIT_ASSERT(!peer->amChoking());
}

void BtUnchokeMessageTest::testToString() {
  BtUnchokeMessage msg;
  CPPUNIT_ASSERT_EQUAL(std::string("unchoke"), msg.toString());
}

} // namespace aria2
