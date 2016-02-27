#include "DHTAnnouncePeerMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "Exception.h"
#include "util.h"
#include "MockDHTMessageFactory.h"
#include "MockDHTMessageDispatcher.h"
#include "MockDHTMessage.h"
#include "DHTPeerAnnounceStorage.h"
#include "bencode2.h"

namespace aria2 {

class DHTAnnouncePeerMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTAnnouncePeerMessageTest);
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
    virtual std::unique_ptr<DHTAnnouncePeerReplyMessage>
    createAnnouncePeerReplyMessage(const std::shared_ptr<DHTNode>& remoteNode,
                                   const std::string& transactionID)
        CXX11_OVERRIDE
    {
      return make_unique<DHTAnnouncePeerReplyMessage>(localNode_, remoteNode,
                                                      transactionID);
    }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(DHTAnnouncePeerMessageTest);

void DHTAnnouncePeerMessageTest::testGetBencodedMessage()
{
  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char infoHash[DHT_ID_LENGTH];
  util::generateRandomData(infoHash, DHT_ID_LENGTH);

  std::string token = "token";
  uint16_t port = 6881;

  DHTAnnouncePeerMessage msg(localNode_, remoteNode_, infoHash, port, token,
                             transactionID);
  msg.setVersion("A200");
  std::string msgbody = msg.getBencodedMessage();

  Dict dict;
  dict.put("t", transactionID);
  dict.put("v", "A200");
  dict.put("y", "q");
  dict.put("q", "announce_peer");
  auto aDict = Dict::g();
  aDict->put("id", String::g(localNode_->getID(), DHT_ID_LENGTH));
  aDict->put("info_hash", String::g(infoHash, DHT_ID_LENGTH));
  aDict->put("port", Integer::g(port));
  aDict->put("token", token);
  dict.put("a", std::move(aDict));

  CPPUNIT_ASSERT_EQUAL(util::percentEncode(bencode2::encode(&dict)),
                       util::percentEncode(msgbody));
}

void DHTAnnouncePeerMessageTest::testDoReceivedAction()
{
  remoteNode_->setIPAddress("192.168.0.1");
  remoteNode_->setPort(6881);

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char infoHash[DHT_ID_LENGTH];
  util::generateRandomData(infoHash, DHT_ID_LENGTH);

  std::string token = "token";
  uint16_t port = 6882;

  DHTPeerAnnounceStorage peerAnnounceStorage;
  MockDHTMessageFactory2 factory;
  factory.setLocalNode(localNode_);
  MockDHTMessageDispatcher dispatcher;

  DHTAnnouncePeerMessage msg(localNode_, remoteNode_, infoHash, port, token,
                             transactionID);
  msg.setPeerAnnounceStorage(&peerAnnounceStorage);
  msg.setMessageFactory(&factory);
  msg.setMessageDispatcher(&dispatcher);

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher.messageQueue_.size());
  auto m = dynamic_cast<DHTAnnouncePeerReplyMessage*>(
      dispatcher.messageQueue_[0].message_.get());
  CPPUNIT_ASSERT(*localNode_ == *m->getLocalNode());
  CPPUNIT_ASSERT(*remoteNode_ == *m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(std::string("announce_peer"), m->getMessageType());
  CPPUNIT_ASSERT_EQUAL(transactionID, m->getTransactionID());
  std::vector<std::shared_ptr<Peer>> peers;
  peerAnnounceStorage.getPeers(peers, infoHash);
  CPPUNIT_ASSERT_EQUAL((size_t)1, peers.size());
  {
    std::shared_ptr<Peer> peer = peers[0];
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->getIPAddress());
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, peer->getPort());
  }
}

} // namespace aria2
