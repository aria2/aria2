#include "DHTFindNodeMessage.h"
#include "DHTNode.h"
#include "DHTUtil.h"
#include "BencodeVisitor.h"
#include "Dictionary.h"
#include "Data.h"
#include "Exception.h"
#include "Util.h"
#include "MockDHTMessageFactory.h"
#include "MockDHTMessage.h"
#include "MockDHTMessageDispatcher.h"
#include "DHTRoutingTable.h"
#include <cppunit/extensions/HelperMacros.h>

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
    createFindNodeReplyMessage(const SharedHandle<DHTNode>& remoteNode,
			       const std::deque<SharedHandle<DHTNode> >& closestKNodes,
			       const std::string& transactionID)
    {
      SharedHandle<MockDHTMessage> m =
	new MockDHTMessage(_localNode, remoteNode, "find_node", transactionID);
      m->_nodes = closestKNodes;
      return m;
    }
  };
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTFindNodeMessageTest);

void DHTFindNodeMessageTest::testGetBencodedMessage()
{
  SharedHandle<DHTNode> localNode = new DHTNode();
  SharedHandle<DHTNode> remoteNode = new DHTNode();

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  SharedHandle<DHTNode> targetNode = new DHTNode();

  DHTFindNodeMessage msg(localNode, remoteNode, targetNode->getID(), transactionID);

  std::string msgbody = msg.getBencodedMessage();

  SharedHandle<Dictionary> cm = new Dictionary();
  cm->put("t", new Data(transactionID));
  cm->put("y", new Data("q"));
  cm->put("q", new Data("find_node"));
  Dictionary* a = new Dictionary();
  cm->put("a", a);
  a->put("id", new Data(localNode->getID(), DHT_ID_LENGTH));
  a->put("target", new Data(targetNode->getID(), DHT_ID_LENGTH));

  BencodeVisitor v;
  cm->accept(&v);

  CPPUNIT_ASSERT_EQUAL(v.getBencodedData(), msgbody);
}

void DHTFindNodeMessageTest::testDoReceivedAction()
{
  SharedHandle<DHTNode> localNode = new DHTNode();
  SharedHandle<DHTNode> remoteNode = new DHTNode();

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  SharedHandle<DHTNode> targetNode = new DHTNode();

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

  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher._messageQueue.size());
  SharedHandle<MockDHTMessage> m = dispatcher._messageQueue[0]._message;
  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(std::string("find_node"), m->getMessageType());
  CPPUNIT_ASSERT_EQUAL(msg.getTransactionID(), m->getTransactionID());
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->_nodes.size());
}

} // namespace aria2
