#include "DefaultBtMessageDispatcher.h"

#include <cassert>

#include <cppunit/extensions/HelperMacros.h>

#include "util.h"
#include "Exception.h"
#include "MockBtMessage.h"
#include "MockBtMessageFactory.h"
#include "prefs.h"
#include "BtCancelSendingPieceEvent.h"
#include "BtHandshakeMessage.h"
#include "Option.h"
#include "RequestGroupMan.h"
#include "ServerStatMan.h"
#include "RequestGroup.h"
#include "DownloadContext.h"
#include "bittorrent_helper.h"
#include "PeerConnection.h"

namespace aria2 {

class DefaultBtMessageDispatcherTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtMessageDispatcherTest);
  CPPUNIT_TEST(testAddMessage);
  CPPUNIT_TEST(testSendMessages);
  CPPUNIT_TEST(testSendMessages_underUploadLimit);
  // See the comment on the definition
  // CPPUNIT_TEST(testSendMessages_overUploadLimit);
  CPPUNIT_TEST(testDoCancelSendingPieceAction);
  CPPUNIT_TEST(testCheckRequestSlotAndDoNecessaryThing);
  CPPUNIT_TEST(testCheckRequestSlotAndDoNecessaryThing_timeout);
  CPPUNIT_TEST(testCheckRequestSlotAndDoNecessaryThing_completeBlock);
  CPPUNIT_TEST(testCountOutstandingRequest);
  CPPUNIT_TEST(testIsOutstandingRequest);
  CPPUNIT_TEST(testGetOutstandingRequest);
  CPPUNIT_TEST(testRemoveOutstandingRequest);
  CPPUNIT_TEST_SUITE_END();

private:
  std::shared_ptr<DownloadContext> dctx_;
  std::shared_ptr<Peer> peer;
  std::unique_ptr<DefaultBtMessageDispatcher> btMessageDispatcher;
  std::unique_ptr<MockBtMessageFactory> messageFactory_;
  std::unique_ptr<RequestGroupMan> rgman_;
  std::shared_ptr<Option> option_;
  std::unique_ptr<RequestGroup> rg_;

public:
  void tearDown() {}

  void testAddMessage();
  void testSendMessages();
  void testSendMessages_underUploadLimit();
  void testSendMessages_overUploadLimit();
  void testDoCancelSendingPieceAction();
  void testCheckRequestSlotAndDoNecessaryThing();
  void testCheckRequestSlotAndDoNecessaryThing_timeout();
  void testCheckRequestSlotAndDoNecessaryThing_completeBlock();
  void testCountOutstandingRequest();
  void testIsOutstandingRequest();
  void testGetOutstandingRequest();
  void testRemoveOutstandingRequest();

  struct EventCheck {
    EventCheck()
        : onQueuedCalled{false}, sendCalled{false}, doCancelActionCalled{false}
    {
    }
    bool onQueuedCalled;
    bool sendCalled;
    bool doCancelActionCalled;
  };

  class MockBtMessage2 : public MockBtMessage {
  public:
    EventCheck* evcheck;
    std::string type;
    MockBtMessage2(EventCheck* evcheck = nullptr) : evcheck{evcheck} {}

    virtual void onQueued() CXX11_OVERRIDE
    {
      if (evcheck) {
        evcheck->onQueuedCalled = true;
      }
    }

    virtual void send() CXX11_OVERRIDE
    {
      if (evcheck) {
        evcheck->sendCalled = true;
      }
    }

    virtual void onCancelSendingPieceEvent(
        const BtCancelSendingPieceEvent& event) CXX11_OVERRIDE
    {
      if (evcheck) {
        evcheck->doCancelActionCalled = true;
      }
    }
  };

  class MockBtMessageFactory2 : public MockBtMessageFactory {
  public:
    virtual std::unique_ptr<BtCancelMessage>
    createCancelMessage(size_t index, int32_t begin,
                        int32_t length) CXX11_OVERRIDE
    {
      return make_unique<BtCancelMessage>(index, begin, length);
    }
  };

  void setUp()
  {
    option_ = std::make_shared<Option>();
    option_->put(PREF_DIR, ".");

    rg_ = make_unique<RequestGroup>(GroupId::create(), option_);

    dctx_ = std::make_shared<DownloadContext>();
    bittorrent::load(A2_TEST_DIR "/test.torrent", dctx_, option_);

    rg_->setDownloadContext(dctx_);

    peer = std::make_shared<Peer>("192.168.0.1", 6969);
    peer->allocateSessionResource(dctx_->getPieceLength(),
                                  dctx_->getTotalLength());
    messageFactory_ = make_unique<MockBtMessageFactory2>();

    rgman_ = make_unique<RequestGroupMan>(
        std::vector<std::shared_ptr<RequestGroup>>{}, 0, option_.get());

    btMessageDispatcher = make_unique<DefaultBtMessageDispatcher>();
    btMessageDispatcher->setPeer(peer);
    btMessageDispatcher->setDownloadContext(dctx_.get());
    btMessageDispatcher->setBtMessageFactory(messageFactory_.get());
    btMessageDispatcher->setCuid(1);
    btMessageDispatcher->setRequestGroupMan(rgman_.get());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtMessageDispatcherTest);

