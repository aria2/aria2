#include "BtRequestMessage.h"
#include "PeerMessageUtil.h"
#include "MockBtContext.h"
#include "MockBtMessage.h"
#include "MockPieceStorage.h"
#include "MockBtMessageFactory.h"
#include "MockBtMessageDispatcher.h"
#include "DefaultBtContext.h"
#include "BtAbortOutstandingRequestEvent.h"
#include "Peer.h"
#include "BtRegistry.h"
#include "PeerObject.h"
#include "BtMessageReceiver.h"
#include "BtRequestFactory.h"
#include "PeerConnection.h"
#include "ExtensionMessageFactory.h"
#include "FileEntry.h"
#include "BtHandshakeMessage.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class BtRequestMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtRequestMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST(testDoReceivedAction_hasPieceAndAmNotChoking);
  CPPUNIT_TEST(testDoReceivedAction_hasPieceAndAmChokingAndFastExtensionEnabled);
  CPPUNIT_TEST(testDoReceivedAction_hasPieceAndAmChokingAndFastExtensionDisabled);
  CPPUNIT_TEST(testDoReceivedAction_doesntHavePieceAndFastExtensionEnabled);
  CPPUNIT_TEST(testDoReceivedAction_doesntHavePieceAndFastExtensionDisabled);
  CPPUNIT_TEST(testHandleAbortRequestEvent);
  CPPUNIT_TEST(testHandleAbortRequestEvent_indexNoMatch);
  CPPUNIT_TEST(testHandleAbortRequestEvent_alreadyInvalidated);
  CPPUNIT_TEST(testHandleAbortRequestEvent_sendingInProgress);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void testCreate();
  void testGetMessage();
  void testDoReceivedAction_hasPieceAndAmNotChoking();
  void testDoReceivedAction_hasPieceAndAmChokingAndFastExtensionEnabled();
  void testDoReceivedAction_hasPieceAndAmChokingAndFastExtensionDisabled();
  void testDoReceivedAction_doesntHavePieceAndFastExtensionEnabled();
  void testDoReceivedAction_doesntHavePieceAndFastExtensionDisabled();
  void testHandleAbortRequestEvent();
  void testHandleAbortRequestEvent_indexNoMatch();
  void testHandleAbortRequestEvent_alreadyInvalidated();
  void testHandleAbortRequestEvent_sendingInProgress();
  void testToString();

  class MockPieceStorage2 : public MockPieceStorage {
  public:
    virtual bool hasPiece(size_t index) {
      return index == 1;
    }
  };

  class MockBtMessage2 : public MockBtMessage {
  public:
    std::string type;
    size_t index;
    uint32_t begin;
    size_t length;
  public:
    MockBtMessage2(std::string type, size_t index, uint32_t begin, size_t length):type(type), index(index), begin(begin), length(length) {}
  };

  typedef SharedHandle<MockBtMessage2> MockBtMessage2Handle;

  class MockBtMessageFactory2 : public MockBtMessageFactory {
  public:
    virtual SharedHandle<BtMessage>
    createPieceMessage(size_t index, uint32_t begin, size_t length) {
      SharedHandle<MockBtMessage2> btMsg(new MockBtMessage2("piece", index, begin, length));
      return btMsg;
    }

    virtual SharedHandle<BtMessage>
    createRejectMessage(size_t index, uint32_t begin, size_t length) {
      SharedHandle<MockBtMessage2> btMsg(new MockBtMessage2("reject", index, begin, length));
      return btMsg;
    }
  };

  typedef SharedHandle<MockBtMessageFactory2> MockBtMessageFactory2Handle;

  SharedHandle<Peer> peer;
  SharedHandle<MockBtMessageDispatcher> dispatcher;
  SharedHandle<BtRequestMessage> msg;

  void setUp() {
    BtRegistry::unregisterAll();

    SharedHandle<MockBtContext> btContext(new MockBtContext());
    btContext->setInfoHash((const unsigned char*)"12345678901234567890");
    btContext->setPieceLength(16*1024);
    btContext->setTotalLength(256*1024);

    SharedHandle<MockPieceStorage> pieceStorage(new MockPieceStorage2());

    BtRegistry::registerPieceStorage(btContext->getInfoHashAsString(),
				     pieceStorage);

    peer.reset(new Peer("host", 6969));
    peer->allocateSessionResource(btContext->getPieceLength(),
				  btContext->getTotalLength());
    SharedHandle<PeerObjectCluster> cluster(new PeerObjectCluster());
    BtRegistry::registerPeerObjectCluster(btContext->getInfoHashAsString(),
					  cluster);
    SharedHandle<PeerObject> po(new PeerObject());
    PEER_OBJECT_CLUSTER(btContext)->registerHandle(peer->getID(), po);

    dispatcher.reset(new MockBtMessageDispatcher());

    PEER_OBJECT(btContext, peer)->btMessageDispatcher = dispatcher;
    PEER_OBJECT(btContext, peer)->btMessageFactory.reset(new MockBtMessageFactory2());

    msg.reset(new BtRequestMessage());
    msg->setBtContext(btContext);
    msg->setPeer(peer);
    msg->setIndex(1);
    msg->setBegin(16);
    msg->setLength(32);
    msg->setBlockIndex(2);
    msg->setBtMessageDispatcher(dispatcher);
    msg->setBtMessageFactory(BT_MESSAGE_FACTORY(btContext, peer));
  }
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtRequestMessageTest);

