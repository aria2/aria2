#include "DHTMessageFactoryImpl.h"

#include <cstring>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "RecoverableException.h"
#include "util.h"
#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "Peer.h"
#include "bittorrent_helper.h"
#include "DHTBucket.h"
#include "DHTPingMessage.h"
#include "DHTPingReplyMessage.h"
#include "DHTFindNodeMessage.h"
#include "DHTFindNodeReplyMessage.h"
#include "DHTGetPeersMessage.h"
#include "DHTGetPeersReplyMessage.h"
#include "DHTAnnouncePeerMessage.h"
#include "DHTAnnouncePeerReplyMessage.h"
#include "bencode2.h"

namespace aria2 {

class DHTMessageFactoryImplTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTMessageFactoryImplTest);
  CPPUNIT_TEST(testCreatePingMessage);
  CPPUNIT_TEST(testCreatePingReplyMessage);
  CPPUNIT_TEST(testCreateFindNodeMessage);
  CPPUNIT_TEST(testCreateFindNodeReplyMessage);
  CPPUNIT_TEST(testCreateFindNodeReplyMessage6);
  CPPUNIT_TEST(testCreateGetPeersMessage);
  CPPUNIT_TEST(testCreateGetPeersReplyMessage);
  CPPUNIT_TEST(testCreateGetPeersReplyMessage6);
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
    factory.reset(new DHTMessageFactoryImpl(AF_INET));
    factory->setLocalNode(localNode);
    memset(transactionID, 0xff, DHT_TRANSACTION_ID_LENGTH);
    memset(remoteNodeID, 0x0f, DHT_ID_LENGTH);
    routingTable.reset(new DHTRoutingTable(localNode));
    factory->setRoutingTable(routingTable.get());
  }

  void tearDown() {}

  void testCreatePingMessage();
  void testCreatePingReplyMessage();
  void testCreateFindNodeMessage();
  void testCreateFindNodeReplyMessage();
  void testCreateFindNodeReplyMessage6();
  void testCreateGetPeersMessage();
  void testCreateGetPeersReplyMessage();
  void testCreateGetPeersReplyMessage6();
  void testCreateAnnouncePeerMessage();
  void testCreateAnnouncePeerReplyMessage();
  void testReceivedErrorMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTMessageFactoryImplTest);

void DHTMessageFactoryImplTest::testCreatePingMessage()
{
  Dict dict;
  dict.put("t", String::g(transactionID, DHT_TRANSACTION_ID_LENGTH));
  dict.put("y", "q");
  dict.put("q", "ping");
  SharedHandle<Dict> aDict = Dict::g();
  aDict->put("id", String::g(remoteNodeID, DHT_ID_LENGTH));
  dict.put("a", aDict);
  
  SharedHandle<DHTPingMessage> m
    (dynamic_pointer_cast<DHTPingMessage>
     (factory->createQueryMessage(&dict, "192.168.0.1", 6881)));
  SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
  CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
                       util::toHex(m->getTransactionID()));
}

void DHTMessageFactoryImplTest::testCreatePingReplyMessage()
{
  Dict dict;
  dict.put("t", String::g(transactionID, DHT_TRANSACTION_ID_LENGTH));
  dict.put("y", "r");
  SharedHandle<Dict> rDict = Dict::g();
  rDict->put("id", String::g(remoteNodeID, DHT_ID_LENGTH));
  dict.put("r", rDict);

  SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);
  
  SharedHandle<DHTPingReplyMessage> m
    (dynamic_pointer_cast<DHTPingReplyMessage>
     (factory->createResponseMessage("ping", &dict,
                                     remoteNode->getIPAddress(),
                                     remoteNode->getPort())));

  CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
  CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
                       util::toHex(m->getTransactionID()));
}

