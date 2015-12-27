#include "BtPieceMessage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "MockBtMessage.h"
#include "MockBtMessageFactory.h"
#include "MockBtMessageDispatcher.h"
#include "BtChokingEvent.h"
#include "BtCancelSendingPieceEvent.h"
#include "FileEntry.h"
#include "Peer.h"
#include "Piece.h"
#include "BtHandshakeMessage.h"
#include "DownloadContext.h"
#include "BtRejectMessage.h"

namespace aria2 {

class BtPieceMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtPieceMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessageHeader);
  CPPUNIT_TEST(testChokingEvent);
  CPPUNIT_TEST(testChokingEvent_allowedFastEnabled);
  CPPUNIT_TEST(testChokingEvent_inAmAllowedIndexSet);
  CPPUNIT_TEST(testChokingEvent_invalidate);
  CPPUNIT_TEST(testCancelSendingPieceEvent);
  CPPUNIT_TEST(testCancelSendingPieceEvent_noMatch);
  CPPUNIT_TEST(testCancelSendingPieceEvent_allowedFastEnabled);
  CPPUNIT_TEST(testCancelSendingPieceEvent_invalidate);
  CPPUNIT_TEST(testToString);

  CPPUNIT_TEST_SUITE_END();

public:
  void testCreate();
  void testCreateMessageHeader();
  void testChokingEvent();
  void testChokingEvent_allowedFastEnabled();
  void testChokingEvent_inAmAllowedIndexSet();
  void testChokingEvent_invalidate();
  void testCancelSendingPieceEvent();
  void testCancelSendingPieceEvent_noMatch();
  void testCancelSendingPieceEvent_allowedFastEnabled();
  void testCancelSendingPieceEvent_invalidate();
  void testToString();

  class MockBtMessageFactory2 : public MockBtMessageFactory {
  public:
    virtual std::unique_ptr<BtRejectMessage>
    createRejectMessage(size_t index, int32_t begin,
                        int32_t length) CXX11_OVERRIDE
    {
      return make_unique<BtRejectMessage>(index, begin, length);
    }
  };

  std::unique_ptr<DownloadContext> dctx_;
  std::unique_ptr<MockBtMessageDispatcher> btMessageDispatcher;
  std::unique_ptr<MockBtMessageFactory> btMessageFactory_;
  std::shared_ptr<Peer> peer;
  std::unique_ptr<BtPieceMessage> msg;

  void setUp()
  {
    dctx_ = make_unique<DownloadContext>(16_k, 256_k, "/path/to/file");

    peer = std::make_shared<Peer>("host", 6969);
    peer->allocateSessionResource(dctx_->getPieceLength(),
                                  dctx_->getTotalLength());

    btMessageDispatcher = make_unique<MockBtMessageDispatcher>();
    btMessageFactory_ = make_unique<MockBtMessageFactory2>();

    msg = make_unique<BtPieceMessage>();
    msg->setIndex(1);
    msg->setBegin(1_k);
    msg->setBlockLength(16_k);
    msg->setDownloadContext(dctx_.get());
    msg->setPeer(peer);
    msg->setBtMessageDispatcher(btMessageDispatcher.get());
    msg->setBtMessageFactory(btMessageFactory_.get());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtPieceMessageTest);

