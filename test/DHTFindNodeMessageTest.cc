#include "DHTFindNodeMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "Exception.h"
#include "util.h"
#include "MockDHTMessageFactory.h"
#include "MockDHTMessage.h"
#include "MockDHTMessageDispatcher.h"
#include "DHTRoutingTable.h"
#include "bencode.h"

namespace aria2 {

class DHTFindNodeMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTFindNodeMessageTest);
  CPPUNIT_TEST(testGetBencodedMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGetBencodedMessage();
  void testDoReceivedAction();

  class MockDHTMessageFactory2:public MockDHTMessageFactory {
  public:
    virtual SharedHandle<DHTMessage>
    createFindNodeReplyMessage
    (const SharedHandle<DHTNode>& remoteNode,
     const std::vector<SharedHandle<DHTNode> >& closestKNodes,
     const std::string& transactionID)
    {
      SharedHandle<MockDHTMessage> m
        (new MockDHTMessage(_localNode, remoteNode, "find_node", transactionID));
      m->_nodes = closestKNodes;
      return m;
    }
  };
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTFindNodeMessageTest);

void DHTFindNodeMessageTest::testGetBencodedMessage()
{
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  SharedHandle<DHTNode> targetNode(new DHTNode());

  DHTFindNodeMessage msg(localNode, remoteNode, targetNode->getID(), transactionID);

  std::string msgbody = msg.getBencodedMessage();

  BDE dict = BDE::dict();
  dict["t"] = transactionID;
  dict["y"] = BDE("q");
  dict["q"] = BDE("find_node");
  BDE aDict = BDE::dict();
  aDict["id"] = BDE(localNode->getID(), DHT_ID_LENGTH);
  aDict["target"] = BDE(targetNode->getID(), DHT_ID_LENGTH);
  dict["a"] = aDict;

  CPPUNIT_ASSERT_EQUAL(bencode::encode(dict), msgbody);
}

void DHTFindNodeMessageTest::testDoReceivedAction()
{
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  SharedHandle<DHTNode> targetNode(new DHTNode());

  MockDHTMessageDispatcher dispatcher;
  MockDHTMessageFactory2 factory;
  factory.setLocalNode(localNode);
  DHTRoutingTable routingTable(localNode);
  routingTable.addNode(targetNode);

  DHTFindNodeMessage msg(localNode, remoteNode, targetNode->getID(), transactionID);
  msg.setMessageDispatcher(WeakHandle<DHTMessageDispatcher>(&dispatcher));
  msg.setMessageFactory(WeakHandle<DHTMessageFactory>(&factory));
  msg.setRoutingTable(WeakHandle<DHTRoutingTable>(&routingTable));

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher._messageQueue.size());
  SharedHandle<MockDHTMessage> m
    (dynamic_pointer_cast<MockDHTMessage>(dispatcher._messageQueue[0]._message));
  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(std::string("find_node"), m->getMessageType());
  CPPUNIT_ASSERT_EQUAL(msg.getTransactionID(), m->getTransactionID());
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->_nodes.size());
}

} // namespace aria2
