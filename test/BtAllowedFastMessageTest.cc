#include "BtAllowedFastMessage.h"

#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "util.h"
#include "Peer.h"
#include "SocketBuffer.h"

namespace aria2 {

class BtAllowedFastMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtAllowedFastMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testOnSendComplete);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testCreate();
  void testCreateMessage();
  void testDoReceivedAction();
  void testOnSendComplete();
  void testToString();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtAllowedFastMessageTest);

void BtAllowedFastMessageTest::testCreate()
{
  unsigned char msg[9];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 5, 17);
  bittorrent::setIntParam(&msg[5], 12345);
  std::shared_ptr<BtAllowedFastMessage> pm(
      BtAllowedFastMessage::create(&msg[4], 5));
  CPPUNIT_ASSERT_EQUAL((uint8_t)17, pm->getId());
  CPPUNIT_ASSERT_EQUAL((size_t)12345, pm->getIndex());

  // case: payload size is wrong
  try {
    unsigned char msg[10];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 6, 17);
    BtAllowedFastMessage::create(&msg[4], 6);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[9];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 5, 18);
    BtAllowedFastMessage::create(&msg[4], 5);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtAllowedFastMessageTest::testCreateMessage()
{
  BtAllowedFastMessage msg;
  msg.setIndex(12345);
  unsigned char data[9];
  bittorrent::createPeerMessageString(data, sizeof(data), 5, 17);
  bittorrent::setIntParam(&data[5], 12345);
  auto rawmsg = msg.createMessage();
  CPPUNIT_ASSERT_EQUAL((size_t)9, rawmsg.size());
  CPPUNIT_ASSERT(std::equal(std::begin(rawmsg), std::end(rawmsg), data));
}

void BtAllowedFastMessageTest::testDoReceivedAction()
{
  BtAllowedFastMessage msg;
  msg.setIndex(1);
  std::shared_ptr<Peer> peer(new Peer("localhost", 6969));
  peer->allocateSessionResource(1_k, 1_m);
  peer->setFastExtensionEnabled(true);
  msg.setPeer(peer);
  CPPUNIT_ASSERT(!peer->isInPeerAllowedIndexSet(1));
  msg.doReceivedAction();
  CPPUNIT_ASSERT(peer->isInPeerAllowedIndexSet(1));

  peer->setFastExtensionEnabled(false);
  try {
    msg.doReceivedAction();
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtAllowedFastMessageTest::testOnSendComplete()
{
  BtAllowedFastMessage msg;
  msg.setIndex(1);
  std::shared_ptr<Peer> peer(new Peer("localhost", 6969));
  peer->allocateSessionResource(1_k, 1_m);
  peer->setFastExtensionEnabled(true);
  msg.setPeer(peer);
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  std::shared_ptr<ProgressUpdate> pu(msg.getProgressUpdate());
  pu->update(0, true);
  CPPUNIT_ASSERT(peer->isInAmAllowedIndexSet(1));
}

void BtAllowedFastMessageTest::testToString()
{
  BtAllowedFastMessage msg;
  msg.setIndex(1);
  CPPUNIT_ASSERT_EQUAL(std::string("allowed fast index=1"), msg.toString());
}

} // namespace aria2