void BtPieceMessageTest::testCreate()
{
  unsigned char msg[13 + 2];
  unsigned char data[2];
  memset(data, 0xff, sizeof(data));
  bittorrent::createPeerMessageString(msg, sizeof(msg), 11, 7);
  bittorrent::setIntParam(&msg[5], 12345);
  bittorrent::setIntParam(&msg[9], 256);
  memcpy(&msg[13], data, sizeof(data));
  std::shared_ptr<BtPieceMessage> pm(BtPieceMessage::create(&msg[4], 11));
  CPPUNIT_ASSERT_EQUAL((uint8_t)7, pm->getId());
  CPPUNIT_ASSERT_EQUAL((size_t)12345, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL(256, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL(2, pm->getBlockLength());

  // case: payload size is wrong
  try {
    unsigned char msg[13];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 9, 7);
    BtPieceMessage::create(&msg[4], 9);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[13 + 2];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 11, 8);
    BtPieceMessage::create(&msg[4], 11);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtPieceMessageTest::testCreateMessageHeader()
{
  BtPieceMessage msg;
  msg.setIndex(12345);
  msg.setBegin(256);
  msg.setBlockLength(1_k);
  unsigned char data[13];
  bittorrent::createPeerMessageString(data, sizeof(data), 9 + 1_k, 7);
  bittorrent::setIntParam(&data[5], 12345);
  bittorrent::setIntParam(&data[9], 256);
  unsigned char rawmsg[13];
  msg.createMessageHeader(rawmsg);
  CPPUNIT_ASSERT(memcmp(rawmsg, data, 13) == 0);
}

void BtPieceMessageTest::testChokingEvent()
{
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->onChokingEvent(BtChokingEvent());

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testChokingEvent_allowedFastEnabled()
{
  peer->setFastExtensionEnabled(true);

  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(peer->isFastExtensionEnabled());

  msg->onChokingEvent(BtChokingEvent());

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT_EQUAL((size_t)1, btMessageDispatcher->messageQueue.size());
  auto rej = static_cast<const BtRejectMessage*>(
      btMessageDispatcher->messageQueue.front().get());
  CPPUNIT_ASSERT_EQUAL((size_t)1, rej->getIndex());
  CPPUNIT_ASSERT_EQUAL((int32_t)1_k, rej->getBegin());
  CPPUNIT_ASSERT_EQUAL((int32_t)16_k, rej->getLength());
}

void BtPieceMessageTest::testChokingEvent_inAmAllowedIndexSet()
{
  peer->setFastExtensionEnabled(true);
  peer->addAmAllowedIndex(1);

  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(peer->isFastExtensionEnabled());

  msg->onChokingEvent(BtChokingEvent());

  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testChokingEvent_invalidate()
{
  msg->setInvalidate(true);
  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->onChokingEvent(BtChokingEvent());

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testCancelSendingPieceEvent()
{
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(1, 1_k, 16_k));

  CPPUNIT_ASSERT(msg->isInvalidate());
}

void BtPieceMessageTest::testCancelSendingPieceEvent_noMatch()
{
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(0, 1_k, 16_k));

  CPPUNIT_ASSERT(!msg->isInvalidate());

  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(1, 0, 16_k));

  CPPUNIT_ASSERT(!msg->isInvalidate());

  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(1, 1_k, 0));

  CPPUNIT_ASSERT(!msg->isInvalidate());
}

void BtPieceMessageTest::testCancelSendingPieceEvent_allowedFastEnabled()
{
  peer->setFastExtensionEnabled(true);
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(peer->isFastExtensionEnabled());

  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(1, 1_k, 16_k));

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT_EQUAL((size_t)1, btMessageDispatcher->messageQueue.size());
  auto rej = static_cast<const BtRejectMessage*>(
      btMessageDispatcher->messageQueue.front().get());
  CPPUNIT_ASSERT_EQUAL((size_t)1, rej->getIndex());
  CPPUNIT_ASSERT_EQUAL((int32_t)1_k, rej->getBegin());
  CPPUNIT_ASSERT_EQUAL((int32_t)16_k, rej->getLength());
}

void BtPieceMessageTest::testCancelSendingPieceEvent_invalidate()
{
  msg->setInvalidate(true);
  peer->setFastExtensionEnabled(true);
  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT(peer->isFastExtensionEnabled());

  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(1, 1_k, 16_k));

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testToString()
{
  CPPUNIT_ASSERT_EQUAL(std::string("piece index=1, begin=1024, length=16384"),
                       msg->toString());
}

} // namespace aria2
