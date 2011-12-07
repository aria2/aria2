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

namespace aria2 {

class BtPieceMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtPieceMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessageHeader);
  CPPUNIT_TEST(testChokingEvent);
  CPPUNIT_TEST(testChokingEvent_allowedFastEnabled);
  CPPUNIT_TEST(testChokingEvent_inAmAllowedIndexSet);
  CPPUNIT_TEST(testChokingEvent_invalidate);
  CPPUNIT_TEST(testChokingEvent_sendingInProgress);
  CPPUNIT_TEST(testCancelSendingPieceEvent);
  CPPUNIT_TEST(testCancelSendingPieceEvent_noMatch);
  CPPUNIT_TEST(testCancelSendingPieceEvent_allowedFastEnabled);
  CPPUNIT_TEST(testCancelSendingPieceEvent_invalidate);
  CPPUNIT_TEST(testCancelSendingPieceEvent_sendingInProgress);
  CPPUNIT_TEST(testToString);

  CPPUNIT_TEST_SUITE_END();
public:
  void testCreate();
  void testCreateMessageHeader();
  void testChokingEvent();
  void testChokingEvent_allowedFastEnabled();
  void testChokingEvent_inAmAllowedIndexSet();
  void testChokingEvent_invalidate();
  void testChokingEvent_sendingInProgress();
  void testCancelSendingPieceEvent();
  void testCancelSendingPieceEvent_noMatch();
  void testCancelSendingPieceEvent_allowedFastEnabled();
  void testCancelSendingPieceEvent_invalidate();
  void testCancelSendingPieceEvent_sendingInProgress();
  void testToString();

  class MockBtMessage2 : public MockBtMessage {
  public:
    size_t index;
    uint32_t begin;
    size_t length;
  public:
    MockBtMessage2(size_t index, uint32_t begin, size_t length):index(index), begin(begin), length(length) {}

  };

  class MockBtMessageFactory2 : public MockBtMessageFactory {
  public:
    virtual SharedHandle<BtMessage>
    createRejectMessage(size_t index,
                        int32_t begin,
                        int32_t length) {
      SharedHandle<MockBtMessage2> msg(new MockBtMessage2(index, begin, length));
      return msg;
    }
  };

  SharedHandle<DownloadContext> dctx_;
  SharedHandle<MockBtMessageDispatcher> btMessageDispatcher;
  SharedHandle<MockBtMessageFactory> btMessageFactory_;
  SharedHandle<Peer> peer;
  SharedHandle<BtPieceMessage> msg;

  void setUp() {
    dctx_.reset(new DownloadContext(16*1024, 256*1024, "/path/to/file"));

    peer.reset(new Peer("host", 6969));
    peer->allocateSessionResource(dctx_->getPieceLength(),
                                  dctx_->getTotalLength());

    btMessageDispatcher.reset(new MockBtMessageDispatcher());
    btMessageFactory_.reset(new MockBtMessageFactory2());

    msg.reset(new BtPieceMessage());
    msg->setIndex(1);
    msg->setBegin(1024);
    msg->setBlockLength(16*1024);
    msg->setDownloadContext(dctx_);
    msg->setPeer(peer);
    msg->setBtMessageDispatcher(btMessageDispatcher.get());
    msg->setBtMessageFactory(btMessageFactory_.get());
  }
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtPieceMessageTest);

void BtPieceMessageTest::testCreate() {
  unsigned char msg[13+2];
  unsigned char data[2];
  memset(data, 0xff, sizeof(data));
  bittorrent::createPeerMessageString(msg, sizeof(msg), 11, 7);
  bittorrent::setIntParam(&msg[5], 12345);
  bittorrent::setIntParam(&msg[9], 256);
  memcpy(&msg[13], data, sizeof(data));
  SharedHandle<BtPieceMessage> pm = BtPieceMessage::create(&msg[4], 11);
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
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[13+2];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 11, 8);
    BtPieceMessage::create(&msg[4], 11);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
}

