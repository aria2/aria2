#include "DefaultBtRequestFactory.h"

#include <algorithm>

#include <cppunit/extensions/HelperMacros.h>

#include "MockBtMessage.h"
#include "MockBtMessageFactory.h"
#include "MockBtMessageDispatcher.h"
#include "MockPieceStorage.h"
#include "Peer.h"
#include "FileEntry.h"
#include "BtHandshakeMessage.h"
#include "DownloadContext.h"
#include "bittorrent_helper.h"

namespace aria2 {

class DefaultBtRequestFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtRequestFactoryTest);
  CPPUNIT_TEST(testAddTargetPiece);
  CPPUNIT_TEST(testRemoveCompletedPiece);
  CPPUNIT_TEST(testCreateRequestMessages);
  CPPUNIT_TEST(testCreateRequestMessages_onEndGame);
  CPPUNIT_TEST(testRemoveTargetPiece);
  CPPUNIT_TEST(testGetTargetPieceIndexes);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Peer> _peer;
  SharedHandle<DefaultBtRequestFactory> _requestFactory;
  SharedHandle<DownloadContext> _dctx;
  SharedHandle<MockPieceStorage> _pieceStorage;
  SharedHandle<MockBtMessageFactory> _messageFactory;
  SharedHandle<MockBtMessageDispatcher> _dispatcher;
public:
  void testAddTargetPiece();
  void testRemoveCompletedPiece();
  void testCreateRequestMessages();
  void testCreateRequestMessages_onEndGame();
  void testRemoveTargetPiece();
  void testGetTargetPieceIndexes();

  class MockBtRequestMessage : public MockBtMessage {
  public:
    size_t index;
    size_t blockIndex;

    MockBtRequestMessage(size_t index, size_t blockIndex):
      index(index), blockIndex(blockIndex) {}
  };
  
  typedef SharedHandle<MockBtRequestMessage> MockBtRequestMessageHandle;
  
  class MockBtMessageFactory2 : public MockBtMessageFactory {
  public:
    virtual SharedHandle<BtMessage>
    createRequestMessage(const SharedHandle<Piece>& piece, size_t blockIndex) {
      return SharedHandle<BtMessage>
	(new MockBtRequestMessage(piece->getIndex(), blockIndex));
    }
  };

  class MockBtMessageDispatcher2 : public MockBtMessageDispatcher {
  public:
    virtual bool isOutstandingRequest(size_t index, size_t blockIndex) {
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

  void setUp()
  {
    _pieceStorage.reset(new MockPieceStorage());

    _peer.reset(new Peer("host", 6969));

    _messageFactory.reset(new MockBtMessageFactory2());

    _dispatcher.reset(new MockBtMessageDispatcher());
    
    _requestFactory.reset(new DefaultBtRequestFactory());
    _requestFactory->setPieceStorage(_pieceStorage);
    _requestFactory->setPeer(_peer);
    _requestFactory->setBtMessageDispatcher(_dispatcher);
    _requestFactory->setBtMessageFactory(_messageFactory);
  }
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtRequestFactoryTest);

void DefaultBtRequestFactoryTest::testAddTargetPiece() {
  {
    SharedHandle<Piece> piece(new Piece(0, 16*1024*10));
    _requestFactory->addTargetPiece(piece);
    CPPUNIT_ASSERT_EQUAL((size_t)1, _requestFactory->countTargetPiece());
  }
  {
    SharedHandle<Piece> piece(new Piece(1, 16*1024*9));
    piece->completeBlock(0);
    _requestFactory->addTargetPiece(piece);
    CPPUNIT_ASSERT_EQUAL((size_t)2, _requestFactory->countTargetPiece());
  }
  CPPUNIT_ASSERT_EQUAL((size_t)18, _requestFactory->countMissingBlock());
}

void DefaultBtRequestFactoryTest::testRemoveCompletedPiece() {
  SharedHandle<Piece> piece1(new Piece(0, 16*1024));
  SharedHandle<Piece> piece2(new Piece(1, 16*1024));
  piece2->setAllBlock();
  _requestFactory->addTargetPiece(piece1);
  _requestFactory->addTargetPiece(piece2);
  CPPUNIT_ASSERT_EQUAL((size_t)2, _requestFactory->countTargetPiece());
  _requestFactory->removeCompletedPiece();
  CPPUNIT_ASSERT_EQUAL((size_t)1, _requestFactory->countTargetPiece());
  CPPUNIT_ASSERT_EQUAL((size_t)0,
		       _requestFactory->getTargetPieces().front()->getIndex());
}

void DefaultBtRequestFactoryTest::testCreateRequestMessages() {
  int PIECE_LENGTH = 16*1024*2;
  SharedHandle<Piece> piece1(new Piece(0, PIECE_LENGTH));
  SharedHandle<Piece> piece2(new Piece(1, PIECE_LENGTH));
  _requestFactory->addTargetPiece(piece1);
  _requestFactory->addTargetPiece(piece2);

  std::deque<SharedHandle<BtMessage> > msgs;
  _requestFactory->createRequestMessages(msgs, 3);

  CPPUNIT_ASSERT_EQUAL((size_t)3, msgs.size());
  std::deque<SharedHandle<BtMessage> >::iterator itr = msgs.begin();
  SharedHandle<MockBtRequestMessage> msg =
    dynamic_pointer_cast<MockBtRequestMessage>(*itr);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->blockIndex);
  ++itr;
  msg = dynamic_pointer_cast<MockBtRequestMessage>(*itr);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->blockIndex);
  ++itr;
  msg = dynamic_pointer_cast<MockBtRequestMessage>(*itr);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->blockIndex);

  {
    std::deque<SharedHandle<BtMessage> > msgs;
    _requestFactory->createRequestMessages(msgs, 3);
    CPPUNIT_ASSERT_EQUAL((size_t)1, msgs.size());
  }
}

