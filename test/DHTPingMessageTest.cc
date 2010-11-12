#include "DHTPingMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "Exception.h"
#include "util.h"
#include "MockDHTMessageFactory.h"
#include "MockDHTMessageDispatcher.h"
#include "MockDHTMessage.h"
#include "bencode2.h"

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
    virtual SharedHandle<DHTResponseMessage>
    createPingReplyMessage(const SharedHandle<DHTNode>& remoteNode,
                           const unsigned char* remoteNodeID,
                           const std::string& transactionID)
    {
      return SharedHandle<MockDHTResponseMessage>
        (new MockDHTResponseMessage(localNode_, remoteNode, "ping_reply",
                                    transactionID));
    }
  };
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTPingMessageTest);

void DHTPingMessageTest::testGetBencodedMessage()
{
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  DHTPingMessage msg(localNode, remoteNode, transactionID);
  msg.setVersion("A200");

  std::string msgbody = msg.getBencodedMessage();

  Dict dict;
  dict.put("t", transactionID);
  dict.put("v", "A200");
  dict.put("y", "q");
  dict.put("q", "ping");
  SharedHandle<Dict> aDict = Dict::g();
  aDict->put("id", String::g(localNode->getID(), DHT_ID_LENGTH));
  dict.put("a", aDict);

  CPPUNIT_ASSERT_EQUAL(bencode2::encode(&dict), msgbody);
}

void DHTPingMessageTest::testDoReceivedAction()
{
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  MockDHTMessageDispatcher dispatcher;
  MockDHTMessageFactory2 factory;
  factory.setLocalNode(localNode);

  DHTPingMessage msg(localNode, remoteNode, transactionID);
  msg.setMessageDispatcher(&dispatcher);
  msg.setMessageFactory(&factory);

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher.messageQueue_.size());
  SharedHandle<MockDHTResponseMessage> m
    (dynamic_pointer_cast<MockDHTResponseMessage>
     (dispatcher.messageQueue_[0].message_));
  CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
  CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(std::string("ping_reply"), m->getMessageType());
  CPPUNIT_ASSERT_EQUAL(msg.getTransactionID(), m->getTransactionID());
}

} // namespace aria2
