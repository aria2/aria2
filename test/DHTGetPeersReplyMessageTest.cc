#include "DHTGetPeersReplyMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "Exception.h"
#include "util.h"
#include "DHTBucket.h"
#include "bittorrent_helper.h"
#include "Peer.h"
#include "bencode2.h"

namespace aria2 {

class DHTGetPeersReplyMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTGetPeersReplyMessageTest);
  CPPUNIT_TEST(testGetBencodedMessage);
  CPPUNIT_TEST(testGetBencodedMessage6);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGetBencodedMessage();

  void testGetBencodedMessage6();
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

  DHTGetPeersReplyMessage msg
    (AF_INET, localNode, remoteNode, token, transactionID);
  msg.setVersion("A200");
  Dict dict;
  dict.put("t", transactionID);
  dict.put("v", "A200");
  dict.put("y", "r");
  SharedHandle<Dict> rDict = Dict::g();
  rDict->put("id", String::g(localNode->getID(), DHT_ID_LENGTH));
  rDict->put("token", token);
  dict.put("r", rDict);
  {
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
    msg.setClosestKNodes
      (std::vector<SharedHandle<DHTNode> >(&nodes[0], &nodes[DHTBucket::K]));
    rDict->put("nodes", compactNodeInfo);

    std::vector<SharedHandle<Peer> > peers;
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
    msg.setValues(peers);
    rDict->put("values", valuesList);

    std::string msgbody = msg.getBencodedMessage();
    CPPUNIT_ASSERT_EQUAL(util::percentEncode(bencode2::encode(&dict)),
                         util::percentEncode(msgbody));
  }
}

void DHTGetPeersReplyMessageTest::testGetBencodedMessage6()
{
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  std::string token = "token";

  DHTGetPeersReplyMessage msg
    (AF_INET6, localNode, remoteNode, token, transactionID);
  msg.setVersion("A200");
  Dict dict;
  dict.put("t", transactionID);
  dict.put("v", "A200");
  dict.put("y", "r");
  SharedHandle<Dict> rDict = Dict::g();
  rDict->put("id", String::g(localNode->getID(), DHT_ID_LENGTH));
  rDict->put("token", token);
  dict.put("r", rDict);
  {
    std::string compactNodeInfo;
    SharedHandle<DHTNode> nodes[8];
    for(size_t i = 0; i < DHTBucket::K; ++i) {
      nodes[i].reset(new DHTNode());
      nodes[i]->setIPAddress("2001::000"+util::uitos(i+1));
      nodes[i]->setPort(6881+i);
      
      unsigned char buf[COMPACT_LEN_IPV6];
      CPPUNIT_ASSERT_EQUAL
        (COMPACT_LEN_IPV6, bittorrent::packcompact
         (buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
      compactNodeInfo +=
        std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
        std::string(&buf[0], &buf[COMPACT_LEN_IPV6]);
    }
    msg.setClosestKNodes
      (std::vector<SharedHandle<DHTNode> >(&nodes[0], &nodes[DHTBucket::K]));
    rDict->put("nodes6", compactNodeInfo);

    std::vector<SharedHandle<Peer> > peers;
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
    msg.setValues(peers);
    rDict->put("values", valuesList);

    std::string msgbody = msg.getBencodedMessage();
    CPPUNIT_ASSERT_EQUAL(util::percentEncode(bencode2::encode(&dict)),
                         util::percentEncode(msgbody));
  }
}

} // namespace aria2