void DefaultBtMessageDispatcherTest::testAddMessage()
{
  auto evcheck = EventCheck{};
  auto msg = make_unique<MockBtMessage2>(&evcheck);
  btMessageDispatcher->addMessageToQueue(std::move(msg));
  CPPUNIT_ASSERT_EQUAL(true, evcheck.onQueuedCalled);
  CPPUNIT_ASSERT_EQUAL((size_t)1,
                       btMessageDispatcher->getMessageQueue().size());
}

void DefaultBtMessageDispatcherTest::testSendMessages()
{
  auto evcheck1 = EventCheck{};
  auto msg1 = make_unique<MockBtMessage2>(&evcheck1);
  msg1->setUploading(false);
  auto evcheck2 = EventCheck{};
  auto msg2 = make_unique<MockBtMessage2>(&evcheck2);
  msg2->setUploading(false);
  btMessageDispatcher->addMessageToQueue(std::move(msg1));
  btMessageDispatcher->addMessageToQueue(std::move(msg2));
  btMessageDispatcher->sendMessagesInternal();

  CPPUNIT_ASSERT(evcheck1.sendCalled);
  CPPUNIT_ASSERT(evcheck2.sendCalled);
}

void DefaultBtMessageDispatcherTest::testSendMessages_underUploadLimit()
{
  auto evcheck1 = EventCheck{};
  auto msg1 = make_unique<MockBtMessage2>(&evcheck1);
  msg1->setUploading(true);
  auto evcheck2 = EventCheck{};
  auto msg2 = make_unique<MockBtMessage2>(&evcheck2);
  msg2->setUploading(true);
  btMessageDispatcher->addMessageToQueue(std::move(msg1));
  btMessageDispatcher->addMessageToQueue(std::move(msg2));
  btMessageDispatcher->sendMessagesInternal();

  CPPUNIT_ASSERT(evcheck1.sendCalled);
  CPPUNIT_ASSERT(evcheck2.sendCalled);
}

void DefaultBtMessageDispatcherTest::testDoCancelSendingPieceAction()
{
  auto evcheck1 = EventCheck{};
  auto msg1 = make_unique<MockBtMessage2>(&evcheck1);
  auto evcheck2 = EventCheck{};
  auto msg2 = make_unique<MockBtMessage2>(&evcheck2);

  btMessageDispatcher->addMessageToQueue(std::move(msg1));
  btMessageDispatcher->addMessageToQueue(std::move(msg2));

  btMessageDispatcher->doCancelSendingPieceAction(0, 0, 0);

  CPPUNIT_ASSERT(evcheck1.doCancelActionCalled);
  CPPUNIT_ASSERT(evcheck2.doCancelActionCalled);
}

namespace {
int MY_PIECE_LENGTH = 16_k;
} // namespace

void DefaultBtMessageDispatcherTest::testCheckRequestSlotAndDoNecessaryThing()
{
  auto piece = std::make_shared<Piece>(0, MY_PIECE_LENGTH);
  size_t index;
  CPPUNIT_ASSERT(piece->getMissingUnusedBlockIndex(index));
  CPPUNIT_ASSERT_EQUAL((size_t)0, index);

  btMessageDispatcher->setRequestTimeout(1_min);
  btMessageDispatcher->addOutstandingRequest(
      make_unique<RequestSlot>(0, 0, MY_PIECE_LENGTH, 0, piece));

  btMessageDispatcher->checkRequestSlotAndDoNecessaryThing();

  CPPUNIT_ASSERT_EQUAL((size_t)0,
                       btMessageDispatcher->getMessageQueue().size());
  CPPUNIT_ASSERT_EQUAL((size_t)1,
                       btMessageDispatcher->getRequestSlots().size());
}

