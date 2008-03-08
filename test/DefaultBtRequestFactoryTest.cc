#include "DefaultBtRequestFactory.h"
#include "MockBtMessage.h"
#include "MockBtMessageFactory.h"
#include "MockBtMessageDispatcher.h"
#include "MockBtContext.h"
#include "BtRegistry.h"
#include "MockPieceStorage.h"
#include "Peer.h"
#include "PeerObject.h"
#include "BtMessageReceiver.h"
#include "PeerConnection.h"
#include "ExtensionMessageFactory.h"
#include "FileEntry.h"
#include <algorithm>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DefaultBtRequestFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtRequestFactoryTest);
  CPPUNIT_TEST(testAddTargetPiece);
  CPPUNIT_TEST(testRemoveCompletedPiece);
  CPPUNIT_TEST(testCreateRequestMessages);
  CPPUNIT_TEST(testCreateRequestMessages_onEndGame);
  CPPUNIT_TEST(testRemoveTargetPiece);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<DefaultBtRequestFactory> btRequestFactory;
  SharedHandle<MockBtContext> btContext;
public:
  DefaultBtRequestFactoryTest():btRequestFactory(0), btContext(0) {}

  void testAddTargetPiece();
  void testRemoveCompletedPiece();
  void testCreateRequestMessages();
  void testCreateRequestMessages_onEndGame();
  void testRemoveTargetPiece();

  class MockBtRequestMessage : public MockBtMessage {
  public:
    size_t index;
    size_t blockIndex;

    MockBtRequestMessage(size_t index, size_t blockIndex):index(index), blockIndex(blockIndex) {}
  };
  
  typedef SharedHandle<MockBtRequestMessage> MockBtRequestMessageHandle;
  
  class MockBtMessageFactory2 : public MockBtMessageFactory {
  public:
    virtual SharedHandle<BtMessage>
    createRequestMessage(const SharedHandle<Piece>& piece, int32_t blockIndex) {
      return new MockBtRequestMessage(piece->getIndex(), blockIndex);
    }
  };

  class MockBtMessageDispatcher2 : public MockBtMessageDispatcher {
  public:
    virtual bool isOutstandingRequest(int32_t index, int32_t blockIndex) {
      return index == 0 && blockIndex == 0;
    }
  };

  class SortMockBtRequestMessage {
  public:
    bool operator()(const SharedHandle<MockBtRequestMessage>& a,
		    const SharedHandle<MockBtRequestMessage>& b) {
      if(a->index < b->index) {
	return true;
      } else if(b->index < a->index) {
	return false;
      } else if(a->blockIndex < b->blockIndex) {
	return true;
      } else if(b->blockIndex < a->blockIndex) {
	return false;
      } else {
	return true;
      }
    }
  };

  void setUp() {
    BtRegistry::unregisterAll();

    btContext = new MockBtContext();
    btContext->setInfoHash((const unsigned char*)"12345678901234567890");
    btContext->setPieceLength(16*1024);
    btContext->setTotalLength(256*1024);

    BtRegistry::registerPieceStorage(btContext->getInfoHashAsString(),
				     new MockPieceStorage());

    SharedHandle<Peer> peer = new Peer("host", 6969);

    BtRegistry::registerPeerObjectCluster(btContext->getInfoHashAsString(),
					  new PeerObjectCluster());
    SharedHandle<PeerObject> peerObject = new PeerObject();
    peerObject->btMessageFactory = new MockBtMessageFactory2();
    PEER_OBJECT_CLUSTER(btContext)->registerHandle(peer->getID(), peerObject);

    btRequestFactory = new DefaultBtRequestFactory();
    btRequestFactory->setBtContext(btContext);
    btRequestFactory->setPeer(peer);
    btRequestFactory->setBtMessageDispatcher(new MockBtMessageDispatcher());
    btRequestFactory->setBtMessageFactory(peerObject->btMessageFactory);
  }
  
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtRequestFactoryTest);

void DefaultBtRequestFactoryTest::testAddTargetPiece() {
  SharedHandle<Piece> piece = new Piece(0, 16*1024);
  btRequestFactory->addTargetPiece(piece);
  CPPUNIT_ASSERT_EQUAL((int32_t)1, btRequestFactory->countTargetPiece());
}

