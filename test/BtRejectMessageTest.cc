#include "BtRejectMessage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "Peer.h"
#include "FileEntry.h"
#include "MockBtMessageDispatcher.h"

namespace aria2 {

class BtRejectMessageTest:public CppUnit::TestFixture {

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
    RequestSlot slot;
  public:
    MockBtMessageDispatcher2():slot(RequestSlot::nullSlot) {}

    void setRequestSlot(const RequestSlot& slot) {
      this->slot = slot;
    }

    virtual RequestSlot getOutstandingRequest
    (size_t index, int32_t begin, int32_t length) {
      if(slot.getIndex() == index && slot.getBegin() == begin &&
         slot.getLength() == length) {
        return slot;
      } else {
        return RequestSlot::nullSlot;
      }
    }

    virtual void removeOutstandingRequest(const RequestSlot& slot) {
      if(this->slot.getIndex() == slot.getIndex() &&
         this->slot.getBegin() == slot.getBegin() &&
         this->slot.getLength() == slot.getLength()) {
        this->slot = RequestSlot::nullSlot;
      }
    }
  };

  typedef SharedHandle<MockBtMessageDispatcher2> MockBtMessageDispatcher2Handle;

  SharedHandle<Peer> peer;
  SharedHandle<MockBtMessageDispatcher2> dispatcher;
  SharedHandle<BtRejectMessage> msg;

  void setUp() {
    peer.reset(new Peer("host", 6969));
    peer->allocateSessionResource(1024, 1024*1024);

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

void BtRejectMessageTest::testCreate() {
  unsigned char msg[17];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 13, 16);
  bittorrent::setIntParam(&msg[5], 12345);
  bittorrent::setIntParam(&msg[9], 256);
  bittorrent::setIntParam(&msg[13], 1024);
  SharedHandle<BtRejectMessage> pm = BtRejectMessage::create(&msg[4], 13);
  CPPUNIT_ASSERT_EQUAL((uint8_t)16, pm->getId());
  CPPUNIT_ASSERT_EQUAL((size_t)12345, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL(256, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL(1024, pm->getLength());

  // case: payload size is wrong
  try {
    unsigned char msg[18];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 14, 16);
    BtRejectMessage::create(&msg[4], 14);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[17];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 13, 17);
    BtRejectMessage::create(&msg[4], 13);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
}

void BtRejectMessageTest::testCreateMessage() {
  BtRejectMessage msg;
  msg.setIndex(12345);
  msg.setBegin(256);
  msg.setLength(1024);
  unsigned char data[17];
  bittorrent::createPeerMessageString(data, sizeof(data), 13, 16);
  bittorrent::setIntParam(&data[5], 12345);
  bittorrent::setIntParam(&data[9], 256);
  bittorrent::setIntParam(&data[13], 1024);
  unsigned char* rawmsg = msg.createMessage();
  CPPUNIT_ASSERT(memcmp(rawmsg, data, 17) == 0);
  delete [] rawmsg;
}

void BtRejectMessageTest::testDoReceivedAction() {
  peer->setFastExtensionEnabled(true);
  RequestSlot slot(1, 16, 32, 2);
  dispatcher->setRequestSlot(slot);
  
  CPPUNIT_ASSERT
    (!RequestSlot::isNull(dispatcher->getOutstandingRequest(1, 16, 32)));

  msg->doReceivedAction();

  CPPUNIT_ASSERT
    (RequestSlot::isNull(dispatcher->getOutstandingRequest(1, 16, 32)));
}

void BtRejectMessageTest::testDoReceivedActionNoMatch() {
  peer->setFastExtensionEnabled(true);
  RequestSlot slot(2, 16, 32, 2);
  dispatcher->setRequestSlot(slot);
  
  CPPUNIT_ASSERT
    (!RequestSlot::isNull(dispatcher->getOutstandingRequest(2, 16, 32)));

  msg->doReceivedAction();

  CPPUNIT_ASSERT
    (!RequestSlot::isNull(dispatcher->getOutstandingRequest(2, 16, 32)));  

}

void BtRejectMessageTest::testDoReceivedActionFastExtensionDisabled() {
  RequestSlot slot(1, 16, 32, 2);
  dispatcher->setRequestSlot(slot);
  
  CPPUNIT_ASSERT
    (!RequestSlot::isNull(dispatcher->getOutstandingRequest(1, 16, 32)));
  try {
    msg->doReceivedAction();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {}

}

void BtRejectMessageTest::testToString() {
  CPPUNIT_ASSERT_EQUAL(std::string("reject index=1, begin=16, length=32"),
                       msg->toString());
}

} // namespace aria2
