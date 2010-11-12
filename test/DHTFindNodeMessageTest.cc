#include "DHTFindNodeMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "Exception.h"
#include "util.h"
#include "MockDHTMessageFactory.h"
#include "MockDHTMessage.h"
#include "MockDHTMessageDispatcher.h"
#include "DHTRoutingTable.h"
#include "bencode2.h"

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
    virtual SharedHandle<DHTResponseMessage>
    createFindNodeReplyMessage
    (const SharedHandle<DHTNode>& remoteNode,
     const std::vector<SharedHandle<DHTNode> >& closestKNodes,
     const std::string& transactionID)
    {
      SharedHandle<MockDHTResponseMessage> m
        (new MockDHTResponseMessage
         (localNode_, remoteNode, "find_node", transactionID));
      m->nodes_ = closestKNodes;
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
  msg.setVersion("A200");
  std::string msgbody = msg.getBencodedMessage();

  Dict dict;
  dict.put("t", transactionID);
  dict.put("v", "A200");
  dict.put("y", "q");
  dict.put("q", "find_node");
  SharedHandle<Dict> aDict = Dict::g();
  aDict->put("id", String::g(localNode->getID(), DHT_ID_LENGTH));
  aDict->put("target", String::g(targetNode->getID(), DHT_ID_LENGTH));
  dict.put("a", aDict);

  CPPUNIT_ASSERT_EQUAL(bencode2::encode(&dict), msgbody);
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
  msg.setMessageDispatcher(&dispatcher);
  msg.setMessageFactory(&factory);
  msg.setRoutingTable(&routingTable);

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher.messageQueue_.size());
  SharedHandle<MockDHTResponseMessage> m
    (dynamic_pointer_cast<MockDHTResponseMessage>
     (dispatcher.messageQueue_[0].message_));
  CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
  CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(std::string("find_node"), m->getMessageType());
  CPPUNIT_ASSERT_EQUAL(msg.getTransactionID(), m->getTransactionID());
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->nodes_.size());
}

} // namespace aria2
