#include "DHTGetPeersMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "Exception.h"
#include "util.h"
#include "MockDHTMessageFactory.h"
#include "MockDHTMessage.h"
#include "MockDHTMessageDispatcher.h"
#include "DHTTokenTracker.h"
#include "DHTPeerAnnounceStorage.h"
#include "DHTRoutingTable.h"
#include "bencode2.h"

namespace aria2 {

class DHTGetPeersMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTGetPeersMessageTest);
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
    createGetPeersReplyMessage
    (const SharedHandle<DHTNode>& remoteNode,
     const std::vector<SharedHandle<DHTNode> >& closestKNodes,
     const std::vector<SharedHandle<Peer> >& peers,
     const std::string& token,
     const std::string& transactionID)
    {
      SharedHandle<MockDHTResponseMessage> m
        (new MockDHTResponseMessage
         (localNode_, remoteNode, "get_peers", transactionID));
      m->nodes_ = closestKNodes;
      m->peers_ = peers;
      m->token_ = token;
      return m;
    }
  };
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTGetPeersMessageTest);

void DHTGetPeersMessageTest::testGetBencodedMessage()
{
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char infoHash[DHT_ID_LENGTH];
  util::generateRandomData(infoHash, DHT_ID_LENGTH);

  DHTGetPeersMessage msg(localNode, remoteNode, infoHash, transactionID);
  msg.setVersion("A200");

  std::string msgbody = msg.getBencodedMessage();

  Dict dict;
  dict.put("t", transactionID);
  dict.put("v", "A200");
  dict.put("y", "q");
  dict.put("q", "get_peers");
  SharedHandle<Dict> aDict = Dict::g();
  aDict->put("id", String::g(localNode->getID(), DHT_ID_LENGTH));
  aDict->put("info_hash", String::g(infoHash, DHT_ID_LENGTH));
  dict.put("a", aDict);

  CPPUNIT_ASSERT_EQUAL(util::percentEncode(bencode2::encode(&dict)),
                       util::percentEncode(msgbody));
}

void DHTGetPeersMessageTest::testDoReceivedAction()
{
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char infoHash[DHT_ID_LENGTH];
  util::generateRandomData(infoHash, DHT_ID_LENGTH);

  DHTTokenTracker tokenTracker;
  MockDHTMessageDispatcher dispatcher;
  MockDHTMessageFactory2 factory;
  factory.setLocalNode(localNode);
  DHTRoutingTable routingTable(localNode);

  DHTGetPeersMessage msg(localNode, remoteNode, infoHash, transactionID);
  msg.setRoutingTable(&routingTable);
  msg.setTokenTracker(&tokenTracker);
  msg.setMessageDispatcher(&dispatcher);
  msg.setMessageFactory(&factory);
  {
    // localhost has peer contact information for that infohash.
    DHTPeerAnnounceStorage peerAnnounceStorage;
    peerAnnounceStorage.addPeerAnnounce(infoHash, "192.168.0.100", 6888);
    peerAnnounceStorage.addPeerAnnounce(infoHash, "192.168.0.101", 6889);

    msg.setPeerAnnounceStorage(&peerAnnounceStorage);
  
    msg.doReceivedAction();

    CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher.messageQueue_.size());
    SharedHandle<MockDHTResponseMessage> m
      (dynamic_pointer_cast<MockDHTResponseMessage>
       (dispatcher.messageQueue_[0].message_));
    CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
    CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(std::string("get_peers"), m->getMessageType());
    CPPUNIT_ASSERT_EQUAL(msg.getTransactionID(), m->getTransactionID());
    CPPUNIT_ASSERT_EQUAL(tokenTracker.generateToken(infoHash, remoteNode->getIPAddress(), remoteNode->getPort()), m->token_);
    CPPUNIT_ASSERT_EQUAL((size_t)0, m->nodes_.size());
    CPPUNIT_ASSERT_EQUAL((size_t)2, m->peers_.size());
    {
      SharedHandle<Peer> peer = m->peers_[0];
      CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.100"), peer->getIPAddress());
      CPPUNIT_ASSERT_EQUAL((uint16_t)6888, peer->getPort());
    }
    {
      SharedHandle<Peer> peer = m->peers_[1];
      CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.101"), peer->getIPAddress());
      CPPUNIT_ASSERT_EQUAL((uint16_t)6889, peer->getPort());
    }
  }
  dispatcher.messageQueue_.clear();
  {
    // localhost doesn't have peer contact information for that infohash.
    DHTPeerAnnounceStorage peerAnnounceStorage;
    DHTRoutingTable routingTable(localNode);
    SharedHandle<DHTNode> returnNode1(new DHTNode());
    routingTable.addNode(returnNode1);

    msg.setPeerAnnounceStorage(&peerAnnounceStorage);
    msg.setRoutingTable(&routingTable);

    msg.doReceivedAction();

    CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher.messageQueue_.size());
    SharedHandle<MockDHTResponseMessage> m
      (dynamic_pointer_cast<MockDHTResponseMessage>
       (dispatcher.messageQueue_[0].message_));
    CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
    CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(std::string("get_peers"), m->getMessageType());
    CPPUNIT_ASSERT_EQUAL(msg.getTransactionID(), m->getTransactionID());
    CPPUNIT_ASSERT_EQUAL(tokenTracker.generateToken(infoHash, remoteNode->getIPAddress(), remoteNode->getPort()), m->token_);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->nodes_.size());
    CPPUNIT_ASSERT(*returnNode1 == *m->nodes_[0]);
    CPPUNIT_ASSERT_EQUAL((size_t)0, m->peers_.size());
  }
}

} // namespace aria2