void DefaultBtMessageDispatcherTest::
    testCheckRequestSlotAndDoNecessaryThing_timeout()
{
  auto piece = std::make_shared<Piece>(0, MY_PIECE_LENGTH);
  size_t index;
  CPPUNIT_ASSERT(piece->getMissingUnusedBlockIndex(index));
  CPPUNIT_ASSERT_EQUAL((size_t)0, index);

  btMessageDispatcher->setRequestTimeout(1_min);
  auto slot = make_unique<RequestSlot>(0, 0, MY_PIECE_LENGTH, 0, piece);
  // make this slot timeout
  slot->setDispatchedTime(Timer::zero());
  btMessageDispatcher->addOutstandingRequest(std::move(slot));
  btMessageDispatcher->checkRequestSlotAndDoNecessaryThing();

  CPPUNIT_ASSERT_EQUAL((size_t)0,
                       btMessageDispatcher->getMessageQueue().size());
  CPPUNIT_ASSERT_EQUAL((size_t)0,
                       btMessageDispatcher->getRequestSlots().size());
  CPPUNIT_ASSERT_EQUAL(false, piece->isBlockUsed(0));
  CPPUNIT_ASSERT_EQUAL(true, peer->snubbing());
}

void DefaultBtMessageDispatcherTest::
    testCheckRequestSlotAndDoNecessaryThing_completeBlock()
{
  auto piece = std::make_shared<Piece>(0, MY_PIECE_LENGTH);
  piece->completeBlock(0);
  btMessageDispatcher->setRequestTimeout(1_min);
  btMessageDispatcher->addOutstandingRequest(
      make_unique<RequestSlot>(0, 0, MY_PIECE_LENGTH, 0, piece));

  btMessageDispatcher->checkRequestSlotAndDoNecessaryThing();

  CPPUNIT_ASSERT_EQUAL((size_t)1,
                       btMessageDispatcher->getMessageQueue().size());
  CPPUNIT_ASSERT_EQUAL((size_t)0,
                       btMessageDispatcher->getRequestSlots().size());
}

void DefaultBtMessageDispatcherTest::testCountOutstandingRequest()
{
  btMessageDispatcher->addOutstandingRequest(
      make_unique<RequestSlot>(0, 0, MY_PIECE_LENGTH, 0));
  CPPUNIT_ASSERT_EQUAL((size_t)1,
                       btMessageDispatcher->countOutstandingRequest());
}

void DefaultBtMessageDispatcherTest::testIsOutstandingRequest()
{
  btMessageDispatcher->addOutstandingRequest(
      make_unique<RequestSlot>(0, 0, MY_PIECE_LENGTH, 0));

  CPPUNIT_ASSERT(btMessageDispatcher->isOutstandingRequest(0, 0));
  CPPUNIT_ASSERT(!btMessageDispatcher->isOutstandingRequest(0, 1));
  CPPUNIT_ASSERT(!btMessageDispatcher->isOutstandingRequest(1, 0));
  CPPUNIT_ASSERT(!btMessageDispatcher->isOutstandingRequest(1, 1));
}

void DefaultBtMessageDispatcherTest::testGetOutstandingRequest()
{
  btMessageDispatcher->addOutstandingRequest(
      make_unique<RequestSlot>(1, 1_k, 16_k, 10));

  CPPUNIT_ASSERT(btMessageDispatcher->getOutstandingRequest(1, 1_k, 16_k));

  CPPUNIT_ASSERT(!btMessageDispatcher->getOutstandingRequest(1, 1_k, 17_k));

  CPPUNIT_ASSERT(!btMessageDispatcher->getOutstandingRequest(1, 2_k, 16_k));

  CPPUNIT_ASSERT(!btMessageDispatcher->getOutstandingRequest(2, 1_k, 16_k));
}

void DefaultBtMessageDispatcherTest::testRemoveOutstandingRequest()
{
  auto piece = std::make_shared<Piece>(1, 1_m);
  size_t blockIndex = 0;
  CPPUNIT_ASSERT(piece->getMissingUnusedBlockIndex(blockIndex));
  uint32_t begin = blockIndex * piece->getBlockLength();
  size_t length = piece->getBlockLength(blockIndex);
  RequestSlot slot;
  btMessageDispatcher->addOutstandingRequest(make_unique<RequestSlot>(
      piece->getIndex(), begin, length, blockIndex, piece));

  auto s2 = btMessageDispatcher->getOutstandingRequest(piece->getIndex(), begin,
                                                       length);
  CPPUNIT_ASSERT(s2);
  CPPUNIT_ASSERT(piece->isBlockUsed(blockIndex));

  btMessageDispatcher->removeOutstandingRequest(s2);

  auto s3 = btMessageDispatcher->getOutstandingRequest(piece->getIndex(), begin,
                                                       length);
  CPPUNIT_ASSERT(!s3);
  CPPUNIT_ASSERT(!piece->isBlockUsed(blockIndex));
}

} // namespace aria2
