#include "BtRequestMessage.h"
#include "PeerMessageUtil.h"
#include "MockBtContext.h"
#include "MockBtMessage.h"
#include "MockPieceStorage.h"
#include "MockBtMessageFactory.h"
#include "MockBtMessageDispatcher.h"
#include "DefaultBtContext.h"
#include "BtAbortOutstandingRequestEvent.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

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
    virtual bool hasPiece(int index) {
      return index == 1;
    }
  };

  class MockBtMessage2 : public MockBtMessage {
  public:
    string type;
    int32_t index;
    int32_t begin;
    uint32_t length;
  public:
    MockBtMessage2(string type, int32_t index, int32_t begin, uint32_t length):type(type), index(index), begin(begin), length(length) {}
  };

  typedef SharedHandle<MockBtMessage2> MockBtMessage2Handle;

  class MockBtMessageFactory2 : public MockBtMessageFactory {
  public:
    virtual BtMessageHandle
    createPieceMessage(int32_t index, int32_t begin, uint32_t length) {
      MockBtMessage2Handle btMsg = new MockBtMessage2("piece", index, begin, length);
      return btMsg;
    }

    virtual BtMessageHandle
    createRejectMessage(int32_t index, int32_t begin, uint32_t length) {
      MockBtMessage2Handle btMsg = new MockBtMessage2("reject", index, begin, length);
      return btMsg;
    }
  };

  typedef SharedHandle<MockBtMessageFactory2> MockBtMessageFactory2Handle;

  PeerHandle peer;
  MockBtMessageDispatcherHandle dispatcher;
  BtRequestMessageHandle msg;

  BtRequestMessageTest():peer(0), dispatcher(0), msg(0) {}

  void setUp() {
    BtRegistry::clear();

    MockBtContextHandle btContext = new MockBtContext();
    btContext->setInfoHash((const unsigned char*)"12345678901234567890");
    btContext->setPieceLength(16*1024);
    btContext->setTotalLength(256*1024);

    MockPieceStorageHandle pieceStorage = new MockPieceStorage2();

    BtRegistry::registerPieceStorage(btContext->getInfoHashAsString(),
				     pieceStorage);

    peer = new Peer("host", 6969, btContext->getPieceLength(), btContext->getTotalLength());

    BtRegistry::registerPeerObjectCluster(btContext->getInfoHashAsString(),
					  new PeerObjectCluster());
    PEER_OBJECT_CLUSTER(btContext)->registerHandle(peer->getId(), new PeerObject());

    dispatcher = new MockBtMessageDispatcher();

    PEER_OBJECT(btContext, peer)->btMessageDispatcher = dispatcher;
    PEER_OBJECT(btContext, peer)->btMessageFactory = new MockBtMessageFactory2();

    msg = new BtRequestMessage();
    msg->setBtContext(btContext);
    msg->setPeer(peer);
    msg->setIndex(1);
    msg->setBegin(16);
    msg->setLength(32);
    msg->setBlockIndex(2);
  }
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtRequestMessageTest);

void BtRequestMessageTest::testCreate() {
  unsigned char msg[17];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 13, 6);
  PeerMessageUtil::setIntParam(&msg[5], 12345);
  PeerMessageUtil::setIntParam(&msg[9], 256);
  PeerMessageUtil::setIntParam(&msg[13], 1024);
  BtRequestMessageHandle pm = BtRequestMessage::create(&msg[4], 13);
  CPPUNIT_ASSERT_EQUAL((uint8_t)6, pm->getId());
  CPPUNIT_ASSERT_EQUAL(12345, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL(256, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL((uint32_t)1024, pm->getLength());

  // case: payload size is wrong
  try {
    unsigned char msg[18];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 14, 6);
    BtRequestMessage::create(&msg[4], 14);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[17];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 13, 7);
    BtRequestMessage::create(&msg[4], 13);
    CPPUNIT_FAIL("exception must be threw.");
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
  peer->amChoking = false;
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher->messageQueue.size());
  MockBtMessage2* pieceMsg = (MockBtMessage2*)dispatcher->messageQueue.front().get();
  CPPUNIT_ASSERT_EQUAL(string("piece"), pieceMsg->type);
  CPPUNIT_ASSERT_EQUAL(1, pieceMsg->index);
  CPPUNIT_ASSERT_EQUAL(16, pieceMsg->begin);
  CPPUNIT_ASSERT_EQUAL((uint32_t)32, pieceMsg->length);
}

