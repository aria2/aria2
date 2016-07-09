#include "BtUnchokeMessage.h"

#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "Peer.h"
#include "SocketBuffer.h"

namespace aria2 {

class BtUnchokeMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtUnchokeMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testCreate();
  void testCreateMessage();
  void testDoReceivedAction();
  void testToString();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtUnchokeMessageTest);

void BtUnchokeMessageTest::testCreate()
{
  unsigned char msg[5];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 1);
  std::shared_ptr<BtUnchokeMessage> pm(BtUnchokeMessage::create(&msg[4], 1));
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, pm->getId());

  // case: payload size is wrong
  try {
    unsigned char msg[6];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 2, 1);
    BtUnchokeMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[5];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 2);
    BtUnchokeMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtUnchokeMessageTest::testCreateMessage()
{
  BtUnchokeMessage msg;
  unsigned char data[5];
  bittorrent::createPeerMessageString(data, sizeof(data), 1, 1);
  auto rawmsg = msg.createMessage();
  CPPUNIT_ASSERT_EQUAL((size_t)5, rawmsg.size());
  CPPUNIT_ASSERT(std::equal(std::begin(rawmsg), std::end(rawmsg), data));
}

void BtUnchokeMessageTest::testDoReceivedAction()
{
  std::shared_ptr<Peer> peer(new Peer("host", 6969));
  peer->allocateSessionResource(1_k, 1_m);
  peer->peerChoking(true);
  BtUnchokeMessage msg;
  msg.setPeer(peer);

  CPPUNIT_ASSERT(peer->peerChoking());
  msg.doReceivedAction();
  CPPUNIT_ASSERT(!peer->peerChoking());
}

void BtUnchokeMessageTest::testToString()
{
  BtUnchokeMessage msg;
  CPPUNIT_ASSERT_EQUAL(std::string("unchoke"), msg.toString());
}

} // namespace aria2
