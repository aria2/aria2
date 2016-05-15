#include "BtPortMessage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "util.h"
#include "array_fun.h"
#include "Peer.h"
#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "MockDHTTask.h"
#include "MockDHTTaskFactory.h"
#include "MockDHTTaskQueue.h"

namespace aria2 {

class BtPortMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtPortMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testDoReceivedAction_bootstrap);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testCreate();
  void testToString();
  void testCreateMessage();
  void testDoReceivedAction();
  void testDoReceivedAction_bootstrap();

  class MockDHTTaskFactory2 : public MockDHTTaskFactory {
  public:
    virtual std::shared_ptr<DHTTask>
    createPingTask(const std::shared_ptr<DHTNode>& remoteNode,
                   int numRetry) CXX11_OVERRIDE
    {
      return std::shared_ptr<DHTTask>(new MockDHTTask(remoteNode));
    }

    virtual std::shared_ptr<DHTTask>
    createNodeLookupTask(const unsigned char* targetID) CXX11_OVERRIDE
    {
      std::shared_ptr<MockDHTTask> task(
          new MockDHTTask(std::shared_ptr<DHTNode>()));
      task->setTargetID(targetID);
      return task;
    }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtPortMessageTest);

void BtPortMessageTest::testCreate()
{
  unsigned char msg[7];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 3, 9);
  bittorrent::setShortIntParam(&msg[5], 12345);
  std::shared_ptr<BtPortMessage> pm(BtPortMessage::create(&msg[4], 3));
  CPPUNIT_ASSERT_EQUAL((uint8_t)9, pm->getId());
  CPPUNIT_ASSERT_EQUAL((uint16_t)12345, pm->getPort());

  // case: payload size is wrong
  try {
    unsigned char msg[8];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 4, 9);
    BtPortMessage::create(&msg[4], 4);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[7];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 3, 10);
    BtPortMessage::create(&msg[4], 3);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtPortMessageTest::testToString()
{
  BtPortMessage msg(1);
  CPPUNIT_ASSERT_EQUAL(std::string("port port=1"), msg.toString());
}

void BtPortMessageTest::testCreateMessage()
{
  BtPortMessage msg(6881);
  unsigned char data[7];
  bittorrent::createPeerMessageString(data, sizeof(data), 3, 9);
  bittorrent::setShortIntParam(&data[5], 6881);
  auto rawmsg = msg.createMessage();
  CPPUNIT_ASSERT_EQUAL((size_t)7, rawmsg.size());
  CPPUNIT_ASSERT(std::equal(std::begin(rawmsg), std::end(rawmsg), data));
}

void BtPortMessageTest::testDoReceivedAction()
{
  unsigned char nodeID[DHT_ID_LENGTH];
  memset(nodeID, 0, DHT_ID_LENGTH);
  std::shared_ptr<DHTNode> localNode(new DHTNode(nodeID));

  // 9 nodes to create at least 2 buckets.
  std::shared_ptr<DHTNode> nodes[9];
  for (size_t i = 0; i < arraySize(nodes); ++i) {
    memset(nodeID, 0, DHT_ID_LENGTH);
    nodeID[DHT_ID_LENGTH - 1] = i + 1;
    nodes[i].reset(new DHTNode(nodeID));
  }

  DHTRoutingTable routingTable(localNode);
  for (size_t i = 0; i < arraySize(nodes); ++i) {
    routingTable.addNode(nodes[i]);
  }

  std::shared_ptr<Peer> peer(new Peer("192.168.0.1", 6881));
  BtPortMessage msg(6881);
  MockDHTTaskQueue taskQueue;
  MockDHTTaskFactory2 taskFactory;
  msg.setLocalNode(localNode.get());
  msg.setRoutingTable(&routingTable);
  msg.setTaskQueue(&taskQueue);
  msg.setTaskFactory(&taskFactory);
  msg.setPeer(peer);

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((size_t)1, taskQueue.immediateTaskQueue_.size());

  auto task =
      std::dynamic_pointer_cast<MockDHTTask>(taskQueue.immediateTaskQueue_[0]);
  std::shared_ptr<DHTNode> node = task->remoteNode_;
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), node->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, node->getPort());
}

void BtPortMessageTest::testDoReceivedAction_bootstrap()
{
  unsigned char nodeID[DHT_ID_LENGTH];
  memset(nodeID, 0, DHT_ID_LENGTH);
  nodeID[0] = 0xff;
  std::shared_ptr<DHTNode> localNode(new DHTNode(nodeID));
  DHTRoutingTable routingTable(localNode); // no nodes , 1 bucket.

  std::shared_ptr<Peer> peer(new Peer("192.168.0.1", 6881));
  BtPortMessage msg(6881);
  MockDHTTaskQueue taskQueue;
  MockDHTTaskFactory2 taskFactory;
  msg.setLocalNode(localNode.get());
  msg.setRoutingTable(&routingTable);
  msg.setTaskQueue(&taskQueue);
  msg.setTaskFactory(&taskFactory);
  msg.setPeer(peer);

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((size_t)2, taskQueue.immediateTaskQueue_.size());
  auto task =
      std::dynamic_pointer_cast<MockDHTTask>(taskQueue.immediateTaskQueue_[0]);
  std::shared_ptr<DHTNode> node(task->remoteNode_);
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), node->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, node->getPort());

  auto task2 =
      std::dynamic_pointer_cast<MockDHTTask>(taskQueue.immediateTaskQueue_[1]);
  CPPUNIT_ASSERT(memcmp(nodeID, task2->targetID_, DHT_ID_LENGTH) == 0);
}

} // namespace aria2
