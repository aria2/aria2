#include "BtPieceMessage.h"
#include "PeerMessageUtil.h"
#include "MockBtContext.h"
#include "MockBtMessage.h"
#include "MockBtMessageFactory.h"
#include "MockBtMessageDispatcher.h"
#include "BtChokingEvent.h"
#include "BtCancelSendingPieceEvent.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtPieceMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtPieceMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessageHeader);
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
  BtPieceMessageTest():btMessageDispatcher(0), peer(0), msg(0) {}

  void testCreate();
  void testGetMessageHeader();
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
    int32_t index;
    int32_t begin;
    int32_t length;
  public:
    MockBtMessage2(int32_t index, int32_t begin, int32_t length):index(index), begin(begin), length(length) {}

  };

  typedef SharedHandle<MockBtMessage2> MockBtMessage2Handle;

  class MockBtMessageFactory2 : public MockBtMessageFactory {
  public:
    virtual BtMessageHandle createRejectMessage(int32_t index,
						int32_t begin,
						int32_t length) {
      MockBtMessage2Handle msg = new MockBtMessage2(index, begin, length);
      return msg;
    }
  };

  typedef SharedHandle<MockBtMessageFactory2> MockBtMessageFactory2Handle;

  MockBtMessageDispatcherHandle btMessageDispatcher;
  PeerHandle peer;
  BtPieceMessageHandle msg;

  void setUp() {
    BtRegistry::clear();

    MockBtContextHandle btContext;
    btContext->setInfoHash((const unsigned char*)"12345678901234567890");
    btContext->setPieceLength(16*1024);
    btContext->setTotalLength(256*1024);

    peer = new Peer("host", 6969, 16*1024, 256*1024);
    BtRegistry::registerPeerObjectCluster(btContext->getInfoHashAsString(),
					  new PeerObjectCluster());
    PEER_OBJECT_CLUSTER(btContext)->registerHandle(peer->getId(), new PeerObject());
    btMessageDispatcher = new MockBtMessageDispatcher();
    PEER_OBJECT(btContext, peer)->btMessageDispatcher = btMessageDispatcher;
    PEER_OBJECT(btContext, peer)->btMessageFactory = new MockBtMessageFactory2();

    msg = new BtPieceMessage();
    msg->setIndex(1);
    msg->setBegin(1024);
    msg->setBlockLength(16*1024);
    msg->setBtContext(btContext);
    msg->setPeer(peer);
    msg->setBtMessageDispatcher(btMessageDispatcher);
    msg->setBtMessageFactory(BT_MESSAGE_FACTORY(btContext, peer));
  }
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtPieceMessageTest);

void BtPieceMessageTest::testCreate() {
  unsigned char msg[13+2];
  unsigned char data[2];
  memset(data, 0xff, sizeof(data));
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 11, 7);
  PeerMessageUtil::setIntParam(&msg[5], 12345);
  PeerMessageUtil::setIntParam(&msg[9], 256);
  memcpy(&msg[13], data, sizeof(data));
  BtPieceMessageHandle pm = BtPieceMessage::create(&msg[4], 11);
  CPPUNIT_ASSERT_EQUAL((int8_t)7, pm->getId());
  CPPUNIT_ASSERT_EQUAL((int32_t)12345, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL((int32_t)256, pm->getBegin());
  CPPUNIT_ASSERT(memcmp(data, pm->getBlock(), sizeof(data)) == 0);
  CPPUNIT_ASSERT_EQUAL((int32_t)2, pm->getBlockLength());

  // case: payload size is wrong
  try {
    unsigned char msg[13];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 9, 7);
    BtPieceMessage::create(&msg[4], 9);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[13+2];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 11, 8);
    BtPieceMessage::create(&msg[4], 11);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
}

void BtPieceMessageTest::testGetMessageHeader() {
  BtPieceMessage msg;
  msg.setIndex(12345);
  msg.setBegin(256);
  msg.setBlockLength(1024);
  unsigned char data[13];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 9+1024, 7);
  PeerMessageUtil::setIntParam(&data[5], 12345);
  PeerMessageUtil::setIntParam(&data[9], 256);
  CPPUNIT_ASSERT(memcmp(msg.getMessageHeader(), data, 13) == 0);
}