void BtRequestMessageTest::testDoReceivedAction_hasPieceAndAmChokingAndFastExtensionEnabled() {
  peer->amChoking = true;
  peer->setFastExtensionEnabled(true);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher->messageQueue.size());
  MockBtMessage2* pieceMsg = (MockBtMessage2*)dispatcher->messageQueue.front().get();
  CPPUNIT_ASSERT_EQUAL(string("reject"), pieceMsg->type);
  CPPUNIT_ASSERT_EQUAL(1, pieceMsg->index);
  CPPUNIT_ASSERT_EQUAL(16, pieceMsg->begin);
  CPPUNIT_ASSERT_EQUAL((uint32_t)32, pieceMsg->length);
}

void BtRequestMessageTest::testDoReceivedAction_hasPieceAndAmChokingAndFastExtensionDisabled() {
  peer->amChoking = true;
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)0, dispatcher->messageQueue.size());
}

void BtRequestMessageTest::testDoReceivedAction_doesntHavePieceAndFastExtensionEnabled() {
  msg->setIndex(2);
  peer->amChoking = false;
  peer->setFastExtensionEnabled(true);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher->messageQueue.size());
  MockBtMessage2* pieceMsg = (MockBtMessage2*)dispatcher->messageQueue.front().get();
  CPPUNIT_ASSERT_EQUAL(string("reject"), pieceMsg->type);
  CPPUNIT_ASSERT_EQUAL(2, pieceMsg->index);
  CPPUNIT_ASSERT_EQUAL(16, pieceMsg->begin);
  CPPUNIT_ASSERT_EQUAL((uint32_t)32, pieceMsg->length);
}

void BtRequestMessageTest::testDoReceivedAction_doesntHavePieceAndFastExtensionDisabled() {
  msg->setIndex(2);
  peer->amChoking = false;
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)0, dispatcher->messageQueue.size());
}

void BtRequestMessageTest::testHandleAbortRequestEvent() {
  PieceHandle piece = new Piece(1, 16*1024);
  BtAbortOutstandingRequestEventHandle event =
    new BtAbortOutstandingRequestEvent(piece);

  CPPUNIT_ASSERT(!msg->isInvalidate());
  msg->handleEvent(event);

  CPPUNIT_ASSERT(msg->isInvalidate());
}

void BtRequestMessageTest::testHandleAbortRequestEvent_indexNoMatch() {
  PieceHandle piece = new Piece(2, 16*1024);
  BtAbortOutstandingRequestEventHandle event =
    new BtAbortOutstandingRequestEvent(piece);

  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  msg->handleEvent(event);
  CPPUNIT_ASSERT(!msg->isInvalidate());
}

void BtRequestMessageTest::testHandleAbortRequestEvent_alreadyInvalidated() {
  PieceHandle piece = new Piece(1, 16*1024);
  BtAbortOutstandingRequestEventHandle event =
    new BtAbortOutstandingRequestEvent(piece);
  msg->setInvalidate(true);

  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  msg->handleEvent(event);
  CPPUNIT_ASSERT(msg->isInvalidate());
}

void BtRequestMessageTest::testHandleAbortRequestEvent_sendingInProgress() {
  PieceHandle piece = new Piece(1, 16*1024);
  BtAbortOutstandingRequestEventHandle event =
    new BtAbortOutstandingRequestEvent(piece);
  msg->setSendingInProgress(true);

  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(msg->isSendingInProgress());
  msg->handleEvent(event);
  CPPUNIT_ASSERT(!msg->isInvalidate());
}

void BtRequestMessageTest::testToString() {
  CPPUNIT_ASSERT_EQUAL(string("request index=1, begin=16, length=32"),
		       msg->toString());
}
