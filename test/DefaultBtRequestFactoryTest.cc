#include "DefaultBtRequestFactory.h"
#include "MockBtMessage.h"
#include "MockBtMessageFactory.h"
#include "MockBtMessageDispatcher.h"
#include "MockBtContext.h"
#include "BtRegistry.h"
#include "MockPieceStorage.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class DefaultBtRequestFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtRequestFactoryTest);
  CPPUNIT_TEST(testAddTargetPiece);
  CPPUNIT_TEST(testRemoveCompletedPiece);
  CPPUNIT_TEST(testCreateRequestMessages);
  CPPUNIT_TEST(testCreateRequestMessages_onEndGame);
  CPPUNIT_TEST(testRemoveTargetPiece);
  CPPUNIT_TEST_SUITE_END();
private:
  DefaultBtRequestFactoryHandle btRequestFactory;
  MockBtContextHandle btContext;
public:
  DefaultBtRequestFactoryTest():btRequestFactory(0), btContext(0) {}

  void testAddTargetPiece();
  void testRemoveCompletedPiece();
  void testCreateRequestMessages();
  void testCreateRequestMessages_onEndGame();
  void testRemoveTargetPiece();

  class MockBtRequestMessage : public MockBtMessage {
  public:
    int index;
    int blockIndex;

    MockBtRequestMessage(int index, int blockIndex):index(index), blockIndex(blockIndex) {}
  };
  
  typedef SharedHandle<MockBtRequestMessage> MockBtRequestMessageHandle;
  
  class MockBtMessageFactory2 : public MockBtMessageFactory {
  public:
    virtual BtMessageHandle
    createRequestMessage(const PieceHandle& piece, uint32_t blockIndex) {
      return new MockBtRequestMessage(piece->getIndex(), blockIndex);
    }
  };

  class MockBtMessageDispatcher2 : public MockBtMessageDispatcher {
  public:
    virtual bool isOutstandingRequest(uint32_t index, uint32_t blockIndex) {
      return index == 0 && blockIndex == 0;
    }
  };

  void setUp() {
    BtRegistry::clear();
    btContext = new MockBtContext();
    btContext->setInfoHash((const unsigned char*)"12345678901234567890");
    btContext->setPieceLength(16*1024);
    btContext->setTotalLength(256*1024);

    BtRegistry::registerPieceStorage(btContext->getInfoHashAsString(),
				     new MockPieceStorage());

    PeerHandle peer = new Peer("host", 6969, btContext->getPieceLength(), btContext->getTotalLength());

    BtRegistry::registerPeerObjectCluster(btContext->getInfoHashAsString(),
					  new PeerObjectCluster());
    PeerObjectHandle peerObject = new PeerObject();
    peerObject->btMessageFactory = new MockBtMessageFactory2();
    PEER_OBJECT_CLUSTER(btContext)->registerHandle(peer->getId(), peerObject);

    btRequestFactory = new DefaultBtRequestFactory();
    btRequestFactory->setBtContext(btContext);
    btRequestFactory->setPeer(peer);
    btRequestFactory->setBtMessageDispatcher(new MockBtMessageDispatcher());
  }
  
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtRequestFactoryTest);

void DefaultBtRequestFactoryTest::testAddTargetPiece() {
  PieceHandle piece = new Piece(0, 16*1024);
  btRequestFactory->addTargetPiece(piece);
  CPPUNIT_ASSERT_EQUAL(1, btRequestFactory->countTargetPiece());
}

void DefaultBtRequestFactoryTest::testRemoveCompletedPiece() {
  PieceHandle piece1 = new Piece(0, 16*1024);
  PieceHandle piece2 = new Piece(1, 16*1024);
  piece2->setAllBlock();
  btRequestFactory->addTargetPiece(piece1);
  btRequestFactory->addTargetPiece(piece2);
  CPPUNIT_ASSERT_EQUAL(2, btRequestFactory->countTargetPiece());
  btRequestFactory->removeCompletedPiece();
  CPPUNIT_ASSERT_EQUAL(1, btRequestFactory->countTargetPiece());
  CPPUNIT_ASSERT_EQUAL(0, btRequestFactory->getTargetPieces().front()->getIndex());
}

