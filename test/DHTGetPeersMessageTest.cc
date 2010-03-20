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
#include "bencode.h"

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
    virtual SharedHandle<DHTMessage>
    createGetPeersReplyMessage(const SharedHandle<DHTNode>& remoteNode,
                               const std::vector<SharedHandle<Peer> >& peers,
                               const std::string& token,
                               const std::string& transactionID)
    {
      SharedHandle<MockDHTMessage> m
        (new MockDHTMessage(_localNode, remoteNode, "get_peers", transactionID));
      m->_peers = peers;
      m->_token = token;
      return m;
    }

    virtual SharedHandle<DHTMessage>
    createGetPeersReplyMessage
    (const SharedHandle<DHTNode>& remoteNode,
     const std::vector<SharedHandle<DHTNode> >& closestKNodes,
     const std::string& token,
     const std::string& transactionID)
    {
      SharedHandle<MockDHTMessage> m
        (new MockDHTMessage(_localNode, remoteNode, "get_peers", transactionID));
      m->_nodes = closestKNodes;
      m->_token = token;
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

  BDE dict = BDE::dict();
  dict["t"] = transactionID;
  dict["v"] = BDE("A200");
  dict["y"] = BDE("q");
  dict["q"] = BDE("get_peers");
  BDE aDict = BDE::dict();
  aDict["id"] = BDE(localNode->getID(), DHT_ID_LENGTH);
  aDict["info_hash"] = BDE(infoHash, DHT_ID_LENGTH);
  dict["a"] = aDict;

  CPPUNIT_ASSERT_EQUAL(util::percentEncode(bencode::encode(dict)),
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

  DHTGetPeersMessage msg(localNode, remoteNode, infoHash, transactionID);
  msg.setTokenTracker(WeakHandle<DHTTokenTracker>
                      (&tokenTracker));
  msg.setMessageDispatcher(WeakHandle<DHTMessageDispatcher>
                           (&dispatcher));
  msg.setMessageFactory(WeakHandle<DHTMessageFactory>
                        (&factory));
  {
    // localhost has peer contact information for that infohash.
    DHTPeerAnnounceStorage peerAnnounceStorage;
    peerAnnounceStorage.addPeerAnnounce(infoHash, "192.168.0.100", 6888);
    peerAnnounceStorage.addPeerAnnounce(infoHash, "192.168.0.101", 6889);

    msg.setPeerAnnounceStorage(WeakHandle<DHTPeerAnnounceStorage>
                               (&peerAnnounceStorage));
  
    msg.doReceivedAction();

    CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher._messageQueue.size());
    SharedHandle<MockDHTMessage> m
      (dynamic_pointer_cast<MockDHTMessage>(dispatcher._messageQueue[0]._message));
    CPPUNIT_ASSERT(localNode == m->getLocalNode());
    CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(std::string("get_peers"), m->getMessageType());
    CPPUNIT_ASSERT_EQUAL(msg.getTransactionID(), m->getTransactionID());
    CPPUNIT_ASSERT_EQUAL(tokenTracker.generateToken(infoHash, remoteNode->getIPAddress(), remoteNode->getPort()), m->_token);
    CPPUNIT_ASSERT_EQUAL((size_t)0, m->_nodes.size());
    CPPUNIT_ASSERT_EQUAL((size_t)2, m->_peers.size());
    {
      SharedHandle<Peer> peer = m->_peers[0];
      CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.100"), peer->ipaddr);
      CPPUNIT_ASSERT_EQUAL((uint16_t)6888, peer->port);
    }
    {
      SharedHandle<Peer> peer = m->_peers[1];
      CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.101"), peer->ipaddr);
      CPPUNIT_ASSERT_EQUAL((uint16_t)6889, peer->port);
    }
  }
  dispatcher._messageQueue.clear();
  {
    // localhost doesn't have peer contact information for that infohash.
    DHTPeerAnnounceStorage peerAnnounceStorage;
    DHTRoutingTable routingTable(localNode);
    SharedHandle<DHTNode> returnNode1(new DHTNode());
    routingTable.addNode(returnNode1);

    msg.setPeerAnnounceStorage(WeakHandle<DHTPeerAnnounceStorage>
                               (&peerAnnounceStorage));
    msg.setRoutingTable(WeakHandle<DHTRoutingTable>
                        (&routingTable));

    msg.doReceivedAction();

    CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher._messageQueue.size());
    SharedHandle<MockDHTMessage> m
      (dynamic_pointer_cast<MockDHTMessage>(dispatcher._messageQueue[0]._message));
    CPPUNIT_ASSERT(localNode == m->getLocalNode());
    CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(std::string("get_peers"), m->getMessageType());
    CPPUNIT_ASSERT_EQUAL(msg.getTransactionID(), m->getTransactionID());
    CPPUNIT_ASSERT_EQUAL(tokenTracker.generateToken(infoHash, remoteNode->getIPAddress(), remoteNode->getPort()), m->_token);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->_nodes.size());
    CPPUNIT_ASSERT(returnNode1 == m->_nodes[0]);
    CPPUNIT_ASSERT_EQUAL((size_t)0, m->_peers.size());
  }
}

} // namespace aria2
