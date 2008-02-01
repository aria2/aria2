#include "DHTGetPeersReplyMessage.h"
#include "DHTNode.h"
#include "DHTUtil.h"
#include "BencodeVisitor.h"
#include "Dictionary.h"
#include "Data.h"
#include "Exception.h"
#include "Util.h"
#include "DHTBucket.h"
#include "PeerMessageUtil.h"
#include "List.h"
#include "Peer.h"
#include <cppunit/extensions/HelperMacros.h>

class DHTGetPeersReplyMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTGetPeersReplyMessageTest);
  CPPUNIT_TEST(testGetBencodedMessage);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGetBencodedMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTGetPeersReplyMessageTest);

void DHTGetPeersReplyMessageTest::testGetBencodedMessage()
{
  DHTNodeHandle localNode = new DHTNode();
  DHTNodeHandle remoteNode = new DHTNode();

  char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  string token = "token";

  DHTGetPeersReplyMessage msg(localNode, remoteNode, token, transactionID);

  SharedHandle<Dictionary> cm = new Dictionary();
  cm->put("t", new Data(transactionID));
  cm->put("y", new Data("r"));
  Dictionary* r = new Dictionary();
  cm->put("r", r);
  r->put("id", new Data(reinterpret_cast<const char*>(localNode->getID()), DHT_ID_LENGTH));
  r->put("token", new Data(token));

  {
    string compactNodeInfo;
    DHTNodeHandle nodes[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    for(size_t i = 0; i < DHTBucket::K; ++i) {
      nodes[i] = new DHTNode();
      nodes[i]->setIPAddress("192.168.0."+Util::uitos(i+1));
      nodes[i]->setPort(6881+i);
      
      char buf[6];
      CPPUNIT_ASSERT(PeerMessageUtil::createcompact(buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
      compactNodeInfo +=
	string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
	string(&buf[0], &buf[sizeof(buf)]);
    }
    msg.setClosestKNodes(DHTNodes(&nodes[0], &nodes[DHTBucket::K]));

    string msgbody = msg.getBencodedMessage();

    r->put("nodes", new Data(compactNodeInfo));

    BencodeVisitor v;
    cm->accept(&v);

    CPPUNIT_ASSERT_EQUAL(Util::urlencode(v.getBencodedData()),
			 Util::urlencode(msgbody));
  }
  r->remove("nodes");
  {
    Peers peers;
    List* values = new List();
    r->put("values", values);
    for(size_t i = 0; i < 4; ++i) {
      PeerHandle peer = new Peer("192.168.0."+Util::uitos(i+1), 6881+i);
      char buffer[6];
      CPPUNIT_ASSERT(PeerMessageUtil::createcompact(buffer, peer->ipaddr, peer->port));
      values->add(new Data(buffer, sizeof(buffer)));
      peers.push_back(peer);
    }
    msg.setValues(peers);
    string msgbody  = msg.getBencodedMessage();
    BencodeVisitor v;
    cm->accept(&v);
    CPPUNIT_ASSERT_EQUAL(Util::urlencode(v.getBencodedData()),
			 Util::urlencode(msgbody));
  }
}
