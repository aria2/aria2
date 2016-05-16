#include "BtHaveAllMessage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "Peer.h"
#include "MockPieceStorage.h"
#include "DlAbortEx.h"
#include "FileEntry.h"

namespace aria2 {

class BtHaveAllMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtHaveAllMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testDoReceivedAction_goodByeSeeder);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testCreate();
  void testCreateMessage();
  void testDoReceivedAction();
  void testDoReceivedAction_goodByeSeeder();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtHaveAllMessageTest);

void BtHaveAllMessageTest::testCreate()
{
  unsigned char msg[5];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 14);
  std::shared_ptr<BtHaveAllMessage> pm(BtHaveAllMessage::create(&msg[4], 1));
  CPPUNIT_ASSERT_EQUAL((uint8_t)14, pm->getId());

  // case: payload size is wrong
  try {
    unsigned char msg[6];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 2, 14);
    BtHaveAllMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[5];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 15);
    BtHaveAllMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtHaveAllMessageTest::testCreateMessage()
{
  BtHaveAllMessage msg;
  unsigned char data[5];
  bittorrent::createPeerMessageString(data, sizeof(data), 1, 14);
  auto rawmsg = msg.createMessage();
  CPPUNIT_ASSERT_EQUAL((size_t)5, rawmsg.size());
  CPPUNIT_ASSERT(std::equal(std::begin(rawmsg), std::end(rawmsg), data));
}

void BtHaveAllMessageTest::testDoReceivedAction()
{
  BtHaveAllMessage msg;
  std::shared_ptr<Peer> peer(new Peer("host", 6969));
  peer->allocateSessionResource(16_k, 256_k);
  peer->setFastExtensionEnabled(true);
  msg.setPeer(peer);
  auto pieceStorage = make_unique<MockPieceStorage>();
  msg.setPieceStorage(pieceStorage.get());

  msg.doReceivedAction();

  CPPUNIT_ASSERT(peer->isSeeder());

  peer->setFastExtensionEnabled(false);

  try {
    msg.doReceivedAction();
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtHaveAllMessageTest::testDoReceivedAction_goodByeSeeder()
{
  BtHaveAllMessage msg;
  std::shared_ptr<Peer> peer(new Peer("ip", 6000));
  peer->allocateSessionResource(1_k, 1_k);
  peer->setFastExtensionEnabled(true);
  msg.setPeer(peer);
  auto pieceStorage = make_unique<MockPieceStorage>();
  msg.setPieceStorage(pieceStorage.get());

  pieceStorage->setDownloadFinished(true);

  try {
    msg.doReceivedAction();
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (DlAbortEx& e) {
    // success
  }
}

} // namespace aria2
