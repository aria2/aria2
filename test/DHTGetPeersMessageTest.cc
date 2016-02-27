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

class DHTGetPeersMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTGetPeersMessageTest);
  CPPUNIT_TEST(testGetBencodedMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST_SUITE_END();

public:
  std::shared_ptr<DHTNode> localNode_;
  std::shared_ptr<DHTNode> remoteNode_;

  void setUp()
  {
    localNode_ = std::make_shared<DHTNode>();
    remoteNode_ = std::make_shared<DHTNode>();
  }

  void tearDown() {}

  void testGetBencodedMessage();
  void testDoReceivedAction();

  class MockDHTMessageFactory2 : public MockDHTMessageFactory {
  public:
    virtual std::unique_ptr<DHTGetPeersReplyMessage> createGetPeersReplyMessage(
        const std::shared_ptr<DHTNode>& remoteNode,
        std::vector<std::shared_ptr<DHTNode>> closestKNodes,
        std::vector<std::shared_ptr<Peer>> peers, const std::string& token,
        const std::string& transactionID) CXX11_OVERRIDE
    {
      auto m = make_unique<DHTGetPeersReplyMessage>(
          AF_INET, localNode_, remoteNode, token, transactionID);
      m->setClosestKNodes(closestKNodes);
      m->setValues(peers);
      return m;
    }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(DHTGetPeersMessageTest);

void DHTGetPeersMessageTest::testGetBencodedMessage()
{
  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char infoHash[DHT_ID_LENGTH];
  util::generateRandomData(infoHash, DHT_ID_LENGTH);

  DHTGetPeersMessage msg(localNode_, remoteNode_, infoHash, transactionID);
  msg.setVersion("A200");

  std::string msgbody = msg.getBencodedMessage();

  Dict dict;
  dict.put("t", transactionID);
  dict.put("v", "A200");
  dict.put("y", "q");
  dict.put("q", "get_peers");
  auto aDict = Dict::g();
  aDict->put("id", String::g(localNode_->getID(), DHT_ID_LENGTH));
  aDict->put("info_hash", String::g(infoHash, DHT_ID_LENGTH));
  dict.put("a", std::move(aDict));

  CPPUNIT_ASSERT_EQUAL(util::percentEncode(bencode2::encode(&dict)),
                       util::percentEncode(msgbody));
}

void DHTGetPeersMessageTest::testDoReceivedAction()
{
  remoteNode_->setIPAddress("192.168.0.1");
  remoteNode_->setPort(6881);

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char infoHash[DHT_ID_LENGTH];
  util::generateRandomData(infoHash, DHT_ID_LENGTH);

  DHTTokenTracker tokenTracker;
  MockDHTMessageDispatcher dispatcher;
  MockDHTMessageFactory2 factory;
  factory.setLocalNode(localNode_);
  DHTRoutingTable routingTable(localNode_);

  DHTGetPeersMessage msg(localNode_, remoteNode_, infoHash, transactionID);
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
    auto m = dynamic_cast<DHTGetPeersReplyMessage*>(
        dispatcher.messageQueue_[0].message_.get());
    CPPUNIT_ASSERT(*localNode_ == *m->getLocalNode());
    CPPUNIT_ASSERT(*remoteNode_ == *m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(std::string("get_peers"), m->getMessageType());
    CPPUNIT_ASSERT_EQUAL(msg.getTransactionID(), m->getTransactionID());
    CPPUNIT_ASSERT_EQUAL(tokenTracker.generateToken(infoHash,
                                                    remoteNode_->getIPAddress(),
                                                    remoteNode_->getPort()),
                         m->getToken());
    CPPUNIT_ASSERT_EQUAL((size_t)0, m->getClosestKNodes().size());
    CPPUNIT_ASSERT_EQUAL((size_t)2, m->getValues().size());
    {
      auto peer = m->getValues()[0];
      CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.100"), peer->getIPAddress());
      CPPUNIT_ASSERT_EQUAL((uint16_t)6888, peer->getPort());
    }
    {
      auto peer = m->getValues()[1];
      CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.101"), peer->getIPAddress());
      CPPUNIT_ASSERT_EQUAL((uint16_t)6889, peer->getPort());
    }
  }
  dispatcher.messageQueue_.clear();
  {
    // localhost doesn't have peer contact information for that infohash.
    DHTPeerAnnounceStorage peerAnnounceStorage;
    DHTRoutingTable routingTable(localNode_);
    std::shared_ptr<DHTNode> returnNode1(new DHTNode());
    routingTable.addNode(returnNode1);

    msg.setPeerAnnounceStorage(&peerAnnounceStorage);
    msg.setRoutingTable(&routingTable);

    msg.doReceivedAction();

    CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher.messageQueue_.size());
    auto m = dynamic_cast<DHTGetPeersReplyMessage*>(
        dispatcher.messageQueue_[0].message_.get());
    CPPUNIT_ASSERT(*localNode_ == *m->getLocalNode());
    CPPUNIT_ASSERT(*remoteNode_ == *m->getRemoteNode());
    CPPUNIT_ASSERT_EQUAL(std::string("get_peers"), m->getMessageType());
    CPPUNIT_ASSERT_EQUAL(msg.getTransactionID(), m->getTransactionID());
    CPPUNIT_ASSERT_EQUAL(tokenTracker.generateToken(infoHash,
                                                    remoteNode_->getIPAddress(),
                                                    remoteNode_->getPort()),
                         m->getToken());
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getClosestKNodes().size());
    CPPUNIT_ASSERT(*returnNode1 == *m->getClosestKNodes()[0]);
    CPPUNIT_ASSERT_EQUAL((size_t)0, m->getValues().size());
  }
}

} // namespace aria2
