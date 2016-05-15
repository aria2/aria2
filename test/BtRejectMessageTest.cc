#include "BtRejectMessage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "Peer.h"
#include "FileEntry.h"
#include "MockBtMessageDispatcher.h"

namespace aria2 {

class BtRejectMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtRejectMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testDoReceivedActionNoMatch);
  CPPUNIT_TEST(testDoReceivedActionFastExtensionDisabled);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void testCreate();
  void testCreateMessage();
  void testDoReceivedAction();
  void testDoReceivedActionNoMatch();
  void testDoReceivedActionFastExtensionDisabled();
  void testToString();

  class MockBtMessageDispatcher2 : public MockBtMessageDispatcher {
  public:
    std::unique_ptr<RequestSlot> slot;

    void setRequestSlot(std::unique_ptr<RequestSlot> s) { slot = std::move(s); }

    virtual const RequestSlot*
    getOutstandingRequest(size_t index, int32_t begin,
                          int32_t length) CXX11_OVERRIDE
    {
      if (slot && slot->getIndex() == index && slot->getBegin() == begin &&
          slot->getLength() == length) {
        return slot.get();
      }
      else {
        return nullptr;
      }
    }

    virtual void removeOutstandingRequest(const RequestSlot* s) CXX11_OVERRIDE
    {
      if (slot->getIndex() == s->getIndex() &&
          slot->getBegin() == s->getBegin() &&
          slot->getLength() == s->getLength()) {
        slot.reset();
      }
    }
  };

  typedef std::shared_ptr<MockBtMessageDispatcher2>
      MockBtMessageDispatcher2Handle;

  std::shared_ptr<Peer> peer;
  std::shared_ptr<MockBtMessageDispatcher2> dispatcher;
  std::shared_ptr<BtRejectMessage> msg;

  void setUp()
  {
    peer.reset(new Peer("host", 6969));
    peer->allocateSessionResource(1_k, 1_m);

    dispatcher.reset(new MockBtMessageDispatcher2());

    msg.reset(new BtRejectMessage());
    msg->setPeer(peer);
    msg->setIndex(1);
    msg->setBegin(16);
    msg->setLength(32);
    msg->setBtMessageDispatcher(dispatcher.get());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtRejectMessageTest);

void BtRejectMessageTest::testCreate()
{
  unsigned char msg[17];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 13, 16);
  bittorrent::setIntParam(&msg[5], 12345);
  bittorrent::setIntParam(&msg[9], 256);
  bittorrent::setIntParam(&msg[13], 1_k);
  std::shared_ptr<BtRejectMessage> pm(BtRejectMessage::create(&msg[4], 13));
  CPPUNIT_ASSERT_EQUAL((uint8_t)16, pm->getId());
  CPPUNIT_ASSERT_EQUAL((size_t)12345, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL(256, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL((int32_t)1_k, pm->getLength());

  // case: payload size is wrong
  try {
    unsigned char msg[18];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 14, 16);
    BtRejectMessage::create(&msg[4], 14);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[17];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 13, 17);
    BtRejectMessage::create(&msg[4], 13);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtRejectMessageTest::testCreateMessage()
{
  BtRejectMessage msg;
  msg.setIndex(12345);
  msg.setBegin(256);
  msg.setLength(1_k);
  unsigned char data[17];
  bittorrent::createPeerMessageString(data, sizeof(data), 13, 16);
  bittorrent::setIntParam(&data[5], 12345);
  bittorrent::setIntParam(&data[9], 256);
  bittorrent::setIntParam(&data[13], 1_k);
  auto rawmsg = msg.createMessage();
  CPPUNIT_ASSERT_EQUAL((size_t)17, rawmsg.size());
  CPPUNIT_ASSERT(std::equal(std::begin(rawmsg), std::end(rawmsg), data));
}

void BtRejectMessageTest::testDoReceivedAction()
{
  peer->setFastExtensionEnabled(true);
  dispatcher->setRequestSlot(make_unique<RequestSlot>(1, 16, 32, 2));

  CPPUNIT_ASSERT(dispatcher->getOutstandingRequest(1, 16, 32));

  msg->doReceivedAction();

  CPPUNIT_ASSERT(!dispatcher->getOutstandingRequest(1, 16, 32));
}

void BtRejectMessageTest::testDoReceivedActionNoMatch()
{
  peer->setFastExtensionEnabled(true);
  dispatcher->setRequestSlot(make_unique<RequestSlot>(2, 16, 32, 2));

  CPPUNIT_ASSERT(dispatcher->getOutstandingRequest(2, 16, 32));

  msg->doReceivedAction();

  CPPUNIT_ASSERT(dispatcher->getOutstandingRequest(2, 16, 32));
}

void BtRejectMessageTest::testDoReceivedActionFastExtensionDisabled()
{
  RequestSlot slot(1, 16, 32, 2);
  dispatcher->setRequestSlot(make_unique<RequestSlot>(1, 16, 32, 2));

  CPPUNIT_ASSERT(dispatcher->getOutstandingRequest(1, 16, 32));
  try {
    msg->doReceivedAction();
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtRejectMessageTest::testToString()
{
  CPPUNIT_ASSERT_EQUAL(std::string("reject index=1, begin=16, length=32"),
                       msg->toString());
}

} // namespace aria2
