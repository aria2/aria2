#include "DHTMessageFactoryImpl.h"
#include "Exception.h"
#include "Util.h"
#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "Dictionary.h"
#include "Data.h"
#include "List.h"
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
#include <cppunit/extensions/HelperMacros.h>

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
  DHTMessageFactoryImplTest():factory(0), routingTable(0), localNode(0) {}

  DHTMessageFactoryImpl* factory;

  SharedHandle<DHTRoutingTable> routingTable;

  SharedHandle<DHTNode> localNode;

  unsigned char transactionID[DHT_TRANSACTION_ID_LENGTH];

  unsigned char remoteNodeID[DHT_ID_LENGTH];

  void setUp()
  {
    localNode = new DHTNode();
    factory = new DHTMessageFactoryImpl();
    factory->setLocalNode(localNode);
    memset(transactionID, 0xff, DHT_TRANSACTION_ID_LENGTH);
    memset(remoteNodeID, 0x0f, DHT_ID_LENGTH);
    routingTable = new DHTRoutingTable(localNode);
    factory->setRoutingTable(routingTable);
  }

  void tearDown()
  {
    delete factory;
  }

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
  SharedHandle<Dictionary> d = new Dictionary();
  d->put("t", new Data(transactionID, DHT_TRANSACTION_ID_LENGTH));
  d->put("y", new Data("q"));
  d->put("q", new Data("ping"));
  Dictionary* a = new Dictionary();
  a->put("id", new Data(remoteNodeID, DHT_ID_LENGTH));
  d->put("a", a);
  
  SharedHandle<DHTPingMessage> m = factory->createQueryMessage(d.get(), "192.168.0.1", 6881);
  SharedHandle<DHTNode> remoteNode = new DHTNode(remoteNodeID);
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
		       Util::toHex(m->getTransactionID()));
}

void DHTMessageFactoryImplTest::testCreatePingReplyMessage()
{
  SharedHandle<Dictionary> d = new Dictionary();
  d->put("t", new Data(transactionID, DHT_TRANSACTION_ID_LENGTH));
  d->put("y", new Data("r"));
  Dictionary* r = new Dictionary();
  r->put("id", new Data(remoteNodeID, DHT_ID_LENGTH));
  d->put("r", r);

  SharedHandle<DHTNode> remoteNode = new DHTNode(remoteNodeID);
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);
  
  SharedHandle<DHTPingReplyMessage> m = factory->createResponseMessage("ping", d.get(), remoteNode);

  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
		       Util::toHex(m->getTransactionID()));
}

