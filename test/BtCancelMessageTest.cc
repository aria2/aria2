#include "BtCancelMessage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "MockBtMessageDispatcher.h"
#include "Peer.h"
#include "FileEntry.h"
#include "Piece.h"

namespace aria2 {

class BtCancelMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtCancelMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST_SUITE_END();

private:
  std::shared_ptr<Peer> peer;

public:
  void setUp() { peer.reset(new Peer("host", 6969)); }

  void testCreate();
  void testCreateMessage();
  void testDoReceivedAction();

  class MockBtMessageDispatcher2 : public MockBtMessageDispatcher {
  public:
    size_t index;
    int32_t begin;
    int32_t length;

  public:
    MockBtMessageDispatcher2() : index(0), begin(0), length(0) {}

    virtual void doCancelSendingPieceAction(size_t index, int32_t begin,
                                            int32_t length) CXX11_OVERRIDE
    {
      this->index = index;
      this->begin = begin;
      this->length = length;
    }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtCancelMessageTest);

void BtCancelMessageTest::testCreate()
{
  unsigned char msg[17];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 13, 8);
  bittorrent::setIntParam(&msg[5], 12345);
  bittorrent::setIntParam(&msg[9], 256);
  bittorrent::setIntParam(&msg[13], 1_k);
  auto pm = BtCancelMessage::create(&msg[4], 13);
  CPPUNIT_ASSERT_EQUAL((uint8_t)8, pm->getId());
  CPPUNIT_ASSERT_EQUAL((size_t)12345, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL(256, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL((int32_t)1_k, pm->getLength());

  // case: payload size is wrong
  try {
    unsigned char msg[18];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 14, 8);
    BtCancelMessage::create(&msg[4], 14);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[17];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 13, 9);
    BtCancelMessage::create(&msg[4], 13);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtCancelMessageTest::testCreateMessage()
{
  BtCancelMessage msg;
  msg.setIndex(12345);
  msg.setBegin(256);
  msg.setLength(1_k);
  unsigned char data[17];
  bittorrent::createPeerMessageString(data, sizeof(data), 13, 8);
  bittorrent::setIntParam(&data[5], 12345);
  bittorrent::setIntParam(&data[9], 256);
  bittorrent::setIntParam(&data[13], 1_k);
  auto rawmsg = msg.createMessage();
  CPPUNIT_ASSERT_EQUAL((size_t)17, rawmsg.size());
  CPPUNIT_ASSERT(std::equal(std::begin(rawmsg), std::end(rawmsg), data));
}

void BtCancelMessageTest::testDoReceivedAction()
{
  BtCancelMessage msg;
  msg.setIndex(1);
  msg.setBegin(32_k);
  msg.setLength(16_k);
  msg.setPeer(peer);
  auto dispatcher = make_unique<MockBtMessageDispatcher2>();
  msg.setBtMessageDispatcher(dispatcher.get());

  msg.doReceivedAction();
  CPPUNIT_ASSERT_EQUAL(msg.getIndex(), dispatcher->index);
  CPPUNIT_ASSERT_EQUAL(msg.getBegin(), dispatcher->begin);
  CPPUNIT_ASSERT_EQUAL(msg.getLength(), dispatcher->length);
}

} // namespace aria2