void DefaultBtRequestFactoryTest::testCreateRequestMessages_onEndGame() {
  SharedHandle<MockBtMessageDispatcher2> dispatcher
    (new MockBtMessageDispatcher2());

  _requestFactory->setBtMessageDispatcher(dispatcher);

  int PIECE_LENGTH = 16*1024*2;
  SharedHandle<Piece> piece1(new Piece(0, PIECE_LENGTH));
  SharedHandle<Piece> piece2(new Piece(1, PIECE_LENGTH));
  _requestFactory->addTargetPiece(piece1);
  _requestFactory->addTargetPiece(piece2);

  std::deque<SharedHandle<BtMessage> > msgs;
  _requestFactory->createRequestMessagesOnEndGame(msgs, 3);

  std::deque<SharedHandle<MockBtRequestMessage> > mmsgs;
  for(std::deque<SharedHandle<BtMessage> >::iterator i = msgs.begin();
      i != msgs.end(); ++i) {
    mmsgs.push_back(dynamic_pointer_cast<MockBtRequestMessage>(*i));
  }

  std::sort(mmsgs.begin(), mmsgs.end(), SortMockBtRequestMessage());

  CPPUNIT_ASSERT_EQUAL((size_t)3, mmsgs.size());
  std::deque<SharedHandle<MockBtRequestMessage> >::iterator itr = mmsgs.begin();
  MockBtRequestMessage* msg = (*itr).get();
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->blockIndex);
  ++itr;
  msg = (*itr).get();
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->blockIndex);
  ++itr;
  msg = (*itr).get();
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->blockIndex);
}

void DefaultBtRequestFactoryTest::testRemoveTargetPiece() {
  SharedHandle<Piece> piece1(new Piece(0, 16*1024));

  _requestFactory->addTargetPiece(piece1);

  CPPUNIT_ASSERT(std::find(_requestFactory->getTargetPieces().begin(),
			   _requestFactory->getTargetPieces().end(),
			   piece1) !=
		 _requestFactory->getTargetPieces().end());

  _requestFactory->removeTargetPiece(piece1);

  CPPUNIT_ASSERT(std::find(_requestFactory->getTargetPieces().begin(),
			   _requestFactory->getTargetPieces().end(),
			   piece1) ==
		 _requestFactory->getTargetPieces().end());
}

void DefaultBtRequestFactoryTest::testGetTargetPieceIndexes()
{
  SharedHandle<Piece> piece1(new Piece(1, 16*1024));
  SharedHandle<Piece> piece3(new Piece(3, 16*1024));
  SharedHandle<Piece> piece5(new Piece(5, 16*1024));

  _requestFactory->addTargetPiece(piece3);
  _requestFactory->addTargetPiece(piece1);
  _requestFactory->addTargetPiece(piece5);

  std::deque<size_t> indexes;
  _requestFactory->getTargetPieceIndexes(indexes);
  CPPUNIT_ASSERT_EQUAL((size_t)3, indexes.size());
  CPPUNIT_ASSERT_EQUAL((size_t)3, indexes[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)1, indexes[1]);
  CPPUNIT_ASSERT_EQUAL((size_t)5, indexes[2]);
}

} // namespace aria2