void BtRequestMessageTest::testCreate() {
  unsigned char msg[17];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 13, 6);
  PeerMessageUtil::setIntParam(&msg[5], 12345);
  PeerMessageUtil::setIntParam(&msg[9], 256);
  PeerMessageUtil::setIntParam(&msg[13], 1024);
  SharedHandle<BtRequestMessage> pm = BtRequestMessage::create(&msg[4], 13);
  CPPUNIT_ASSERT_EQUAL((uint8_t)6, pm->getId());
  CPPUNIT_ASSERT_EQUAL((size_t)12345, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL((uint32_t)256, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL((size_t)1024, pm->getLength());

  // case: payload size is wrong
  try {
    unsigned char msg[18];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 14, 6);
    BtRequestMessage::create(&msg[4], 14);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[17];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 13, 7);
    BtRequestMessage::create(&msg[4], 13);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
}

void BtRequestMessageTest::testGetMessage() {
  BtRequestMessage msg;
  msg.setIndex(12345);
  msg.setBegin(256);
  msg.setLength(1024);
  unsigned char data[17];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 13, 6);
  PeerMessageUtil::setIntParam(&data[5], 12345);
  PeerMessageUtil::setIntParam(&data[9], 256);
  PeerMessageUtil::setIntParam(&data[13], 1024);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 17) == 0);
}

void BtRequestMessageTest::testDoReceivedAction_hasPieceAndAmNotChoking() {
  peer->amChoking(false);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher->messageQueue.size());
  MockBtMessage2* pieceMsg = (MockBtMessage2*)dispatcher->messageQueue.front().get();
  CPPUNIT_ASSERT_EQUAL(std::string("piece"), pieceMsg->type);
  CPPUNIT_ASSERT_EQUAL((size_t)1, pieceMsg->index);
  CPPUNIT_ASSERT_EQUAL((uint32_t)16, pieceMsg->begin);
  CPPUNIT_ASSERT_EQUAL((size_t)32, pieceMsg->length);
}

void BtRequestMessageTest::testDoReceivedAction_hasPieceAndAmChokingAndFastExtensionEnabled() {
  peer->amChoking(true);
  peer->setFastExtensionEnabled(true);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher->messageQueue.size());
  MockBtMessage2* pieceMsg = (MockBtMessage2*)dispatcher->messageQueue.front().get();
  CPPUNIT_ASSERT_EQUAL(std::string("reject"), pieceMsg->type);
  CPPUNIT_ASSERT_EQUAL((size_t)1, pieceMsg->index);
  CPPUNIT_ASSERT_EQUAL((uint32_t)16, pieceMsg->begin);
  CPPUNIT_ASSERT_EQUAL((size_t)32, pieceMsg->length);
}

void BtRequestMessageTest::testDoReceivedAction_hasPieceAndAmChokingAndFastExtensionDisabled() {
  peer->amChoking(true);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)0, dispatcher->messageQueue.size());
}

void BtRequestMessageTest::testDoReceivedAction_doesntHavePieceAndFastExtensionEnabled() {
  msg->setIndex(2);
  peer->amChoking(false);
  peer->setFastExtensionEnabled(true);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher->messageQueue.size());
  MockBtMessage2* pieceMsg = (MockBtMessage2*)dispatcher->messageQueue.front().get();
  CPPUNIT_ASSERT_EQUAL(std::string("reject"), pieceMsg->type);
  CPPUNIT_ASSERT_EQUAL((size_t)2, pieceMsg->index);
  CPPUNIT_ASSERT_EQUAL((uint32_t)16, pieceMsg->begin);
  CPPUNIT_ASSERT_EQUAL((size_t)32, pieceMsg->length);
}

void BtRequestMessageTest::testDoReceivedAction_doesntHavePieceAndFastExtensionDisabled() {
  msg->setIndex(2);
  peer->amChoking(false);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)0, dispatcher->messageQueue.size());
}

void BtRequestMessageTest::testHandleAbortRequestEvent() {
  SharedHandle<Piece> piece(new Piece(1, 16*1024));
  SharedHandle<BtAbortOutstandingRequestEvent> event
    (new BtAbortOutstandingRequestEvent(piece));

  CPPUNIT_ASSERT(!msg->isInvalidate());
  msg->handleEvent(event);

  CPPUNIT_ASSERT(msg->isInvalidate());
}

void BtRequestMessageTest::testHandleAbortRequestEvent_indexNoMatch() {
  SharedHandle<Piece> piece(new Piece(2, 16*1024));
  SharedHandle<BtAbortOutstandingRequestEvent> event
    (new BtAbortOutstandingRequestEvent(piece));

  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  msg->handleEvent(event);
  CPPUNIT_ASSERT(!msg->isInvalidate());
}

void BtRequestMessageTest::testHandleAbortRequestEvent_alreadyInvalidated() {
  SharedHandle<Piece> piece(new Piece(1, 16*1024));
  SharedHandle<BtAbortOutstandingRequestEvent> event
    (new BtAbortOutstandingRequestEvent(piece));
  msg->setInvalidate(true);

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  msg->handleEvent(event);
  CPPUNIT_ASSERT(msg->isInvalidate());
}

void BtRequestMessageTest::testHandleAbortRequestEvent_sendingInProgress() {
  SharedHandle<Piece> piece(new Piece(1, 16*1024));
  SharedHandle<BtAbortOutstandingRequestEvent> event
    (new BtAbortOutstandingRequestEvent(piece));
  msg->setSendingInProgress(true);

  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(msg->isSendingInProgress());
  msg->handleEvent(event);
  CPPUNIT_ASSERT(!msg->isInvalidate());
}

void BtRequestMessageTest::testToString() {
  CPPUNIT_ASSERT_EQUAL(std::string("request index=1, begin=16, length=32"),
		       msg->toString());
}

} // namespace aria2
