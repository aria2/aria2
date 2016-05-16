#include "BtBitfieldMessage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "util.h"
#include "Peer.h"
#include "MockPieceStorage.h"
#include "DlAbortEx.h"
#include "FileEntry.h"

namespace aria2 {

class BtBitfieldMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtBitfieldMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testDoReceivedAction_goodByeSeeder);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testCreate();
  void testCreateMessage();
  void testDoReceivedAction();
  void testDoReceivedAction_goodByeSeeder();
  void testToString();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtBitfieldMessageTest);

void BtBitfieldMessageTest::testCreate()
{
  unsigned char msg[5 + 2];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 3, 5);
  unsigned char bitfield[2];
  memset(bitfield, 0xff, sizeof(bitfield));
  memcpy(&msg[5], bitfield, sizeof(bitfield));
  std::shared_ptr<BtBitfieldMessage> pm(BtBitfieldMessage::create(&msg[4], 3));
  CPPUNIT_ASSERT_EQUAL((uint8_t)5, pm->getId());
  CPPUNIT_ASSERT(memcmp(bitfield, pm->getBitfield(), sizeof(bitfield)) == 0);
  CPPUNIT_ASSERT_EQUAL((size_t)2, pm->getBitfieldLength());
  // case: payload size is wrong
  try {
    unsigned char msg[5];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 5);
    BtBitfieldMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[5 + 2];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 3, 6);
    BtBitfieldMessage::create(&msg[4], 3);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtBitfieldMessageTest::testCreateMessage()
{
  BtBitfieldMessage msg;
  unsigned char bitfield[2];
  memset(bitfield, 0xff, sizeof(bitfield));
  msg.setBitfield(bitfield, sizeof(bitfield));
  unsigned char data[5 + 2];
  bittorrent::createPeerMessageString(data, sizeof(data), 3, 5);
  memcpy(&data[5], bitfield, sizeof(bitfield));
  auto rawmsg = msg.createMessage();
  CPPUNIT_ASSERT_EQUAL((size_t)7, rawmsg.size());
  CPPUNIT_ASSERT(std::equal(std::begin(rawmsg), std::end(rawmsg), data));
}

void BtBitfieldMessageTest::testDoReceivedAction()
{
  std::shared_ptr<Peer> peer(new Peer("host1", 6969));
  peer->allocateSessionResource(16_k, 256_k);
  BtBitfieldMessage msg;
  msg.setPeer(peer);
  auto pieceStorage = make_unique<MockPieceStorage>();
  msg.setPieceStorage(pieceStorage.get());
  unsigned char bitfield[] = {0xff, 0xff};
  msg.setBitfield(bitfield, sizeof(bitfield));

  CPPUNIT_ASSERT_EQUAL(
      std::string("0000"),
      util::toHex(peer->getBitfield(), peer->getBitfieldLength()));
  msg.doReceivedAction();
  CPPUNIT_ASSERT_EQUAL(
      std::string("ffff"),
      util::toHex(peer->getBitfield(), peer->getBitfieldLength()));
}

void BtBitfieldMessageTest::testDoReceivedAction_goodByeSeeder()
{
  std::shared_ptr<Peer> peer(new Peer("ip", 6000));
  peer->allocateSessionResource(1_k, 1_k);
  BtBitfieldMessage msg;
  msg.setPeer(peer);
  auto pieceStorage = make_unique<MockPieceStorage>();
  msg.setPieceStorage(pieceStorage.get());
  unsigned char bitfield[] = {0x00};
  msg.setBitfield(bitfield, sizeof(bitfield));

  // peer is not seeder and client have not completed download
  msg.doReceivedAction();

  pieceStorage->setDownloadFinished(true);

  // client completed download, but peer is not seeder
  msg.doReceivedAction();

  pieceStorage->setDownloadFinished(false);
  bitfield[0] = 0x80;
  msg.setBitfield(bitfield, sizeof(bitfield));

  // peer is seeder, but client have not completed download
  msg.doReceivedAction();

  pieceStorage->setDownloadFinished(true);
  try {
    msg.doReceivedAction();
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (DlAbortEx& e) {
    // success
  }
}

void BtBitfieldMessageTest::testToString()
{
  BtBitfieldMessage msg;
  unsigned char bitfield[] = {0xff, 0xff};
  msg.setBitfield(bitfield, sizeof(bitfield));

  CPPUNIT_ASSERT_EQUAL(std::string("bitfield ffff"), msg.toString());
}

} // namespace aria2
