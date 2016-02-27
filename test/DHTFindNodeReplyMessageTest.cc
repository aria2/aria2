#include "DHTFindNodeReplyMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "Exception.h"
#include "util.h"
#include "DHTBucket.h"
#include "bittorrent_helper.h"
#include "bencode2.h"

namespace aria2 {

class DHTFindNodeReplyMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTFindNodeReplyMessageTest);
  CPPUNIT_TEST(testGetBencodedMessage);
  CPPUNIT_TEST(testGetBencodedMessage6);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testGetBencodedMessage();

  void testGetBencodedMessage6();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DHTFindNodeReplyMessageTest);

void DHTFindNodeReplyMessageTest::testGetBencodedMessage()
{
  std::shared_ptr<DHTNode> localNode(new DHTNode());
  std::shared_ptr<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  DHTFindNodeReplyMessage msg(AF_INET, localNode, remoteNode, transactionID);
  msg.setVersion("A200");
  std::string compactNodeInfo;
  std::shared_ptr<DHTNode> nodes[8];
  for (size_t i = 0; i < DHTBucket::K; ++i) {
    nodes[i].reset(new DHTNode());
    nodes[i]->setIPAddress("192.168.0." + util::uitos(i + 1));
    nodes[i]->setPort(6881 + i);

    unsigned char buf[COMPACT_LEN_IPV6];
    CPPUNIT_ASSERT_EQUAL(COMPACT_LEN_IPV4,
                         bittorrent::packcompact(buf, nodes[i]->getIPAddress(),
                                                 nodes[i]->getPort()));
    compactNodeInfo +=
        std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH]) +
        std::string(&buf[0], &buf[COMPACT_LEN_IPV4]);
  }
  msg.setClosestKNodes(
      std::vector<std::shared_ptr<DHTNode>>(&nodes[0], &nodes[DHTBucket::K]));

  std::string msgbody = msg.getBencodedMessage();

  Dict dict;
  dict.put("t", transactionID);
  dict.put("v", "A200");
  dict.put("y", "r");
  auto rDict = Dict::g();
  rDict->put("id", String::g(localNode->getID(), DHT_ID_LENGTH));
  rDict->put("nodes", compactNodeInfo);
  dict.put("r", std::move(rDict));

  CPPUNIT_ASSERT_EQUAL(bencode2::encode(&dict), msgbody);
}

void DHTFindNodeReplyMessageTest::testGetBencodedMessage6()
{
  std::shared_ptr<DHTNode> localNode(new DHTNode());
  std::shared_ptr<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  DHTFindNodeReplyMessage msg(AF_INET6, localNode, remoteNode, transactionID);
  msg.setVersion("A200");
  std::string compactNodeInfo;
  std::shared_ptr<DHTNode> nodes[8];
  for (size_t i = 0; i < DHTBucket::K; ++i) {
    nodes[i].reset(new DHTNode());
    nodes[i]->setIPAddress("2001::000" + util::uitos(i + 1));
    nodes[i]->setPort(6881 + i);

    unsigned char buf[COMPACT_LEN_IPV6];
    CPPUNIT_ASSERT_EQUAL(COMPACT_LEN_IPV6,
                         bittorrent::packcompact(buf, nodes[i]->getIPAddress(),
                                                 nodes[i]->getPort()));
    compactNodeInfo +=
        std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH]) +
        std::string(&buf[0], &buf[COMPACT_LEN_IPV6]);
  }
  msg.setClosestKNodes(
      std::vector<std::shared_ptr<DHTNode>>(&nodes[0], &nodes[DHTBucket::K]));

  std::string msgbody = msg.getBencodedMessage();

  Dict dict;
  dict.put("t", transactionID);
  dict.put("v", "A200");
  dict.put("y", "r");
  auto rDict = Dict::g();
  rDict->put("id", String::g(localNode->getID(), DHT_ID_LENGTH));
  rDict->put("nodes6", compactNodeInfo);
  dict.put("r", std::move(rDict));

  CPPUNIT_ASSERT_EQUAL(bencode2::encode(&dict), msgbody);
}

} // namespace aria2