void DHTMessageFactoryImplTest::testCreateFindNodeMessage()
{
  Dict dict;
  dict.put("t", String::g(transactionID, DHT_TRANSACTION_ID_LENGTH));
  dict.put("y", "q");
  dict.put("q", "find_node");
  SharedHandle<Dict> aDict = Dict::g();
  aDict->put("id", String::g(remoteNodeID, DHT_ID_LENGTH));
  unsigned char targetNodeID[DHT_ID_LENGTH];
  memset(targetNodeID, 0x11, DHT_ID_LENGTH);
  aDict->put("target", String::g(targetNodeID, DHT_ID_LENGTH));
  dict.put("a", aDict);
  
  SharedHandle<DHTFindNodeMessage> m
    (dynamic_pointer_cast<DHTFindNodeMessage>
     (factory->createQueryMessage(&dict, "192.168.0.1", 6881)));
  SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
  CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
                       util::toHex(m->getTransactionID()));
  CPPUNIT_ASSERT_EQUAL(util::toHex(targetNodeID, DHT_ID_LENGTH),
                       util::toHex(m->getTargetNodeID(), DHT_ID_LENGTH));
}

void DHTMessageFactoryImplTest::testCreateFindNodeReplyMessage()
{
  try {
    Dict dict;
    dict.put("t", String::g(transactionID, DHT_TRANSACTION_ID_LENGTH));
    dict.put("y", "r");
    SharedHandle<Dict> rDict = Dict::g();
    rDict->put("id", String::g(remoteNodeID, DHT_ID_LENGTH));
    std::string compactNodeInfo;
    SharedHandle<DHTNode> nodes[8];
    for(size_t i = 0; i < DHTBucket::K; ++i) {
      nodes[i].reset(new DHTNode());
      nodes[i]->setIPAddress("192.168.0."+util::uitos(i+1));
      nodes[i]->setPort(6881+i);

      unsigned char buf[COMPACT_LEN_IPV6];
      CPPUNIT_ASSERT_EQUAL
        (COMPACT_LEN_IPV4,
         bittorrent::packcompact
         (buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
      compactNodeInfo +=
        std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
        std::string(&buf[0], &buf[COMPACT_LEN_IPV4]);
    }
    rDict->put("nodes", compactNodeInfo);
    dict.put("r", rDict);

    SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
    remoteNode->setIPAddress("192.168.0.1");
    remoteNode->setPort(6881);
  
    SharedHandle<DHTFindNodeReplyMessage> m
      (dynamic_pointer_cast<DHTFindNodeReplyMessage>
       (factory->createResponseMessage("find_node", &dict,
                                       remoteNode->getIPAddress(),
                                       remoteNode->getPort())));

    CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
    CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL((size_t)DHTBucket::K, m->getClosestKNodes().size());
    CPPUNIT_ASSERT(*nodes[0] == *m->getClosestKNodes()[0]);
    CPPUNIT_ASSERT(*nodes[7] == *m->getClosestKNodes()[7]);
    CPPUNIT_ASSERT_EQUAL(util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
                         util::toHex(m->getTransactionID()));
  } catch(Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void DHTMessageFactoryImplTest::testCreateFindNodeReplyMessage6()
{
  factory.reset(new DHTMessageFactoryImpl(AF_INET6));
  factory->setLocalNode(localNode);
  factory->setRoutingTable(routingTable.get());
  try {
    Dict dict;
    dict.put("t", String::g(transactionID, DHT_TRANSACTION_ID_LENGTH));
    dict.put("y", "r");
    SharedHandle<Dict> rDict = Dict::g();
    rDict->put("id", String::g(remoteNodeID, DHT_ID_LENGTH));
    std::string compactNodeInfo;
    SharedHandle<DHTNode> nodes[8];
    for(size_t i = 0; i < DHTBucket::K; ++i) {
      nodes[i].reset(new DHTNode());
      nodes[i]->setIPAddress("2001::000"+util::uitos(i+1));
      nodes[i]->setPort(6881+i);

      unsigned char buf[COMPACT_LEN_IPV6];
      CPPUNIT_ASSERT_EQUAL
        (COMPACT_LEN_IPV6,
         bittorrent::packcompact
         (buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
      compactNodeInfo +=
        std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
        std::string(&buf[0], &buf[COMPACT_LEN_IPV6]);
    }
    rDict->put("nodes6", compactNodeInfo);
    dict.put("r", rDict);

    SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
    remoteNode->setIPAddress("2001::2001");
    remoteNode->setPort(6881);
  
    SharedHandle<DHTFindNodeReplyMessage> m
      (dynamic_pointer_cast<DHTFindNodeReplyMessage>
       (factory->createResponseMessage("find_node", &dict,
                                       remoteNode->getIPAddress(),
                                       remoteNode->getPort())));

    CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
    CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL((size_t)DHTBucket::K, m->getClosestKNodes().size());
    CPPUNIT_ASSERT(*nodes[0] == *m->getClosestKNodes()[0]);
    CPPUNIT_ASSERT(*nodes[7] == *m->getClosestKNodes()[7]);
    CPPUNIT_ASSERT_EQUAL(util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
                         util::toHex(m->getTransactionID()));
  } catch(Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void DHTMessageFactoryImplTest::testCreateGetPeersMessage()
{
  Dict dict;
  dict.put("t", String::g(transactionID, DHT_TRANSACTION_ID_LENGTH));
  dict.put("y", "q");
  dict.put("q", "get_peers");
  SharedHandle<Dict> aDict = Dict::g();
  aDict->put("id", String::g(remoteNodeID, DHT_ID_LENGTH));
  unsigned char infoHash[DHT_ID_LENGTH];
  memset(infoHash, 0x11, DHT_ID_LENGTH);
  aDict->put("info_hash", String::g(infoHash, DHT_ID_LENGTH));
  dict.put("a", aDict);
  
  SharedHandle<DHTGetPeersMessage> m
    (dynamic_pointer_cast<DHTGetPeersMessage>
     (factory->createQueryMessage(&dict, "192.168.0.1", 6881)));
  SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
  CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
                       util::toHex(m->getTransactionID()));
  CPPUNIT_ASSERT_EQUAL(util::toHex(infoHash, DHT_ID_LENGTH),
                       util::toHex(m->getInfoHash(), DHT_ID_LENGTH));
}

void DHTMessageFactoryImplTest::testCreateGetPeersReplyMessage()
{
  try {
    Dict dict;
    dict.put("t", String::g(transactionID, DHT_TRANSACTION_ID_LENGTH));
    dict.put("y", "r");
    SharedHandle<Dict> rDict = Dict::g();
    rDict->put("id", String::g(remoteNodeID, DHT_ID_LENGTH));
    std::string compactNodeInfo;
    SharedHandle<DHTNode> nodes[8];
    for(size_t i = 0; i < DHTBucket::K; ++i) {
      nodes[i].reset(new DHTNode());
      nodes[i]->setIPAddress("192.168.0."+util::uitos(i+1));
      nodes[i]->setPort(6881+i);

      unsigned char buf[COMPACT_LEN_IPV6];
      CPPUNIT_ASSERT_EQUAL
        (COMPACT_LEN_IPV4,
         bittorrent::packcompact
         (buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
      compactNodeInfo +=
        std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
        std::string(&buf[0], &buf[COMPACT_LEN_IPV4]);
    }
    rDict->put("nodes", compactNodeInfo);

    std::deque<SharedHandle<Peer> > peers;
    SharedHandle<List> valuesList = List::g();
    for(size_t i = 0; i < 4; ++i) {
      SharedHandle<Peer> peer(new Peer("192.168.0."+util::uitos(i+1), 6881+i));
      unsigned char buffer[COMPACT_LEN_IPV6];
      CPPUNIT_ASSERT_EQUAL
        (COMPACT_LEN_IPV4,
         bittorrent::packcompact
         (buffer, peer->getIPAddress(), peer->getPort()));
      valuesList->append(String::g(buffer, COMPACT_LEN_IPV4));
      peers.push_back(peer);
    }
    rDict->put("values", valuesList);

    rDict->put("token", "token");
    dict.put("r", rDict);

    SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
    remoteNode->setIPAddress("192.168.0.1");
    remoteNode->setPort(6881);
  
    SharedHandle<DHTGetPeersReplyMessage> m
      (dynamic_pointer_cast<DHTGetPeersReplyMessage>
       (factory->createResponseMessage("get_peers", &dict,
                                       remoteNode->getIPAddress(),
                                       remoteNode->getPort())));

    CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
    CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(std::string("token"), m->getToken());
    CPPUNIT_ASSERT_EQUAL((size_t)DHTBucket::K, m->getClosestKNodes().size());
    CPPUNIT_ASSERT(*nodes[0] == *m->getClosestKNodes()[0]);
    CPPUNIT_ASSERT(*nodes[7] == *m->getClosestKNodes()[7]);
    CPPUNIT_ASSERT_EQUAL((size_t)4, m->getValues().size());
    CPPUNIT_ASSERT(*peers[0] == *m->getValues()[0]);
    CPPUNIT_ASSERT(*peers[3] == *m->getValues()[3]);
    CPPUNIT_ASSERT_EQUAL(util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
                         util::toHex(m->getTransactionID()));
  } catch(Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void DHTMessageFactoryImplTest::testCreateGetPeersReplyMessage6()
{
  factory.reset(new DHTMessageFactoryImpl(AF_INET6));
  factory->setLocalNode(localNode);
  factory->setRoutingTable(routingTable.get());
  try {
    Dict dict;
    dict.put("t", String::g(transactionID, DHT_TRANSACTION_ID_LENGTH));
    dict.put("y", "r");
    SharedHandle<Dict> rDict = Dict::g();
    rDict->put("id", String::g(remoteNodeID, DHT_ID_LENGTH));
    std::string compactNodeInfo;
    SharedHandle<DHTNode> nodes[8];
    for(size_t i = 0; i < DHTBucket::K; ++i) {
      nodes[i].reset(new DHTNode());
      nodes[i]->setIPAddress("2001::000"+util::uitos(i+1));
      nodes[i]->setPort(6881+i);

      unsigned char buf[COMPACT_LEN_IPV6];
      CPPUNIT_ASSERT_EQUAL
        (COMPACT_LEN_IPV6,
         bittorrent::packcompact
         (buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
      compactNodeInfo +=
        std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
        std::string(&buf[0], &buf[COMPACT_LEN_IPV6]);
    }
    rDict->put("nodes6", compactNodeInfo);

    std::deque<SharedHandle<Peer> > peers;
    SharedHandle<List> valuesList = List::g();
    for(size_t i = 0; i < 4; ++i) {
      SharedHandle<Peer> peer(new Peer("2001::100"+util::uitos(i+1), 6881+i));
      unsigned char buffer[COMPACT_LEN_IPV6];
      CPPUNIT_ASSERT_EQUAL
        (COMPACT_LEN_IPV6,
         bittorrent::packcompact
         (buffer, peer->getIPAddress(), peer->getPort()));
      valuesList->append(String::g(buffer, COMPACT_LEN_IPV6));
      peers.push_back(peer);
    }
    rDict->put("values", valuesList);

    rDict->put("token", "token");
    dict.put("r", rDict);

    SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
    remoteNode->setIPAddress("2001::2001");
    remoteNode->setPort(6881);
  
    SharedHandle<DHTGetPeersReplyMessage> m
      (dynamic_pointer_cast<DHTGetPeersReplyMessage>
       (factory->createResponseMessage("get_peers", &dict,
                                       remoteNode->getIPAddress(),
                                       remoteNode->getPort())));

    CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
    CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(std::string("token"), m->getToken());
    CPPUNIT_ASSERT_EQUAL((size_t)DHTBucket::K, m->getClosestKNodes().size());
    CPPUNIT_ASSERT(*nodes[0] == *m->getClosestKNodes()[0]);
    CPPUNIT_ASSERT(*nodes[7] == *m->getClosestKNodes()[7]);
    CPPUNIT_ASSERT_EQUAL((size_t)4, m->getValues().size());
    CPPUNIT_ASSERT(*peers[0] == *m->getValues()[0]);
    CPPUNIT_ASSERT(*peers[3] == *m->getValues()[3]);
    CPPUNIT_ASSERT_EQUAL(util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
                         util::toHex(m->getTransactionID()));
  } catch(Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void DHTMessageFactoryImplTest::testCreateAnnouncePeerMessage()
{
  try {
    Dict dict;
    dict.put("t", String::g(transactionID, DHT_TRANSACTION_ID_LENGTH));
    dict.put("y", "q");
    dict.put("q", "announce_peer");
    SharedHandle<Dict> aDict = Dict::g();
    aDict->put("id", String::g(remoteNodeID, DHT_ID_LENGTH));
    unsigned char infoHash[DHT_ID_LENGTH];
    memset(infoHash, 0x11, DHT_ID_LENGTH);
    aDict->put("info_hash", String::g(infoHash, DHT_ID_LENGTH));
    std::string token = "ffff";
    uint16_t port = 6881;
    aDict->put("port", Integer::g(port));
    aDict->put("token", token);
    dict.put("a", aDict);
  
    SharedHandle<DHTAnnouncePeerMessage> m
      (dynamic_pointer_cast<DHTAnnouncePeerMessage>
       (factory->createQueryMessage(&dict, "192.168.0.1", 6882)));
    SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
    remoteNode->setIPAddress("192.168.0.1");
    remoteNode->setPort(6882);

    CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
    CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(token, m->getToken());
    CPPUNIT_ASSERT_EQUAL(util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
                         util::toHex(m->getTransactionID()));
    CPPUNIT_ASSERT_EQUAL(util::toHex(infoHash, DHT_ID_LENGTH),
                         util::toHex(m->getInfoHash(), DHT_ID_LENGTH));
    CPPUNIT_ASSERT_EQUAL(port, m->getTCPPort());
  } catch(Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void DHTMessageFactoryImplTest::testCreateAnnouncePeerReplyMessage()
{
  Dict dict;
  dict.put("t", String::g(transactionID, DHT_TRANSACTION_ID_LENGTH));
  dict.put("y", "r");
  SharedHandle<Dict> rDict = Dict::g();
  rDict->put("id", String::g(remoteNodeID, DHT_ID_LENGTH));
  dict.put("r", rDict);

  SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);
  
  SharedHandle<DHTAnnouncePeerReplyMessage> m
    (dynamic_pointer_cast<DHTAnnouncePeerReplyMessage>
     (factory->createResponseMessage("announce_peer", &dict,
                                     remoteNode->getIPAddress(),
                                     remoteNode->getPort())));

  CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
  CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
                       util::toHex(m->getTransactionID()));
}

void DHTMessageFactoryImplTest::testReceivedErrorMessage()
{
  Dict dict;
  dict.put("t", String::g(transactionID, DHT_TRANSACTION_ID_LENGTH));
  dict.put("y", "e");
  SharedHandle<List> list = List::g();
  list->append(Integer::g(404));
  list->append("Not found");
  dict.put("e", list);

  SharedHandle<DHTNode> remoteNode(new DHTNode(remoteNodeID));
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  try {
    factory->createResponseMessage("announce_peer", &dict,
                                   remoteNode->getIPAddress(),
                                   remoteNode->getPort());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

} // namespace aria2
