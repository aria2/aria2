#include "BtPortMessage.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include "DHTNode.h"
#include "MockDHTTask.h"
#include "MockDHTTaskFactory.h"
#include "MockDHTTaskQueue.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtPortMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtPortMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreate();
  void testToString();
  void testGetMessage();
  void testDoReceivedAction();

  class MockDHTTaskFactory2:public MockDHTTaskFactory {
  public:
    virtual DHTTaskHandle createPingTask(const DHTNodeHandle& remoteNode,
					 size_t numRetry)
    {
      return new MockDHTTask(remoteNode);
    }
  };
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtPortMessageTest);

void BtPortMessageTest::testCreate() {
  unsigned char msg[7];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 3, 9);
  PeerMessageUtil::setShortIntParam(&msg[5], 12345);
  SharedHandle<BtPortMessage> pm = BtPortMessage::create(&msg[4], 3);
  CPPUNIT_ASSERT_EQUAL((int8_t)9, pm->getId());
  CPPUNIT_ASSERT_EQUAL((uint16_t)12345, pm->getPort());

  // case: payload size is wrong
  try {
    unsigned char msg[8];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 4, 9);
    BtPortMessage::create(&msg[4], 4);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[7];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 3, 10);
    BtPortMessage::create(&msg[4], 3);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
}

void BtPortMessageTest::testToString() {
  BtPortMessage msg(1);
  CPPUNIT_ASSERT_EQUAL(string("port port=1"), msg.toString());
}

void BtPortMessageTest::testGetMessage() {
  BtPortMessage msg(6881);
  unsigned char data[7];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 3, 9);
  PeerMessageUtil::setShortIntParam(&data[5], 6881);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 7) == 0);
}

void BtPortMessageTest::testDoReceivedAction()
{
  PeerHandle peer = new Peer("192.168.0.1", 6881);
  BtPortMessage msg(6881);
  MockDHTTaskQueue taskQueue;
  MockDHTTaskFactory2 taskFactory;
  msg.setTaskQueue(&taskQueue);
  msg.setTaskFactory(&taskFactory);
  msg.setPeer(peer);

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((size_t)1, taskQueue._immediateTaskQueue.size());
  DHTNodeHandle node = SharedHandle<MockDHTTask>(taskQueue._immediateTaskQueue.front())->_remoteNode;
  CPPUNIT_ASSERT_EQUAL(string("192.168.0.1"), node->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, node->getPort());
}
