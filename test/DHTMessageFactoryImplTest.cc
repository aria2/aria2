#include "DHTMessageFactoryImpl.h"

#include <cstring>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "RecoverableException.h"
#include "Util.h"
#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "Peer.h"
#include "PeerMessageUtil.h"
#include "DHTBucket.h"
#include "DHTPingMessage.h"
#include "DHTPingReplyMessage.h"
#include "DHTFindNodeMessage.h"
#include "DHTFindNodeReplyMessage.h"
#include "DHTGetPeersMessage.h"
#include "DHTGetPeersReplyMessage.h"
#include "DHTAnnouncePeerMessage.h"
#include "DHTAnnouncePeerReplyMessage.h"
#include "bencode.h"

namespace aria2 {

class DHTMessageFactoryImplTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTMessageFactoryImplTest);
  CPPUNIT_TEST(testCreatePingMessage);
  CPPUNIT_TEST(testCreatePingReplyMessage);
  CPPUNIT_TEST(testCreateFindNodeMessage);
  CPPUNIT_TEST(testCreateFindNodeReplyMessage);
  CPPUNIT_TEST(testCreateGetPeersMessage);
  CPPUNIT_TEST(testCreateGetPeersReplyMessage_nodes);
  CPPUNIT_TEST(testCreateGetPeersReplyMessage_values);
  CPPUNIT_TEST(testCreateAnnouncePeerMessage);
  CPPUNIT_TEST(testCreateAnnouncePeerReplyMessage);
  CPPUNIT_TEST(testReceivedErrorMessage);
  CPPUNIT_TEST_SUITE_END();
public:
  SharedHandle<DHTMessageFactoryImpl> factory;

  SharedHandle<DHTRoutingTable> routingTable;

  SharedHandle<DHTNode> localNode;

  unsigned char transactionID[DHT_TRANSACTION_ID_LENGTH];

  unsigned char remoteNodeID[DHT_ID_LENGTH];

  void setUp()
  {
    localNode.reset(new DHTNode());
    factory.reset(new DHTMessageFactoryImpl());
    factory->setLocalNode(localNode);
    memset(transactionID, 0xff, DHT_TRANSACTION_ID_LENGTH);
    memset(remoteNodeID, 0x0f, DHT_ID_LENGTH);
    routingTable.reset(new DHTRoutingTable(localNode));
    factory->setRoutingTable(routingTable);
  }

  void tearDown() {}

  void testCreatePingMessage();
  void testCreatePingReplyMessage();
  void testCreateFindNodeMessage();
  void testCreateFindNodeReplyMessage();
  void testCreateGetPeersMessage();
  void testCreateGetPeersReplyMessage_nodes();
  void testCreateGetPeersReplyMessage_values();
  void testCreateAnnouncePeerMessage();
  void testCreateAnnouncePeerReplyMessage();
  void testReceivedErrorMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTMessageFactoryImplTest);

void DHTMessageFactoryImplTest::testCreatePingMessage()
{
  bencode::BDE dict = bencode::BDE::dict();
  dict["t"] = bencode::BDE(transactionID, DHT_TRANSACTION_ID_LENGTH);
  dict["y"] = bencode::BDE("q");
  dict["q"] = bencode::BDE("ping");
  bencode::BDE aDict = bencode::BDE::dict();
  aDict["id"] = bencode::BDE(remoteNodeID, DHT_ID_LENGTH);
  dict["a"] = aDict;
  
  SharedHandle<DHTPingMessage> m
    (dynamic_pointer_cast<DHTPingMessage>
     (factory->createQueryMessage(dict, "192.168.0.1", 6881)));
  SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
		       Util::toHex(m->getTransactionID()));
}

