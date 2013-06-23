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
  std::shared_ptr<Peer> peer_;
  std::shared_ptr<DefaultBtRequestFactory> requestFactory_;
  std::shared_ptr<DownloadContext> dctx_;
  std::shared_ptr<MockPieceStorage> pieceStorage_;
  std::shared_ptr<MockBtMessageFactory> messageFactory_;
  std::shared_ptr<MockBtMessageDispatcher> dispatcher_;
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

  typedef std::shared_ptr<MockBtRequestMessage> MockBtRequestMessageHandle;

  class MockBtMessageFactory2 : public MockBtMessageFactory {
  public:
    virtual std::shared_ptr<BtMessage>
    createRequestMessage(const std::shared_ptr<Piece>& piece, size_t blockIndex) {
      return std::shared_ptr<BtMessage>
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
    bool operator()(const std::shared_ptr<MockBtRequestMessage>& a,
                    const std::shared_ptr<MockBtRequestMessage>& b) {
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
    pieceStorage_.reset(new MockPieceStorage());

    peer_.reset(new Peer("host", 6969));

    messageFactory_.reset(new MockBtMessageFactory2());

    dispatcher_.reset(new MockBtMessageDispatcher());

    requestFactory_.reset(new DefaultBtRequestFactory());
    requestFactory_->setPieceStorage(pieceStorage_.get());
    requestFactory_->setPeer(peer_);
    requestFactory_->setBtMessageDispatcher(dispatcher_.get());
    requestFactory_->setBtMessageFactory(messageFactory_.get());
  }
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtRequestFactoryTest);

void DefaultBtRequestFactoryTest::testAddTargetPiece() {
  {
    std::shared_ptr<Piece> piece(new Piece(0, 16*1024*10));
    requestFactory_->addTargetPiece(piece);
    CPPUNIT_ASSERT_EQUAL((size_t)1, requestFactory_->countTargetPiece());
  }
  {
    std::shared_ptr<Piece> piece(new Piece(1, 16*1024*9));
    piece->completeBlock(0);
    requestFactory_->addTargetPiece(piece);
    CPPUNIT_ASSERT_EQUAL((size_t)2, requestFactory_->countTargetPiece());
  }
  CPPUNIT_ASSERT_EQUAL((size_t)18, requestFactory_->countMissingBlock());
}

void DefaultBtRequestFactoryTest::testRemoveCompletedPiece() {
  std::shared_ptr<Piece> piece1(new Piece(0, 16*1024));
  std::shared_ptr<Piece> piece2(new Piece(1, 16*1024));
  piece2->setAllBlock();
  requestFactory_->addTargetPiece(piece1);
  requestFactory_->addTargetPiece(piece2);
  CPPUNIT_ASSERT_EQUAL((size_t)2, requestFactory_->countTargetPiece());
  requestFactory_->removeCompletedPiece();
  CPPUNIT_ASSERT_EQUAL((size_t)1, requestFactory_->countTargetPiece());
  CPPUNIT_ASSERT_EQUAL((size_t)0,
                       requestFactory_->getTargetPieces().front()->getIndex());
}

void DefaultBtRequestFactoryTest::testCreateRequestMessages() {
  int PIECE_LENGTH = 16*1024*2;
  std::shared_ptr<Piece> piece1(new Piece(0, PIECE_LENGTH));
  std::shared_ptr<Piece> piece2(new Piece(1, PIECE_LENGTH));
  requestFactory_->addTargetPiece(piece1);
  requestFactory_->addTargetPiece(piece2);

  std::vector<std::shared_ptr<BtMessage> > msgs;
  requestFactory_->createRequestMessages(msgs, 3);

  CPPUNIT_ASSERT_EQUAL((size_t)3, msgs.size());
  std::vector<std::shared_ptr<BtMessage> >::iterator itr = msgs.begin();
  auto msg = std::dynamic_pointer_cast<MockBtRequestMessage>(*itr);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->blockIndex);
  ++itr;
  msg = std::dynamic_pointer_cast<MockBtRequestMessage>(*itr);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->blockIndex);
  ++itr;
  msg = std::dynamic_pointer_cast<MockBtRequestMessage>(*itr);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msg->index);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msg->blockIndex);

  {
    std::vector<std::shared_ptr<BtMessage> > msgs;
    requestFactory_->createRequestMessages(msgs, 3);
    CPPUNIT_ASSERT_EQUAL((size_t)1, msgs.size());
  }
}

void DefaultBtRequestFactoryTest::testCreateRequestMessages_onEndGame() {
  std::shared_ptr<MockBtMessageDispatcher2> dispatcher
    (new MockBtMessageDispatcher2());

  requestFactory_->setBtMessageDispatcher(dispatcher.get());

  int PIECE_LENGTH = 16*1024*2;
  std::shared_ptr<Piece> piece1(new Piece(0, PIECE_LENGTH));
  std::shared_ptr<Piece> piece2(new Piece(1, PIECE_LENGTH));
  requestFactory_->addTargetPiece(piece1);
  requestFactory_->addTargetPiece(piece2);

  std::vector<std::shared_ptr<BtMessage> > msgs;
  requestFactory_->createRequestMessagesOnEndGame(msgs, 3);

  std::vector<std::shared_ptr<MockBtRequestMessage> > mmsgs;
  for(std::vector<std::shared_ptr<BtMessage> >::iterator i = msgs.begin();
      i != msgs.end(); ++i) {
    mmsgs.push_back(std::dynamic_pointer_cast<MockBtRequestMessage>(*i));
  }

  std::sort(mmsgs.begin(), mmsgs.end(), SortMockBtRequestMessage());

  CPPUNIT_ASSERT_EQUAL((size_t)3, mmsgs.size());
  std::vector<std::shared_ptr<MockBtRequestMessage> >::iterator itr =mmsgs.begin();
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
  std::shared_ptr<Piece> piece1(new Piece(0, 16*1024));

  requestFactory_->addTargetPiece(piece1);

  CPPUNIT_ASSERT(std::find_if(requestFactory_->getTargetPieces().begin(),
                              requestFactory_->getTargetPieces().end(),
                              derefEqual(piece1)) !=
                 requestFactory_->getTargetPieces().end());

  requestFactory_->removeTargetPiece(piece1);

  CPPUNIT_ASSERT(std::find_if(requestFactory_->getTargetPieces().begin(),
                              requestFactory_->getTargetPieces().end(),
                              derefEqual(piece1)) ==
                 requestFactory_->getTargetPieces().end());
}

void DefaultBtRequestFactoryTest::testGetTargetPieceIndexes()
{
  std::shared_ptr<Piece> piece1(new Piece(1, 16*1024));
  std::shared_ptr<Piece> piece3(new Piece(3, 16*1024));
  std::shared_ptr<Piece> piece5(new Piece(5, 16*1024));

  requestFactory_->addTargetPiece(piece3);
  requestFactory_->addTargetPiece(piece1);
  requestFactory_->addTargetPiece(piece5);

  std::vector<size_t> indexes;
  requestFactory_->getTargetPieceIndexes(indexes);
  CPPUNIT_ASSERT_EQUAL((size_t)3, indexes.size());
  CPPUNIT_ASSERT_EQUAL((size_t)3, indexes[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)1, indexes[1]);
  CPPUNIT_ASSERT_EQUAL((size_t)5, indexes[2]);
}

} // namespace aria2
