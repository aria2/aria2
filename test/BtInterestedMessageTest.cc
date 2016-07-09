#include "BtInterestedMessage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "Peer.h"
#include "MockPeerStorage.h"
#include "SocketBuffer.h"

namespace aria2 {

class BtInterestedMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtInterestedMessageTest);
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

CPPUNIT_TEST_SUITE_REGISTRATION(BtInterestedMessageTest);

void BtInterestedMessageTest::testCreate()
{
  unsigned char msg[5];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 2);
  std::shared_ptr<BtInterestedMessage> pm(
      BtInterestedMessage::create(&msg[4], 1));
  CPPUNIT_ASSERT_EQUAL((uint8_t)2, pm->getId());

  // case: payload size is wrong
  try {
    unsigned char msg[6];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 2, 2);
    BtInterestedMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[5];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 3);
    BtInterestedMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtInterestedMessageTest::testCreateMessage()
{
  BtInterestedMessage msg;
  unsigned char data[5];
  bittorrent::createPeerMessageString(data, sizeof(data), 1, 2);
  auto rawmsg = msg.createMessage();
  CPPUNIT_ASSERT_EQUAL((size_t)5, rawmsg.size());
  CPPUNIT_ASSERT(std::equal(std::begin(rawmsg), std::end(rawmsg), data));
}

void BtInterestedMessageTest::testDoReceivedAction()
{
  BtInterestedMessage msg;
  std::shared_ptr<Peer> peer(new Peer("host", 6969));
  peer->allocateSessionResource(1_k, 1_m);
  msg.setPeer(peer);

  auto peerStorage = make_unique<MockPeerStorage>();

  msg.setPeerStorage(peerStorage.get());

  CPPUNIT_ASSERT(!peer->peerInterested());
  msg.doReceivedAction();
  CPPUNIT_ASSERT(peer->peerInterested());
  CPPUNIT_ASSERT_EQUAL(1, peerStorage->getNumChokeExecuted());

  peer->amChoking(false);
  msg.doReceivedAction();
  CPPUNIT_ASSERT_EQUAL(1, peerStorage->getNumChokeExecuted());
}

void BtInterestedMessageTest::testToString()
{
  BtInterestedMessage msg;
  CPPUNIT_ASSERT_EQUAL(std::string("interested"), msg.toString());
}

} // namespace aria2