void DefaultBtRequestFactoryTest::testRemoveCompletedPiece() {
  SharedHandle<Piece> piece1 = new Piece(0, 16*1024);
  SharedHandle<Piece> piece2 = new Piece(1, 16*1024);
  piece2->setAllBlock();
  btRequestFactory->addTargetPiece(piece1);
  btRequestFactory->addTargetPiece(piece2);
  CPPUNIT_ASSERT_EQUAL((int32_t)2, btRequestFactory->countTargetPiece());
  btRequestFactory->removeCompletedPiece();
  CPPUNIT_ASSERT_EQUAL((int32_t)1, btRequestFactory->countTargetPiece());
  CPPUNIT_ASSERT_EQUAL((size_t)0, btRequestFactory->getTargetPieces().front()->getIndex());
}

void DefaultBtRequestFactoryTest::testCreateRequestMessages() {
  int PIECE_LENGTH = 16*1024*2;
  SharedHandle<Piece> piece1 = new Piece(0, PIECE_LENGTH);
  SharedHandle<Piece> piece2 = new Piece(1, PIECE_LENGTH);
  btRequestFactory->addTargetPiece(piece1);
  btRequestFactory->addTargetPiece(piece2);

  std::deque<SharedHandle<BtMessage> > msgs = btRequestFactory->createRequestMessages(3);

  CPPUNIT_ASSERT_EQUAL((size_t)3, msgs.size());
  std::deque<SharedHandle<BtMessage> >::iterator itr = msgs.begin();
  MockBtRequestMessage* msg = (MockBtRequestMessage*)itr->get();
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->blockIndex);
  ++itr;
  msg = (MockBtRequestMessage*)itr->get();
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->blockIndex);
  ++itr;
  msg = (MockBtRequestMessage*)itr->get();
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->blockIndex);

  CPPUNIT_ASSERT_EQUAL((size_t)1, btRequestFactory->createRequestMessages(3).size());
}

void DefaultBtRequestFactoryTest::testCreateRequestMessages_onEndGame() {
  MockBtMessageDispatcher2* dispatcher = new MockBtMessageDispatcher2();

  btRequestFactory->setBtMessageDispatcher(dispatcher);

  int PIECE_LENGTH = 16*1024*2;
  SharedHandle<Piece> piece1 = new Piece(0, PIECE_LENGTH);
  SharedHandle<Piece> piece2 = new Piece(1, PIECE_LENGTH);
  btRequestFactory->addTargetPiece(piece1);
  btRequestFactory->addTargetPiece(piece2);

  std::deque<SharedHandle<BtMessage> > msgs = btRequestFactory->createRequestMessagesOnEndGame(3);

  std::sort(msgs.begin(), msgs.end(), SortMockBtRequestMessage());

  CPPUNIT_ASSERT_EQUAL((size_t)3, msgs.size());
  std::deque<SharedHandle<BtMessage> >::iterator itr = msgs.begin();
  MockBtRequestMessage* msg = (MockBtRequestMessage*)itr->get();
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->blockIndex);
  ++itr;
  msg = (MockBtRequestMessage*)itr->get();
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->blockIndex);
  ++itr;
  msg = (MockBtRequestMessage*)itr->get();
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->blockIndex);
}

void DefaultBtRequestFactoryTest::testRemoveTargetPiece() {
  SharedHandle<Piece> piece1 = new Piece(0, 16*1024);

  btRequestFactory->addTargetPiece(piece1);

  CPPUNIT_ASSERT(std::find(btRequestFactory->getTargetPieces().begin(),
			   btRequestFactory->getTargetPieces().end(),
			   piece1) != btRequestFactory->getTargetPieces().end());

  btRequestFactory->removeTargetPiece(piece1);

  CPPUNIT_ASSERT(std::find(btRequestFactory->getTargetPieces().begin(),
			   btRequestFactory->getTargetPieces().end(),
			   piece1) == btRequestFactory->getTargetPieces().end());
}

} // namespace aria2
