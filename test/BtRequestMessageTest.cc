#include "BtRequestMessage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "MockBtMessage.h"
#include "MockPieceStorage.h"
#include "MockBtMessageFactory.h"
#include "MockBtMessageDispatcher.h"
#include "BtAbortOutstandingRequestEvent.h"
#include "Peer.h"
#include "FileEntry.h"
#include "BtHandshakeMessage.h"
#include "RangeBtMessageValidator.h"
#include "DlAbortEx.h"

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
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testValidate_lengthTooLong);
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
  void testValidate();
  void testValidate_lengthTooLong();

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
    MockBtMessage2(std::string type, size_t index, uint32_t begin,
		   size_t length)
      :
      type(type), index(index), begin(begin), length(length) {}
  };

  typedef SharedHandle<MockBtMessage2> MockBtMessage2Handle;

  class MockBtMessageFactory2 : public MockBtMessageFactory {
  public:
    virtual SharedHandle<BtMessage>
    createPieceMessage(size_t index, uint32_t begin, size_t length) {
      SharedHandle<MockBtMessage2> btMsg
	(new MockBtMessage2("piece", index, begin, length));
      return btMsg;
    }

    virtual SharedHandle<BtMessage>
    createRejectMessage(size_t index, uint32_t begin, size_t length) {
      SharedHandle<MockBtMessage2> btMsg
	(new MockBtMessage2("reject", index, begin, length));
      return btMsg;
    }
  };

  typedef SharedHandle<MockBtMessageFactory2> MockBtMessageFactory2Handle;

  SharedHandle<MockPieceStorage> _pieceStorage;
  SharedHandle<Peer> _peer;
  SharedHandle<MockBtMessageDispatcher> _dispatcher;
  SharedHandle<MockBtMessageFactory> _messageFactory;
  SharedHandle<BtRequestMessage> msg;

  void setUp() {
    _pieceStorage.reset(new MockPieceStorage2());

    _peer.reset(new Peer("host", 6969));
    _peer->allocateSessionResource(16*1024, 256*1024);

    _dispatcher.reset(new MockBtMessageDispatcher());

    _messageFactory.reset(new MockBtMessageFactory2());

    msg.reset(new BtRequestMessage());
    msg->setPeer(_peer);
    msg->setIndex(1);
    msg->setBegin(16);
    msg->setLength(32);
    msg->setBlockIndex(2);
    msg->setBtMessageDispatcher(_dispatcher);
    msg->setBtMessageFactory(_messageFactory);
    msg->setPieceStorage(_pieceStorage);
  }
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtRequestMessageTest);

void BtRequestMessageTest::testCreate() {
  unsigned char msg[17];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 13, 6);
  bittorrent::setIntParam(&msg[5], 12345);
  bittorrent::setIntParam(&msg[9], 256);
  bittorrent::setIntParam(&msg[13], 1024);
  SharedHandle<BtRequestMessage> pm = BtRequestMessage::create(&msg[4], 13);
  CPPUNIT_ASSERT_EQUAL((uint8_t)6, pm->getId());
  CPPUNIT_ASSERT_EQUAL((size_t)12345, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL((uint32_t)256, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL((size_t)1024, pm->getLength());

  // case: payload size is wrong
  try {
    unsigned char msg[18];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 14, 6);
    BtRequestMessage::create(&msg[4], 14);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[17];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 13, 7);
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
  bittorrent::createPeerMessageString(data, sizeof(data), 13, 6);
  bittorrent::setIntParam(&data[5], 12345);
  bittorrent::setIntParam(&data[9], 256);
  bittorrent::setIntParam(&data[13], 1024);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 17) == 0);
}

void BtRequestMessageTest::testDoReceivedAction_hasPieceAndAmNotChoking() {
  _peer->amChoking(false);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, _dispatcher->messageQueue.size());
  MockBtMessage2* pieceMsg =
    (MockBtMessage2*)_dispatcher->messageQueue.front().get();
  CPPUNIT_ASSERT_EQUAL(std::string("piece"), pieceMsg->type);
  CPPUNIT_ASSERT_EQUAL((size_t)1, pieceMsg->index);
  CPPUNIT_ASSERT_EQUAL((uint32_t)16, pieceMsg->begin);
  CPPUNIT_ASSERT_EQUAL((size_t)32, pieceMsg->length);
}

