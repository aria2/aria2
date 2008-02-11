#include "DHTPingMessage.h"
#include "DHTNode.h"
#include "DHTUtil.h"
#include "BencodeVisitor.h"
#include "Dictionary.h"
#include "Data.h"
#include "Exception.h"
#include "Util.h"
#include "MockDHTMessageFactory.h"
#include "MockDHTMessageDispatcher.h"
#include "MockDHTMessage.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHTPingMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTPingMessageTest);
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
    createPingReplyMessage(const SharedHandle<DHTNode>& remoteNode,
			   const unsigned char* remoteNodeID,
			   const std::string& transactionID)
    {
      return new MockDHTMessage(_localNode, remoteNode, "ping_reply",
				transactionID);
    }
  };
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTPingMessageTest);

void DHTPingMessageTest::testGetBencodedMessage()
{
  SharedHandle<DHTNode> localNode = new DHTNode();
  SharedHandle<DHTNode> remoteNode = new DHTNode();

  char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  DHTPingMessage msg(localNode, remoteNode, transactionID);

  std::string msgbody = msg.getBencodedMessage();

  SharedHandle<Dictionary> cm = new Dictionary();
  cm->put("t", new Data(transactionID));
  cm->put("y", new Data("q"));
  cm->put("q", new Data("ping"));
  Dictionary* a = new Dictionary();
  cm->put("a", a);
  a->put("id", new Data(reinterpret_cast<const char*>(localNode->getID()), DHT_ID_LENGTH));

  BencodeVisitor v;
  cm->accept(&v);

  CPPUNIT_ASSERT_EQUAL(v.getBencodedData(), msgbody);
}

void DHTPingMessageTest::testDoReceivedAction()
{
  SharedHandle<DHTNode> localNode = new DHTNode();
  SharedHandle<DHTNode> remoteNode = new DHTNode();

  char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  MockDHTMessageDispatcher dispatcher;
  MockDHTMessageFactory2 factory;
  factory.setLocalNode(localNode);

  DHTPingMessage msg(localNode, remoteNode, transactionID);
  msg.setMessageDispatcher(&dispatcher);
  msg.setMessageFactory(&factory);

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher._messageQueue.size());
  SharedHandle<MockDHTMessage> m = dispatcher._messageQueue[0]._message;
  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(std::string("ping_reply"), m->getMessageType());
  CPPUNIT_ASSERT_EQUAL(msg.getTransactionID(), m->getTransactionID());
}

} // namespace aria2
