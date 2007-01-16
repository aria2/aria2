#include "BtRejectMessage.h"
#include "PeerMessageUtil.h"
#include "Peer.h"
#include "MockBtMessageDispatcher.h"
#include "MockBtContext.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtRejectMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtRejectMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testDoReceivedActionNoMatch);
  CPPUNIT_TEST(testDoReceivedActionFastExtensionDisabled);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void testCreate();
  void testGetMessage();
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

    virtual RequestSlot getOutstandingRequest(int32_t index, int32_t begin,
					      uint32_t length) {
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

  PeerHandle peer;
  MockBtMessageDispatcher2Handle dispatcher;
  BtRejectMessageHandle msg;

  BtRejectMessageTest():peer(0), dispatcher(0), msg(0) {}

  void setUp() {
    BtRegistry::clear();
    peer = new Peer("host", 6969, 16*1024, 256*1024);

    MockBtContextHandle btContext = new MockBtContext();
    btContext->setInfoHash((const unsigned char*)"12345678901234567890");
    BtRegistry::registerPeerObjectCluster(btContext->getInfoHashAsString(),
					  new PeerObjectCluster());
    PEER_OBJECT_CLUSTER(btContext)->registerHandle(peer->getId(), new PeerObject());
    dispatcher = new MockBtMessageDispatcher2();
    PEER_OBJECT(btContext, peer)->btMessageDispatcher = dispatcher;

    msg = new BtRejectMessage();
    msg->setPeer(peer);
    msg->setBtContext(btContext);
    msg->setIndex(1);
    msg->setBegin(16);
    msg->setLength(32);
    msg->setBtMessageDispatcher(dispatcher);

  }
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtRejectMessageTest);

void BtRejectMessageTest::testCreate() {
  unsigned char msg[17];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 13, 16);
  PeerMessageUtil::setIntParam(&msg[5], 12345);
  PeerMessageUtil::setIntParam(&msg[9], 256);
  PeerMessageUtil::setIntParam(&msg[13], 1024);
  BtRejectMessageHandle pm = BtRejectMessage::create(&msg[4], 13);
  CPPUNIT_ASSERT_EQUAL((uint8_t)16, pm->getId());
  CPPUNIT_ASSERT_EQUAL(12345, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL(256, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL((uint32_t)1024, pm->getLength());

  // case: payload size is wrong
  try {
    unsigned char msg[18];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 14, 16);
    BtRejectMessage::create(&msg[4], 14);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[17];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 13, 17);
    BtRejectMessage::create(&msg[4], 13);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void BtRejectMessageTest::testGetMessage() {
  BtRejectMessage msg;
  msg.setIndex(12345);
  msg.setBegin(256);
  msg.setLength(1024);
  unsigned char data[17];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 13, 16);
  PeerMessageUtil::setIntParam(&data[5], 12345);
  PeerMessageUtil::setIntParam(&data[9], 256);
  PeerMessageUtil::setIntParam(&data[13], 1024);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 17) == 0);
}

void BtRejectMessageTest::testDoReceivedAction() {
  peer->setFastExtensionEnabled(true);
  RequestSlot slot(1, 16, 32, 2);
  dispatcher->setRequestSlot(slot);
  
  CPPUNIT_ASSERT(!RequestSlot::isNull(dispatcher->getOutstandingRequest(1, 16, 32)));

  msg->doReceivedAction();

  CPPUNIT_ASSERT(RequestSlot::isNull(dispatcher->getOutstandingRequest(1, 16, 32)));
}

void BtRejectMessageTest::testDoReceivedActionNoMatch() {
  peer->setFastExtensionEnabled(true);
  RequestSlot slot(2, 16, 32, 2);
  dispatcher->setRequestSlot(slot);
  
  CPPUNIT_ASSERT(!RequestSlot::isNull(dispatcher->getOutstandingRequest(2, 16, 32)));

  msg->doReceivedAction();

  CPPUNIT_ASSERT(!RequestSlot::isNull(dispatcher->getOutstandingRequest(2, 16, 32)));  

}

void BtRejectMessageTest::testDoReceivedActionFastExtensionDisabled() {
  RequestSlot slot(1, 16, 32, 2);
  dispatcher->setRequestSlot(slot);
  
  CPPUNIT_ASSERT(!RequestSlot::isNull(dispatcher->getOutstandingRequest(1, 16, 32)));
  try {
    msg->doReceivedAction();
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {}

}

void BtRejectMessageTest::testToString() {
  CPPUNIT_ASSERT_EQUAL(string("reject index=1, begin=16, length=32"),
		       msg->toString());
}
