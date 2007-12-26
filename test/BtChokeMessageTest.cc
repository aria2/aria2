#include "BtChokeMessage.h"
#include "PeerMessageUtil.h"
#include "MockBtMessageDispatcher.h"
#include "MockBtRequestFactory.h"
#include "MockBtContext.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtChokeMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtChokeMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testOnSendComplete);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  BtChokeMessageTest():peer(0), btContext(0) {}

  PeerHandle peer;
  MockBtContextHandle btContext;

  void setUp() {
    BtRegistry::unregisterAll();    
    peer = new Peer("host", 6969);
    btContext = new MockBtContext();
    btContext->setInfoHash((const unsigned char*)"12345678901234567890");
    BtRegistry::registerPeerObjectCluster(btContext->getInfoHashAsString(),
					  new PeerObjectCluster());
    PEER_OBJECT_CLUSTER(btContext)->registerHandle(peer->getId(), new PeerObject());
  }

  void testCreate();
  void testGetMessage();
  void testDoReceivedAction();
  void testOnSendComplete();
  void testToString();

  class MockBtMessageDispatcher2 : public MockBtMessageDispatcher {
  public:
    bool doChokedActionCalled;
    bool doChokingActionCalled;
  public:
    MockBtMessageDispatcher2():doChokedActionCalled(false), doChokingActionCalled(false) {}

    virtual void doChokedAction() {
      doChokedActionCalled = true;
    }

    virtual void doChokingAction() {
      doChokingActionCalled = true;
    }
  };

  typedef SharedHandle<MockBtMessageDispatcher2> MockBtMessageDispatcher2Handle;

  class MockBtRequestFactory2 : public MockBtRequestFactory {
  public:
    bool doChokedActionCalled;
  public:
    MockBtRequestFactory2():doChokedActionCalled(false) {}

    virtual void doChokedAction() {
      doChokedActionCalled = true;
    }
  };

  typedef SharedHandle<MockBtRequestFactory2> MockBtRequestFactory2Handle;
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtChokeMessageTest);

void BtChokeMessageTest::testCreate() {
  unsigned char msg[5];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 0);
  BtChokeMessageHandle pm = BtChokeMessage::create(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL((int8_t)0, pm->getId());

  // case: payload size is wrong
  try {
    unsigned char msg[6];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 2, 0);
    BtChokeMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[5];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 1);
    BtChokeMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
}

void BtChokeMessageTest::testGetMessage() {
  BtChokeMessage msg;
  unsigned char data[5];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 1, 0);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 5) == 0);
}

void BtChokeMessageTest::testDoReceivedAction() {
  BtChokeMessage msg;
  msg.setPeer(peer);
  msg.setBtContext(btContext);

  MockBtMessageDispatcher2Handle dispatcher = new MockBtMessageDispatcher2();
  msg.setBtMessageDispatcher(dispatcher);
  MockBtRequestFactory2Handle requestFactory = new MockBtRequestFactory2();
  msg.setBtRequestFactory(requestFactory);

  msg.doReceivedAction();

  CPPUNIT_ASSERT(dispatcher->doChokedActionCalled);
  CPPUNIT_ASSERT(peer->peerChoking);
}

void BtChokeMessageTest::testOnSendComplete() {
  BtChokeMessage msg;
  msg.setPeer(peer);
  msg.setBtContext(btContext);

  MockBtMessageDispatcher2Handle dispatcher = new MockBtMessageDispatcher2();
  msg.setBtMessageDispatcher(dispatcher);

  msg.onSendComplete();

  CPPUNIT_ASSERT(dispatcher->doChokingActionCalled);
  CPPUNIT_ASSERT(peer->amChoking);  
}

void BtChokeMessageTest::testToString() {
  BtChokeMessage msg;
  CPPUNIT_ASSERT_EQUAL(string("choke"), msg.toString());
}