void DHTMessageFactoryImplTest::testCreateFindNodeMessage()
{
  SharedHandle<Dictionary> d = new Dictionary();
  d->put("t", new Data(transactionID, DHT_TRANSACTION_ID_LENGTH));
  d->put("y", new Data("q"));
  d->put("q", new Data("find_node"));
  Dictionary* a = new Dictionary();
  a->put("id", new Data(remoteNodeID, DHT_ID_LENGTH));
  unsigned char targetNodeID[DHT_ID_LENGTH];
  memset(targetNodeID, 0x11, DHT_ID_LENGTH);
  a->put("target", new Data(targetNodeID, DHT_ID_LENGTH));
  d->put("a", a);
  
  SharedHandle<DHTFindNodeMessage> m = factory->createQueryMessage(d.get(), "192.168.0.1", 6881);
  SharedHandle<DHTNode> remoteNode = new DHTNode(remoteNodeID);
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
    SharedHandle<Dictionary> d = new Dictionary();
    d->put("t", new Data(transactionID, DHT_TRANSACTION_ID_LENGTH));
    d->put("y", new Data("r"));
    Dictionary* r = new Dictionary();
    r->put("id", new Data(remoteNodeID, DHT_ID_LENGTH));
    std::string compactNodeInfo;
    SharedHandle<DHTNode> nodes[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    for(size_t i = 0; i < DHTBucket::K; ++i) {
      nodes[i] = new DHTNode();
      nodes[i]->setIPAddress("192.168.0."+Util::uitos(i+1));
      nodes[i]->setPort(6881+i);

      char buf[6];
      CPPUNIT_ASSERT(PeerMessageUtil::createcompact(buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
      compactNodeInfo +=
	std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
	std::string(&buf[0], &buf[sizeof(buf)]);
    }
    r->put("nodes", new Data(compactNodeInfo));
    d->put("r", r);

    SharedHandle<DHTNode> remoteNode = new DHTNode(remoteNodeID);
    remoteNode->setIPAddress("192.168.0.1");
    remoteNode->setPort(6881);
  
    SharedHandle<DHTFindNodeReplyMessage> m = factory->createResponseMessage("find_node", d.get(), remoteNode);

    CPPUNIT_ASSERT(localNode == m->getLocalNode());
    CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL((size_t)DHTBucket::K, m->getClosestKNodes().size());
    CPPUNIT_ASSERT(nodes[0] == m->getClosestKNodes()[0]);
    CPPUNIT_ASSERT(nodes[7] == m->getClosestKNodes()[7]);
    CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
			 Util::toHex(m->getTransactionID()));
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    CPPUNIT_FAIL("exception thrown.");
  }
}

void DHTMessageFactoryImplTest::testCreateGetPeersMessage()
{
  SharedHandle<Dictionary> d = new Dictionary();
  d->put("t", new Data(transactionID, DHT_TRANSACTION_ID_LENGTH));
  d->put("y", new Data("q"));
  d->put("q", new Data("get_peers"));
  Dictionary* a = new Dictionary();
  a->put("id", new Data(remoteNodeID, DHT_ID_LENGTH));
  unsigned char infoHash[DHT_ID_LENGTH];
  memset(infoHash, 0x11, DHT_ID_LENGTH);
  a->put("info_hash", new Data(infoHash, DHT_ID_LENGTH));
  d->put("a", a);
  
  SharedHandle<DHTGetPeersMessage> m = factory->createQueryMessage(d.get(), "192.168.0.1", 6881);
  SharedHandle<DHTNode> remoteNode = new DHTNode(remoteNodeID);
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
    SharedHandle<Dictionary> d = new Dictionary();
    d->put("t", new Data(transactionID, DHT_TRANSACTION_ID_LENGTH));
    d->put("y", new Data("r"));
    Dictionary* r = new Dictionary();
    r->put("id", new Data(remoteNodeID, DHT_ID_LENGTH));
    std::string compactNodeInfo;
    SharedHandle<DHTNode> nodes[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    for(size_t i = 0; i < DHTBucket::K; ++i) {
      nodes[i] = new DHTNode();
      nodes[i]->setIPAddress("192.168.0."+Util::uitos(i+1));
      nodes[i]->setPort(6881+i);

      char buf[6];
      CPPUNIT_ASSERT(PeerMessageUtil::createcompact(buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
      compactNodeInfo +=
	std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
	std::string(&buf[0], &buf[sizeof(buf)]);
    }
    r->put("nodes", new Data(compactNodeInfo));
    r->put("token", new Data("token"));
    d->put("r", r);

    SharedHandle<DHTNode> remoteNode = new DHTNode(remoteNodeID);
    remoteNode->setIPAddress("192.168.0.1");
    remoteNode->setPort(6881);
  
    SharedHandle<DHTGetPeersReplyMessage> m = factory->createResponseMessage("get_peers", d.get(), remoteNode);

    CPPUNIT_ASSERT(localNode == m->getLocalNode());
    CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(std::string("token"), m->getToken());
    CPPUNIT_ASSERT_EQUAL((size_t)DHTBucket::K, m->getClosestKNodes().size());
    CPPUNIT_ASSERT(nodes[0] == m->getClosestKNodes()[0]);
    CPPUNIT_ASSERT(nodes[7] == m->getClosestKNodes()[7]);
    CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
			 Util::toHex(m->getTransactionID()));
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    CPPUNIT_FAIL("exception thrown.");
  }
}

void DHTMessageFactoryImplTest::testCreateGetPeersReplyMessage_values()
{
  try {
    SharedHandle<Dictionary> d = new Dictionary();
    d->put("t", new Data(transactionID, DHT_TRANSACTION_ID_LENGTH));
    d->put("y", new Data("r"));
    Dictionary* r = new Dictionary();
    r->put("id", new Data(remoteNodeID, DHT_ID_LENGTH));

    std::deque<SharedHandle<Peer> > peers;
    List* values = new List();
    r->put("values", values);
    for(size_t i = 0; i < 4; ++i) {
      SharedHandle<Peer> peer = new Peer("192.168.0."+Util::uitos(i+1), 6881+i);
      char buffer[6];
      CPPUNIT_ASSERT(PeerMessageUtil::createcompact(buffer, peer->ipaddr, peer->port));
      values->add(new Data(buffer, sizeof(buffer)));
      peers.push_back(peer);
    }
    r->put("values", values);
    r->put("token", new Data("token"));
    d->put("r", r);

    SharedHandle<DHTNode> remoteNode = new DHTNode(remoteNodeID);
    remoteNode->setIPAddress("192.168.0.1");
    remoteNode->setPort(6881);
  
    SharedHandle<DHTGetPeersReplyMessage> m = factory->createResponseMessage("get_peers", d.get(), remoteNode);

    CPPUNIT_ASSERT(localNode == m->getLocalNode());
    CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(std::string("token"), m->getToken());
    CPPUNIT_ASSERT_EQUAL((size_t)4, m->getValues().size());
    CPPUNIT_ASSERT(peers[0] == m->getValues()[0]);
    CPPUNIT_ASSERT(peers[3] == m->getValues()[3]);
    CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
			 Util::toHex(m->getTransactionID()));
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    CPPUNIT_FAIL("exception thrown.");
  }
}

void DHTMessageFactoryImplTest::testCreateAnnouncePeerMessage()
{
  try {
    SharedHandle<Dictionary> d = new Dictionary();
    d->put("t", new Data(transactionID, DHT_TRANSACTION_ID_LENGTH));
    d->put("y", new Data("q"));
    d->put("q", new Data("announce_peer"));
    Dictionary* a = new Dictionary();
    a->put("id", new Data(remoteNodeID, DHT_ID_LENGTH));
    unsigned char infoHash[DHT_ID_LENGTH];
    memset(infoHash, 0x11, DHT_ID_LENGTH);
    a->put("info_hash", new Data(infoHash, DHT_ID_LENGTH));
    std::string token = "ffff";
    uint16_t port = 6881;
    a->put("port", new Data(Util::uitos(port), true));
    a->put("token", new Data(token));
    d->put("a", a);
  
    SharedHandle<DHTAnnouncePeerMessage> m = factory->createQueryMessage(d.get(), "192.168.0.1", 6882);
    SharedHandle<DHTNode> remoteNode = new DHTNode(remoteNodeID);
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
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    std::string msg = e->getMsg();
    delete e;
    CPPUNIT_FAIL(msg);
  }
}

void DHTMessageFactoryImplTest::testCreateAnnouncePeerReplyMessage()
{
  SharedHandle<Dictionary> d = new Dictionary();
  d->put("t", new Data(transactionID, DHT_TRANSACTION_ID_LENGTH));
  d->put("y", new Data("r"));
  Dictionary* r = new Dictionary();
  r->put("id", new Data(remoteNodeID, DHT_ID_LENGTH));
  d->put("r", r);

  SharedHandle<DHTNode> remoteNode = new DHTNode(remoteNodeID);
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);
  
  SharedHandle<DHTAnnouncePeerReplyMessage> m = factory->createResponseMessage("announce_peer", d.get(), remoteNode);

  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(Util::toHex(transactionID, DHT_TRANSACTION_ID_LENGTH),
		       Util::toHex(m->getTransactionID()));
}

void DHTMessageFactoryImplTest::testReceivedErrorMessage()
{
  SharedHandle<Dictionary> d = new Dictionary();
  d->put("t", new Data(transactionID, DHT_TRANSACTION_ID_LENGTH));
  d->put("y", new Data("e"));
  List* l = new List();
  l->add(new Data("404"));
  l->add(new Data("Not found"));
  d->put("e", l);

  SharedHandle<DHTNode> remoteNode = new DHTNode(remoteNodeID);
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  try {
    factory->createResponseMessage("announce_peer", d.get(), remoteNode);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException* e) {
    std::cerr << *e << std::endl;
  }
}

} // namespace aria2
