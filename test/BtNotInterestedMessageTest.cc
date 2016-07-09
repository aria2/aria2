#include "BtNotInterestedMessage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "Peer.h"
#include "MockPeerStorage.h"
#include "SocketBuffer.h"

namespace aria2 {

class BtNotInterestedMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtNotInterestedMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCreate();
  void testCreateMessage();
  void testDoReceivedAction();
  void testToString();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtNotInterestedMessageTest);

void BtNotInterestedMessageTest::testCreate()
{
  unsigned char msg[5];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 3);
  std::shared_ptr<BtNotInterestedMessage> pm(
      BtNotInterestedMessage::create(&msg[4], 1));
  CPPUNIT_ASSERT_EQUAL((uint8_t)3, pm->getId());

  // case: payload size is wrong
  try {
    unsigned char msg[6];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 2, 3);
    BtNotInterestedMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[5];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 4);
    BtNotInterestedMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtNotInterestedMessageTest::testCreateMessage()
{
  BtNotInterestedMessage msg;
  unsigned char data[5];
  bittorrent::createPeerMessageString(data, sizeof(data), 1, 3);
  auto rawmsg = msg.createMessage();
  CPPUNIT_ASSERT_EQUAL((size_t)5, rawmsg.size());
  CPPUNIT_ASSERT(std::equal(std::begin(rawmsg), std::end(rawmsg), data));
}

void BtNotInterestedMessageTest::testDoReceivedAction()
{
  std::shared_ptr<Peer> peer(new Peer("host", 6969));
  peer->allocateSessionResource(1_k, 1_m);
  peer->peerInterested(true);

  auto peerStorage = make_unique<MockPeerStorage>();

  BtNotInterestedMessage msg;
  msg.setPeer(peer);
  msg.setPeerStorage(peerStorage.get());

  CPPUNIT_ASSERT(peer->peerInterested());
  msg.doReceivedAction();
  CPPUNIT_ASSERT(!peer->peerInterested());
  CPPUNIT_ASSERT_EQUAL(0, peerStorage->getNumChokeExecuted());

  peer->amChoking(false);
  msg.doReceivedAction();
  CPPUNIT_ASSERT_EQUAL(1, peerStorage->getNumChokeExecuted());
}

void BtNotInterestedMessageTest::testToString()
{
  BtNotInterestedMessage msg;
  CPPUNIT_ASSERT_EQUAL(std::string("not interested"), msg.toString());
}

} // namespace aria2