void DHTMessageFactoryImplTest::testCreatePingReplyMessage()
{
  bencode::BDE dict = bencode::BDE::dict();
  dict["t"] = bencode::BDE(transactionID, DHT_TRANSACTION_ID_LENGTH);
  dict["y"] = bencode::BDE("r");
  bencode::BDE rDict = bencode::BDE::dict();
  rDict["id"] = bencode::BDE(remoteNodeID, DHT_ID_LENGTH);
  dict["r"] = rDict;

  SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);
  
  SharedHandle<DHTPingReplyMessage> m
    (dynamic_pointer_cast<DHTPingReplyMessage>
     (factory->createResponseMessage("ping", dict,
				     remoteNode->getIPAddress(),
				     remoteNode->getPort())));

  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
		       Util::toHex(m->getTransactionID()));
}

void DHTMessageFactoryImplTest::testCreateFindNodeMessage()
{
  bencode::BDE dict = bencode::BDE::dict();
  dict["t"] = bencode::BDE(transactionID, DHT_TRANSACTION_ID_LENGTH);
  dict["y"] = bencode::BDE("q");
  dict["q"] = bencode::BDE("find_node");
  bencode::BDE aDict = bencode::BDE::dict();
  aDict["id"] = bencode::BDE(remoteNodeID, DHT_ID_LENGTH);
  unsigned char targetNodeID[DHT_ID_LENGTH];
  memset(targetNodeID, 0x11, DHT_ID_LENGTH);
  aDict["target"] = bencode::BDE(targetNodeID, DHT_ID_LENGTH);
  dict["a"] = aDict;
  
  SharedHandle<DHTFindNodeMessage> m
    (dynamic_pointer_cast<DHTFindNodeMessage>
     (factory->createQueryMessage(dict, "192.168.0.1", 6881)));
  SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
		       Util::toHex(m->getTransactionID()));
  CPPUNIT_ASSERT_EQUAL(Util::toHex(targetNodeID, DHT_ID_LENGTH),
		       Util::toHex(m->getTargetNodeID(), DHT_ID_LENGTH));
}

