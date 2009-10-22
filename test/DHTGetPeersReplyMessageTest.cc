#include "DHTGetPeersReplyMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "Exception.h"
#include "util.h"
#include "DHTBucket.h"
#include "bittorrent_helper.h"
#include "Peer.h"
#include "bencode.h"

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
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  std::string token = "token";

  DHTGetPeersReplyMessage msg(localNode, remoteNode, token, transactionID);

  BDE dict = BDE::dict();
  dict["t"] = transactionID;
  dict["y"] = BDE("r");
  BDE rDict = BDE::dict();
  rDict["id"] = BDE(localNode->getID(), DHT_ID_LENGTH);
  rDict["token"] = token;
  dict["r"] = rDict;
  {
    std::string compactNodeInfo;
    SharedHandle<DHTNode> nodes[8];
    for(size_t i = 0; i < DHTBucket::K; ++i) {
      nodes[i].reset(new DHTNode());
      nodes[i]->setIPAddress("192.168.0."+util::uitos(i+1));
      nodes[i]->setPort(6881+i);
      
      unsigned char buf[6];
      CPPUNIT_ASSERT(bittorrent::createcompact
		     (buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
      compactNodeInfo +=
	std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
	std::string(&buf[0], &buf[sizeof(buf)]);
    }
    msg.setClosestKNodes
      (std::deque<SharedHandle<DHTNode> >(&nodes[0], &nodes[DHTBucket::K]));

    std::string msgbody = msg.getBencodedMessage();

    rDict["nodes"] = compactNodeInfo;

    CPPUNIT_ASSERT_EQUAL(util::urlencode(bencode::encode(dict)),
			 util::urlencode(msgbody));
  }
  rDict.removeKey("nodes");
  {
    std::deque<SharedHandle<Peer> > peers;
    BDE valuesList = BDE::list();
    for(size_t i = 0; i < 4; ++i) {
      SharedHandle<Peer> peer(new Peer("192.168.0."+util::uitos(i+1), 6881+i));
      unsigned char buffer[6];
      CPPUNIT_ASSERT(bittorrent::createcompact
		     (buffer, peer->ipaddr, peer->port));
      valuesList << BDE(buffer, sizeof(buffer));
      peers.push_back(peer);
    }
    rDict["values"] = valuesList;

    msg.setValues(peers);
    std::string msgbody  = msg.getBencodedMessage();

    CPPUNIT_ASSERT_EQUAL(util::urlencode(bencode::encode(dict)),
			 util::urlencode(msgbody));
  }
}

} // namespace aria2
