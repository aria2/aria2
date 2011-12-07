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
  CPPUNIT_TEST(testCreateMessage);
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
  void testCreateMessage();
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
    createPieceMessage(size_t index, int32_t begin, int32_t length) {
      SharedHandle<MockBtMessage2> btMsg
        (new MockBtMessage2("piece", index, begin, length));
      return btMsg;
    }

    virtual SharedHandle<BtMessage>
    createRejectMessage(size_t index, int32_t begin, int32_t length) {
      SharedHandle<MockBtMessage2> btMsg
        (new MockBtMessage2("reject", index, begin, length));
      return btMsg;
    }
  };

  typedef SharedHandle<MockBtMessageFactory2> MockBtMessageFactory2Handle;

  SharedHandle<MockPieceStorage> pieceStorage_;
  SharedHandle<Peer> peer_;
  SharedHandle<MockBtMessageDispatcher> dispatcher_;
  SharedHandle<MockBtMessageFactory> messageFactory_;
  SharedHandle<BtRequestMessage> msg;

  void setUp() {
    pieceStorage_.reset(new MockPieceStorage2());

    peer_.reset(new Peer("host", 6969));
    peer_->allocateSessionResource(16*1024, 256*1024);

    dispatcher_.reset(new MockBtMessageDispatcher());

    messageFactory_.reset(new MockBtMessageFactory2());

    msg.reset(new BtRequestMessage());
    msg->setPeer(peer_);
    msg->setIndex(1);
    msg->setBegin(16);
    msg->setLength(32);
    msg->setBlockIndex(2);
    msg->setBtMessageDispatcher(dispatcher_.get());
    msg->setBtMessageFactory(messageFactory_.get());
    msg->setPieceStorage(pieceStorage_);
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
  CPPUNIT_ASSERT_EQUAL(256, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL(1024, pm->getLength());

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

void BtRequestMessageTest::testCreateMessage() {
  BtRequestMessage msg;
  msg.setIndex(12345);
  msg.setBegin(256);
  msg.setLength(1024);
  unsigned char data[17];
  bittorrent::createPeerMessageString(data, sizeof(data), 13, 6);
  bittorrent::setIntParam(&data[5], 12345);
  bittorrent::setIntParam(&data[9], 256);
  bittorrent::setIntParam(&data[13], 1024);
  unsigned char* rawmsg = msg.createMessage();
  CPPUNIT_ASSERT(memcmp(rawmsg, data, 17) == 0);
  delete [] rawmsg;
}

void BtRequestMessageTest::testDoReceivedAction_hasPieceAndAmNotChoking() {
  peer_->amChoking(false);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher_->messageQueue.size());
  SharedHandle<MockBtMessage2> pieceMsg =
    dynamic_pointer_cast<MockBtMessage2>(dispatcher_->messageQueue.front());
  CPPUNIT_ASSERT_EQUAL(std::string("piece"), pieceMsg->type);
  CPPUNIT_ASSERT_EQUAL((size_t)1, pieceMsg->index);
  CPPUNIT_ASSERT_EQUAL((uint32_t)16, pieceMsg->begin);
  CPPUNIT_ASSERT_EQUAL((size_t)32, pieceMsg->length);
}

void BtRequestMessageTest::testDoReceivedAction_hasPieceAndAmChokingAndFastExtensionEnabled() {
  peer_->amChoking(true);
  peer_->setFastExtensionEnabled(true);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher_->messageQueue.size());
  SharedHandle<MockBtMessage2> pieceMsg =
    dynamic_pointer_cast<MockBtMessage2>(dispatcher_->messageQueue.front());
  CPPUNIT_ASSERT_EQUAL(std::string("reject"), pieceMsg->type);
  CPPUNIT_ASSERT_EQUAL((size_t)1, pieceMsg->index);
  CPPUNIT_ASSERT_EQUAL((uint32_t)16, pieceMsg->begin);
  CPPUNIT_ASSERT_EQUAL((size_t)32, pieceMsg->length);
}

void BtRequestMessageTest::testDoReceivedAction_hasPieceAndAmChokingAndFastExtensionDisabled() {
  peer_->amChoking(true);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)0, dispatcher_->messageQueue.size());
}

void BtRequestMessageTest::testDoReceivedAction_doesntHavePieceAndFastExtensionEnabled() {
  msg->setIndex(2);
  peer_->amChoking(false);
  peer_->setFastExtensionEnabled(true);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher_->messageQueue.size());
  SharedHandle<MockBtMessage2> pieceMsg =
    dynamic_pointer_cast<MockBtMessage2>(dispatcher_->messageQueue.front());
  CPPUNIT_ASSERT_EQUAL(std::string("reject"), pieceMsg->type);
  CPPUNIT_ASSERT_EQUAL((size_t)2, pieceMsg->index);
  CPPUNIT_ASSERT_EQUAL((uint32_t)16, pieceMsg->begin);
  CPPUNIT_ASSERT_EQUAL((size_t)32, pieceMsg->length);
}

void BtRequestMessageTest::testDoReceivedAction_doesntHavePieceAndFastExtensionDisabled() {
  msg->setIndex(2);
  peer_->amChoking(false);
  msg->doReceivedAction();
  
  CPPUNIT_ASSERT_EQUAL((size_t)0, dispatcher_->messageQueue.size());
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
  msg.validate();
}

void BtRequestMessageTest::testValidate_lengthTooLong() {
  BtRequestMessage msg(0, 0, 16*1024+1);
  msg.setBtMessageValidator
    (SharedHandle<BtMessageValidator>
     (new RangeBtMessageValidator(&msg, 1024, 256*1024)));
  try {
    msg.validate();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    CPPUNIT_ASSERT_EQUAL(std::string("Length too long: 16385 > 16KB"),
                         std::string(e.what()));
  }
}

} // namespace aria2