void BtPieceMessageTest::testCreateMessageHeader() {
  BtPieceMessage msg;
  msg.setIndex(12345);
  msg.setBegin(256);
  msg.setBlockLength(1024);
  unsigned char data[13];
  bittorrent::createPeerMessageString(data, sizeof(data), 9+1024, 7);
  bittorrent::setIntParam(&data[5], 12345);
  bittorrent::setIntParam(&data[9], 256);
  unsigned char* rawmsg = msg.createMessageHeader();
  CPPUNIT_ASSERT(memcmp(rawmsg, data, 13) == 0);
  delete [] rawmsg;
}

void BtPieceMessageTest::testChokingEvent() {
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->onChokingEvent(BtChokingEvent());

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testChokingEvent_allowedFastEnabled() {
  peer->setFastExtensionEnabled(true);

  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(peer->isFastExtensionEnabled());

  msg->onChokingEvent(BtChokingEvent());

  CPPUNIT_ASSERT(msg->isInvalidate());  
  CPPUNIT_ASSERT_EQUAL((size_t)1, btMessageDispatcher->messageQueue.size());
  SharedHandle<MockBtMessage2> rej =
    dynamic_pointer_cast<MockBtMessage2>
    (btMessageDispatcher->messageQueue.front());
  CPPUNIT_ASSERT_EQUAL((size_t)1, rej->index);
  CPPUNIT_ASSERT_EQUAL((uint32_t)1024, rej->begin);
  CPPUNIT_ASSERT_EQUAL((size_t)16*1024, rej->length);
}

void BtPieceMessageTest::testChokingEvent_inAmAllowedIndexSet() {
  peer->setFastExtensionEnabled(true);
  peer->addAmAllowedIndex(1);

  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(peer->isFastExtensionEnabled());

  msg->onChokingEvent(BtChokingEvent());

  CPPUNIT_ASSERT(!msg->isInvalidate());  
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testChokingEvent_invalidate() {
  msg->setInvalidate(true);
  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->onChokingEvent(BtChokingEvent());

  CPPUNIT_ASSERT(msg->isInvalidate());  
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testChokingEvent_sendingInProgress() {
  msg->setSendingInProgress(true);
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->onChokingEvent(BtChokingEvent());

  CPPUNIT_ASSERT(!msg->isInvalidate());  
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testCancelSendingPieceEvent() {
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(1, 1024, 16*1024));

  CPPUNIT_ASSERT(msg->isInvalidate());
}

void BtPieceMessageTest::testCancelSendingPieceEvent_noMatch() {
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(0, 1024, 16*1024));

  CPPUNIT_ASSERT(!msg->isInvalidate());
  
  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(1, 0, 16*1024));

  CPPUNIT_ASSERT(!msg->isInvalidate());

  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(1, 1024, 0));

  CPPUNIT_ASSERT(!msg->isInvalidate());
}

void BtPieceMessageTest::testCancelSendingPieceEvent_allowedFastEnabled() {
  peer->setFastExtensionEnabled(true);
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(peer->isFastExtensionEnabled());

  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(1, 1024, 16*1024));

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT_EQUAL((size_t)1, btMessageDispatcher->messageQueue.size());
  SharedHandle<MockBtMessage2> rej =
    dynamic_pointer_cast<MockBtMessage2>
    (btMessageDispatcher->messageQueue.front());
  CPPUNIT_ASSERT_EQUAL((size_t)1, rej->index);
  CPPUNIT_ASSERT_EQUAL((uint32_t)1024, rej->begin);
  CPPUNIT_ASSERT_EQUAL((size_t)16*1024, rej->length);
}

void BtPieceMessageTest::testCancelSendingPieceEvent_invalidate() {
  msg->setInvalidate(true);
  peer->setFastExtensionEnabled(true);
  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(peer->isFastExtensionEnabled());

  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(1, 1024, 16*1024));

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testCancelSendingPieceEvent_sendingInProgress() {
  msg->setSendingInProgress(true);
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->onCancelSendingPieceEvent(BtCancelSendingPieceEvent(1, 1024, 16*1024));

  CPPUNIT_ASSERT(!msg->isInvalidate());
}

void BtPieceMessageTest::testToString() {
  CPPUNIT_ASSERT_EQUAL(std::string("piece index=1, begin=1024, length=16384"),
                       msg->toString());
}

} // namespace aria2