void BtRequestMessageTest::testDoReceivedAction_hasPieceAndAmChokingAndFastExtensionEnabled() {
  _peer->amChoking(true);
  _peer->setFastExtensionEnabled(true);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, _dispatcher->messageQueue.size());
  MockBtMessage2* pieceMsg =
    (MockBtMessage2*)_dispatcher->messageQueue.front().get();
  CPPUNIT_ASSERT_EQUAL(std::string("reject"), pieceMsg->type);
  CPPUNIT_ASSERT_EQUAL((size_t)1, pieceMsg->index);
  CPPUNIT_ASSERT_EQUAL((uint32_t)16, pieceMsg->begin);
  CPPUNIT_ASSERT_EQUAL((size_t)32, pieceMsg->length);
}

void BtRequestMessageTest::testDoReceivedAction_hasPieceAndAmChokingAndFastExtensionDisabled() {
  _peer->amChoking(true);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)0, _dispatcher->messageQueue.size());
}

void BtRequestMessageTest::testDoReceivedAction_doesntHavePieceAndFastExtensionEnabled() {
  msg->setIndex(2);
  _peer->amChoking(false);
  _peer->setFastExtensionEnabled(true);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, _dispatcher->messageQueue.size());
  MockBtMessage2* pieceMsg =
    (MockBtMessage2*)_dispatcher->messageQueue.front().get();
  CPPUNIT_ASSERT_EQUAL(std::string("reject"), pieceMsg->type);
  CPPUNIT_ASSERT_EQUAL((size_t)2, pieceMsg->index);
  CPPUNIT_ASSERT_EQUAL((uint32_t)16, pieceMsg->begin);
  CPPUNIT_ASSERT_EQUAL((size_t)32, pieceMsg->length);
}

void BtRequestMessageTest::testDoReceivedAction_doesntHavePieceAndFastExtensionDisabled() {
  msg->setIndex(2);
  _peer->amChoking(false);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)0, _dispatcher->messageQueue.size());
}

void BtRequestMessageTest::testHandleAbortRequestEvent() {
  SharedHandle<Piece> piece(new Piece(1, 16*1024));
  CPPUNIT_ASSERT(!msg->isInvalidate());
  msg->onAbortOutstandingRequestEvent(BtAbortOutstandingRequestEvent(piece));
  CPPUNIT_ASSERT(msg->isInvalidate());
}

void BtRequestMessageTest::testHandleAbortRequestEvent_indexNoMatch() {
  SharedHandle<Piece> piece(new Piece(2, 16*1024));
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  msg->onAbortOutstandingRequestEvent(BtAbortOutstandingRequestEvent(piece));
  CPPUNIT_ASSERT(!msg->isInvalidate());
}

void BtRequestMessageTest::testHandleAbortRequestEvent_alreadyInvalidated() {
  SharedHandle<Piece> piece(new Piece(1, 16*1024));
  msg->setInvalidate(true);
  CPPUNIT_ASSERT(msg->isInvalidate());
  CPPUNIT_ASSERT(!msg->isSendingInProgress());
  msg->onAbortOutstandingRequestEvent(BtAbortOutstandingRequestEvent(piece));
  CPPUNIT_ASSERT(msg->isInvalidate());
}

void BtRequestMessageTest::testHandleAbortRequestEvent_sendingInProgress() {
  SharedHandle<Piece> piece(new Piece(1, 16*1024));
  msg->setSendingInProgress(true);
  CPPUNIT_ASSERT(!msg->isInvalidate());
  CPPUNIT_ASSERT(msg->isSendingInProgress());
  msg->onAbortOutstandingRequestEvent(BtAbortOutstandingRequestEvent(piece));
  CPPUNIT_ASSERT(!msg->isInvalidate());
}

void BtRequestMessageTest::testToString() {
  CPPUNIT_ASSERT_EQUAL(std::string("request index=1, begin=16, length=32"),
		       msg->toString());
}

void BtRequestMessageTest::testValidate() {
  BtRequestMessage msg(0, 0, 16*1024);
  msg.setBtMessageValidator
    (SharedHandle<BtMessageValidator>
     (new RangeBtMessageValidator(&msg, 1024, 256*1024)));
  std::deque<std::string> errors;

  msg.validate(errors);
}

void BtRequestMessageTest::testValidate_lengthTooLong() {
  BtRequestMessage msg(0, 0, 16*1024+1);
  msg.setBtMessageValidator
    (SharedHandle<BtMessageValidator>
     (new RangeBtMessageValidator(&msg, 1024, 256*1024)));
  std::deque<std::string> errors;
  try {
    msg.validate(errors);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    CPPUNIT_ASSERT_EQUAL(std::string("Length too long: 16385 > 16KB"),
			 std::string(e.what()));
  }
}

} // namespace aria2