void BtPieceMessageTest::testChokingEvent() {
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->handleEvent(new BtChokingEvent());

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testChokingEvent_allowedFastEnabled() {
  peer->setFastExtensionEnabled(true);

  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(peer->isFastExtensionEnabled());

  msg->handleEvent(new BtChokingEvent());

  CPPUNIT_ASSERT(msg->isInvalidate());  
  CPPUNIT_ASSERT_EQUAL((size_t)1, btMessageDispatcher->messageQueue.size());
  MockBtMessage2* rej = (MockBtMessage2*)btMessageDispatcher->messageQueue.front().get();
  CPPUNIT_ASSERT_EQUAL((int32_t)1, rej->index);
  CPPUNIT_ASSERT_EQUAL((int32_t)1024, rej->begin);
  CPPUNIT_ASSERT_EQUAL((int32_t)16*1024, rej->length);
}

void BtPieceMessageTest::testChokingEvent_inAmAllowedIndexSet() {
  peer->setFastExtensionEnabled(true);
  peer->addAmAllowedIndex(1);

  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(peer->isFastExtensionEnabled());

  msg->handleEvent(new BtChokingEvent());

  CPPUNIT_ASSERT(!msg->isInvalidate());  
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testChokingEvent_invalidate() {
  msg->setInvalidate(true);
  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->handleEvent(new BtChokingEvent());

  CPPUNIT_ASSERT(msg->isInvalidate());  
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testChokingEvent_sendingInProgress() {
  msg->setSendingInProgress(true);
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->handleEvent(new BtChokingEvent());

  CPPUNIT_ASSERT(!msg->isInvalidate());  
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testCancelSendingPieceEvent() {
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->handleEvent(new BtCancelSendingPieceEvent(1, 1024, 16*1024));

  CPPUNIT_ASSERT(msg->isInvalidate());
}

void BtPieceMessageTest::testCancelSendingPieceEvent_noMatch() {
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->handleEvent(new BtCancelSendingPieceEvent(0, 1024, 16*1024));

  CPPUNIT_ASSERT(!msg->isInvalidate());

  msg->handleEvent(new BtCancelSendingPieceEvent(1, 0, 16*1024));

  CPPUNIT_ASSERT(!msg->isInvalidate());

  msg->handleEvent(new BtCancelSendingPieceEvent(1, 1024, 0));

  CPPUNIT_ASSERT(!msg->isInvalidate());
}

void BtPieceMessageTest::testCancelSendingPieceEvent_allowedFastEnabled() {
  peer->setFastExtensionEnabled(true);
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(peer->isFastExtensionEnabled());

  msg->handleEvent(new BtCancelSendingPieceEvent(1, 1024, 16*1024));

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT_EQUAL((size_t)1, btMessageDispatcher->messageQueue.size());
  MockBtMessage2* rej = (MockBtMessage2*)btMessageDispatcher->messageQueue.front().get();
  CPPUNIT_ASSERT_EQUAL((int32_t)1, rej->index);
  CPPUNIT_ASSERT_EQUAL((int32_t)1024, rej->begin);
  CPPUNIT_ASSERT_EQUAL((int32_t)16*1024, rej->length);
}

void BtPieceMessageTest::testCancelSendingPieceEvent_invalidate() {
  msg->setInvalidate(true);
  peer->setFastExtensionEnabled(true);
  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  CPPUNIT_ASSERT(peer->isFastExtensionEnabled());

  msg->handleEvent(new BtCancelSendingPieceEvent(1, 1024, 16*1024));

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT_EQUAL((size_t)0, btMessageDispatcher->messageQueue.size());
}

void BtPieceMessageTest::testCancelSendingPieceEvent_sendingInProgress() {
  msg->setSendingInProgress(true);
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(msg->isSendingInProgress());
  CPPUNIT_ASSERT(!peer->isFastExtensionEnabled());

  msg->handleEvent(new BtCancelSendingPieceEvent(1, 1024, 16*1024));

  CPPUNIT_ASSERT(!msg->isInvalidate());
}

void BtPieceMessageTest::testToString() {
  CPPUNIT_ASSERT_EQUAL(string("piece index=1, begin=1024, length=16384"),
		 msg->toString());
}