void DefaultBtRequestFactoryTest::testCreateRequestMessages() {
  int PIECE_LENGTH = 16*1024*2;
  PieceHandle piece1 = new Piece(0, PIECE_LENGTH);
  PieceHandle piece2 = new Piece(1, PIECE_LENGTH);
  btRequestFactory->addTargetPiece(piece1);
  btRequestFactory->addTargetPiece(piece2);

  BtMessages msgs = btRequestFactory->createRequestMessages(3);

  CPPUNIT_ASSERT_EQUAL((size_t)3, msgs.size());
  BtMessages::iterator itr = msgs.begin();
  MockBtRequestMessage* msg = (MockBtRequestMessage*)itr->get();
  CPPUNIT_ASSERT_EQUAL(0, msg->index);
  CPPUNIT_ASSERT_EQUAL(0, msg->blockIndex);
  ++itr;
  msg = (MockBtRequestMessage*)itr->get();
  CPPUNIT_ASSERT_EQUAL(0, msg->index);
  CPPUNIT_ASSERT_EQUAL(1, msg->blockIndex);
  ++itr;
  msg = (MockBtRequestMessage*)itr->get();
  CPPUNIT_ASSERT_EQUAL(1, msg->index);
  CPPUNIT_ASSERT_EQUAL(0, msg->blockIndex);

  CPPUNIT_ASSERT_EQUAL((size_t)1, btRequestFactory->createRequestMessages(3).size());
}

void DefaultBtRequestFactoryTest::testCreateRequestMessages_onEndGame() {
  MockBtMessageDispatcher2* dispatcher = new MockBtMessageDispatcher2();

  btRequestFactory->setBtMessageDispatcher(dispatcher);

  int PIECE_LENGTH = 16*1024*2;
  PieceHandle piece1 = new Piece(0, PIECE_LENGTH);
  PieceHandle piece2 = new Piece(1, PIECE_LENGTH);
  btRequestFactory->addTargetPiece(piece1);
  btRequestFactory->addTargetPiece(piece2);

  BtMessages msgs = btRequestFactory->createRequestMessagesOnEndGame(3);

  CPPUNIT_ASSERT_EQUAL((size_t)3, msgs.size());
  BtMessages::iterator itr = msgs.begin();
  MockBtRequestMessage* msg = (MockBtRequestMessage*)itr->get();
  CPPUNIT_ASSERT_EQUAL(0, msg->index);
  CPPUNIT_ASSERT_EQUAL(1, msg->blockIndex);
  ++itr;
  msg = (MockBtRequestMessage*)itr->get();
  CPPUNIT_ASSERT_EQUAL(1, msg->index);
  CPPUNIT_ASSERT_EQUAL(0, msg->blockIndex);
  ++itr;
  msg = (MockBtRequestMessage*)itr->get();
  CPPUNIT_ASSERT_EQUAL(1, msg->index);
  CPPUNIT_ASSERT_EQUAL(1, msg->blockIndex);
}

void DefaultBtRequestFactoryTest::testRemoveTargetPiece() {
  PieceHandle piece1 = new Piece(0, 16*1024);

  btRequestFactory->addTargetPiece(piece1);

  CPPUNIT_ASSERT(find(btRequestFactory->getTargetPieces().begin(),
		      btRequestFactory->getTargetPieces().end(),
		      piece1) != btRequestFactory->getTargetPieces().end());

  btRequestFactory->removeTargetPiece(piece1);

  CPPUNIT_ASSERT(find(btRequestFactory->getTargetPieces().begin(),
		      btRequestFactory->getTargetPieces().end(),
		      piece1) == btRequestFactory->getTargetPieces().end());
}