void DHTMessageFactoryImplTest::testCreateFindNodeReplyMessage()
{
  try {
    bencode::BDE dict = bencode::BDE::dict();
    dict["t"] = bencode::BDE(transactionID, DHT_TRANSACTION_ID_LENGTH);
    dict["y"] = bencode::BDE("r");
    bencode::BDE rDict = bencode::BDE::dict();
    rDict["id"] = bencode::BDE(remoteNodeID, DHT_ID_LENGTH);
    std::string compactNodeInfo;
    SharedHandle<DHTNode> nodes[8];
    for(size_t i = 0; i < DHTBucket::K; ++i) {
      nodes[i].reset(new DHTNode());
      nodes[i]->setIPAddress("192.168.0."+Util::uitos(i+1));
      nodes[i]->setPort(6881+i);

      unsigned char buf[6];
      CPPUNIT_ASSERT(PeerMessageUtil::createcompact
		     (buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
      compactNodeInfo +=
	std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
	std::string(&buf[0], &buf[sizeof(buf)]);
    }
    rDict["nodes"] = compactNodeInfo;
    dict["r"] = rDict;

    SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
    remoteNode->setIPAddress("192.168.0.1");
    remoteNode->setPort(6881);
  
    SharedHandle<DHTFindNodeReplyMessage> m
      (dynamic_pointer_cast<DHTFindNodeReplyMessage>
       (factory->createResponseMessage("find_node", dict,
				       remoteNode->getIPAddress(),
				       remoteNode->getPort())));

    CPPUNIT_ASSERT(localNode == m->getLocalNode());
    CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL((size_t)DHTBucket::K, m->getClosestKNodes().size());
    CPPUNIT_ASSERT(nodes[0] == m->getClosestKNodes()[0]);
    CPPUNIT_ASSERT(nodes[7] == m->getClosestKNodes()[7]);
    CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
			 Util::toHex(m->getTransactionID()));
  } catch(Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void DHTMessageFactoryImplTest::testCreateGetPeersMessage()
{
  bencode::BDE dict = bencode::BDE::dict();
  dict["t"] = bencode::BDE(transactionID, DHT_TRANSACTION_ID_LENGTH);
  dict["y"] = bencode::BDE("q");
  dict["q"] = bencode::BDE("get_peers");
  bencode::BDE aDict = bencode::BDE::dict();
  aDict["id"] = bencode::BDE(remoteNodeID, DHT_ID_LENGTH);
  unsigned char infoHash[DHT_ID_LENGTH];
  memset(infoHash, 0x11, DHT_ID_LENGTH);
  aDict["info_hash"] = bencode::BDE(infoHash, DHT_ID_LENGTH);
  dict["a"] = aDict;
  
  SharedHandle<DHTGetPeersMessage> m
    (dynamic_pointer_cast<DHTGetPeersMessage>
     (factory->createQueryMessage(dict, "192.168.0.1", 6881)));
  SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
		       Util::toHex(m->getTransactionID()));
  CPPUNIT_ASSERT_EQUAL(Util::toHex(infoHash, DHT_ID_LENGTH),
		       Util::toHex(m->getInfoHash(), DHT_ID_LENGTH));
}

void DHTMessageFactoryImplTest::testCreateGetPeersReplyMessage_nodes()
{
  try {
    bencode::BDE dict = bencode::BDE::dict();
    dict["t"] = bencode::BDE(transactionID, DHT_TRANSACTION_ID_LENGTH);
    dict["y"] = bencode::BDE("r");
    bencode::BDE rDict = bencode::BDE::dict();
    rDict["id"] = bencode::BDE(remoteNodeID, DHT_ID_LENGTH);
    std::string compactNodeInfo;
    SharedHandle<DHTNode> nodes[8];
    for(size_t i = 0; i < DHTBucket::K; ++i) {
      nodes[i].reset(new DHTNode());
      nodes[i]->setIPAddress("192.168.0."+Util::uitos(i+1));
      nodes[i]->setPort(6881+i);

      unsigned char buf[6];
      CPPUNIT_ASSERT(PeerMessageUtil::createcompact
		     (buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
      compactNodeInfo +=
	std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
	std::string(&buf[0], &buf[sizeof(buf)]);
    }
    rDict["nodes"] = compactNodeInfo;
    rDict["token"] = bencode::BDE("token");
    dict["r"] = rDict;

    SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
    remoteNode->setIPAddress("192.168.0.1");
    remoteNode->setPort(6881);
  
    SharedHandle<DHTGetPeersReplyMessage> m
      (dynamic_pointer_cast<DHTGetPeersReplyMessage>
       (factory->createResponseMessage("get_peers", dict,
				       remoteNode->getIPAddress(),
				       remoteNode->getPort())));

    CPPUNIT_ASSERT(localNode == m->getLocalNode());
    CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(std::string("token"), m->getToken());
    CPPUNIT_ASSERT_EQUAL((size_t)DHTBucket::K, m->getClosestKNodes().size());
    CPPUNIT_ASSERT(nodes[0] == m->getClosestKNodes()[0]);
    CPPUNIT_ASSERT(nodes[7] == m->getClosestKNodes()[7]);
    CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
			 Util::toHex(m->getTransactionID()));
  } catch(Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void DHTMessageFactoryImplTest::testCreateGetPeersReplyMessage_values()
{
  try {
    bencode::BDE dict = bencode::BDE::dict();
    dict["t"] = bencode::BDE(transactionID, DHT_TRANSACTION_ID_LENGTH);
    dict["y"] = bencode::BDE("r");
    bencode::BDE rDict = bencode::BDE::dict();
    rDict["id"] = bencode::BDE(remoteNodeID, DHT_ID_LENGTH);

    std::deque<SharedHandle<Peer> > peers;
    bencode::BDE valuesList = bencode::BDE::list();
    for(size_t i = 0; i < 4; ++i) {
      SharedHandle<Peer> peer(new Peer("192.168.0."+Util::uitos(i+1), 6881+i));
      unsigned char buffer[6];
      CPPUNIT_ASSERT(PeerMessageUtil::createcompact(buffer, peer->ipaddr,
						    peer->port));
      valuesList << bencode::BDE(buffer, sizeof(buffer));
      peers.push_back(peer);
    }
    rDict["values"] = valuesList;
    rDict["token"] = bencode::BDE("token");
    dict["r"] = rDict;

    SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
    remoteNode->setIPAddress("192.168.0.1");
    remoteNode->setPort(6881);
  
    SharedHandle<DHTGetPeersReplyMessage> m
      (dynamic_pointer_cast<DHTGetPeersReplyMessage>
       (factory->createResponseMessage("get_peers", dict,
				       remoteNode->getIPAddress(),
				       remoteNode->getPort())));

    CPPUNIT_ASSERT(localNode == m->getLocalNode());
    CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(std::string("token"), m->getToken());
    CPPUNIT_ASSERT_EQUAL((size_t)4, m->getValues().size());
    CPPUNIT_ASSERT(peers[0] == m->getValues()[0]);
    CPPUNIT_ASSERT(peers[3] == m->getValues()[3]);
    CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
			 Util::toHex(m->getTransactionID()));
  } catch(Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void DHTMessageFactoryImplTest::testCreateAnnouncePeerMessage()
{
  try {
    bencode::BDE dict = bencode::BDE::dict();
    dict["t"] = bencode::BDE(transactionID, DHT_TRANSACTION_ID_LENGTH);
    dict["y"] = bencode::BDE("q");
    dict["q"] = bencode::BDE("announce_peer");
    bencode::BDE aDict = bencode::BDE::dict();
    aDict["id"] = bencode::BDE(remoteNodeID, DHT_ID_LENGTH);
    unsigned char infoHash[DHT_ID_LENGTH];
    memset(infoHash, 0x11, DHT_ID_LENGTH);
    aDict["info_hash"] = bencode::BDE(infoHash, DHT_ID_LENGTH);
    std::string token = "ffff";
    uint16_t port = 6881;
    aDict["port"] = port;
    aDict["token"] = token;
    dict["a"] = aDict;
  
    SharedHandle<DHTAnnouncePeerMessage> m
      (dynamic_pointer_cast<DHTAnnouncePeerMessage>
       (factory->createQueryMessage(dict, "192.168.0.1", 6882)));
    SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
    remoteNode->setIPAddress("192.168.0.1");
    remoteNode->setPort(6882);

    CPPUNIT_ASSERT(localNode == m->getLocalNode());
    CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(token, m->getToken());
    CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
			 Util::toHex(m->getTransactionID()));
    CPPUNIT_ASSERT_EQUAL(Util::toHex(infoHash, DHT_ID_LENGTH),
			 Util::toHex(m->getInfoHash(), DHT_ID_LENGTH));
    CPPUNIT_ASSERT_EQUAL(port, m->getTCPPort());
  } catch(Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void DHTMessageFactoryImplTest::testCreateAnnouncePeerReplyMessage()
{
  bencode::BDE dict = bencode::BDE::dict();
  dict["t"] = bencode::BDE(transactionID, DHT_TRANSACTION_ID_LENGTH);
  dict["y"] = bencode::BDE("r");
  bencode::BDE rDict = bencode::BDE::dict();
  rDict["id"] = bencode::BDE(remoteNodeID, DHT_ID_LENGTH);
  dict["r"] = rDict;

  SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);
  
  SharedHandle<DHTAnnouncePeerReplyMessage> m
    (dynamic_pointer_cast<DHTAnnouncePeerReplyMessage>
     (factory->createResponseMessage("announce_peer", dict,
				     remoteNode->getIPAddress(),
				     remoteNode->getPort())));

  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
		       Util::toHex(m->getTransactionID()));
}

void DHTMessageFactoryImplTest::testReceivedErrorMessage()
{
  bencode::BDE dict = bencode::BDE::dict();
  dict["t"] = bencode::BDE(transactionID, DHT_TRANSACTION_ID_LENGTH);
  dict["y"] = bencode::BDE("e");
  bencode::BDE list = bencode::BDE::list();
  list << 404;
  list << bencode::BDE("Not found");
  dict["e"] = list;

  SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  try {
    factory->createResponseMessage("announce_peer", dict,
				   remoteNode->getIPAddress(),
				   remoteNode->getPort());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

} // namespace aria2
