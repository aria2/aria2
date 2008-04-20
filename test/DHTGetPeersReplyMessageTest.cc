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

namespace aria2 {

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
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  std::string token = "token";

  DHTGetPeersReplyMessage msg(localNode, remoteNode, token, transactionID);

  SharedHandle<Dictionary> cm(new Dictionary());
  cm->put("t", new Data(transactionID));
  cm->put("y", new Data("r"));
  Dictionary* r = new Dictionary();
  cm->put("r", r);
  r->put("id", new Data(localNode->getID(), DHT_ID_LENGTH));
  r->put("token", new Data(token));

  {
    std::string compactNodeInfo;
    SharedHandle<DHTNode> nodes[8];
    for(size_t i = 0; i < DHTBucket::K; ++i) {
      nodes[i].reset(new DHTNode());
      nodes[i]->setIPAddress("192.168.0."+Util::uitos(i+1));
      nodes[i]->setPort(6881+i);
      
      unsigned char buf[6];
      CPPUNIT_ASSERT(PeerMessageUtil::createcompact(buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
      compactNodeInfo +=
	std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
	std::string(&buf[0], &buf[sizeof(buf)]);
    }
    msg.setClosestKNodes(std::deque<SharedHandle<DHTNode> >(&nodes[0], &nodes[DHTBucket::K]));

    std::string msgbody = msg.getBencodedMessage();

    r->put("nodes", new Data(compactNodeInfo));

    BencodeVisitor v;
    cm->accept(&v);

    CPPUNIT_ASSERT_EQUAL(Util::urlencode(v.getBencodedData()),
			 Util::urlencode(msgbody));
  }
  r->remove("nodes");
  {
    std::deque<SharedHandle<Peer> > peers;
    List* values = new List();
    r->put("values", values);
    for(size_t i = 0; i < 4; ++i) {
      SharedHandle<Peer> peer(new Peer("192.168.0."+Util::uitos(i+1), 6881+i));
      unsigned char buffer[6];
      CPPUNIT_ASSERT(PeerMessageUtil::createcompact(buffer, peer->ipaddr, peer->port));
      values->add(new Data(buffer, sizeof(buffer)));
      peers.push_back(peer);
    }
    msg.setValues(peers);
    std::string msgbody  = msg.getBencodedMessage();
    BencodeVisitor v;
    cm->accept(&v);
    CPPUNIT_ASSERT_EQUAL(Util::urlencode(v.getBencodedData()),
			 Util::urlencode(msgbody));
  }
}

} // namespace aria2
