#include "BtCancelMessage.h"
#include "PeerMessageUtil.h"
#include "MockBtMessageDispatcher.h"
#include "MockBtContext.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtCancelMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtCancelMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  BtCancelMessageTest():peer(0), btContext(0) {}

  PeerHandle peer;
  MockBtContextHandle btContext;

  void setUp() {
    BtRegistry::unregisterAll();    
    peer = new Peer("host", 6969, 16*1024, 256*1024);
    btContext = new MockBtContext();
    btContext->setInfoHash((const unsigned char*)"12345678901234567890");
    BtRegistry::registerPeerObjectCluster(btContext->getInfoHashAsString(),
					  new PeerObjectCluster());
    PEER_OBJECT_CLUSTER(btContext)->registerHandle(peer->getId(), new PeerObject());
  }

  void testCreate();
  void testGetMessage();
  void testDoReceivedAction();

  class MockBtMessageDispatcher2 : public MockBtMessageDispatcher {
  public:
    int32_t index;
    int32_t begin;
    int32_t length;
  public:
    MockBtMessageDispatcher2():index(0),
			       begin(0),
			       length(0) {}

    virtual void doCancelSendingPieceAction(int32_t index, int32_t begin, int32_t length) {
      this->index = index;
      this->begin = begin;
      this->length = length;
    }
  };

  typedef SharedHandle<MockBtMessageDispatcher2> MockBtMessageDispatcher2Handle;
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtCancelMessageTest);

void BtCancelMessageTest::testCreate() {
  unsigned char msg[17];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 13, 8);
  PeerMessageUtil::setIntParam(&msg[5], 12345);
  PeerMessageUtil::setIntParam(&msg[9], 256);
  PeerMessageUtil::setIntParam(&msg[13], 1024);
  BtCancelMessageHandle pm = BtCancelMessage::create(&msg[4], 13);
  CPPUNIT_ASSERT_EQUAL((int8_t)8, pm->getId());
  CPPUNIT_ASSERT_EQUAL((int32_t)12345, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL((int32_t)256, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL((int32_t)1024, pm->getLength());

  // case: payload size is wrong
  try {
    unsigned char msg[18];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 14, 8);
    BtCancelMessage::create(&msg[4], 14);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[17];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 13, 9);
    BtCancelMessage::create(&msg[4], 13);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
}

void BtCancelMessageTest::testGetMessage() {
  BtCancelMessage msg;
  msg.setIndex(12345);
  msg.setBegin(256);
  msg.setLength(1024);
  unsigned char data[17];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 13, 8);
  PeerMessageUtil::setIntParam(&data[5], 12345);
  PeerMessageUtil::setIntParam(&data[9], 256);
  PeerMessageUtil::setIntParam(&data[13], 1024);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 17) == 0);
}

void BtCancelMessageTest::testDoReceivedAction() {
  BtCancelMessage msg;
  msg.setIndex(1);
  msg.setBegin(2*16*1024);
  msg.setLength(16*1024);
  msg.setBtContext(btContext);
  msg.setPeer(peer);
  MockBtMessageDispatcher2Handle dispatcher = new MockBtMessageDispatcher2();
  msg.setBtMessageDispatcher(dispatcher);

  msg.doReceivedAction();
  CPPUNIT_ASSERT_EQUAL(msg.getIndex(), dispatcher->index);
  CPPUNIT_ASSERT_EQUAL(msg.getBegin(), dispatcher->begin);
  CPPUNIT_ASSERT_EQUAL(msg.getLength(), dispatcher->length);
}
