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

class DHTAnnouncePeerMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTAnnouncePeerMessageTest);
  CPPUNIT_TEST(testGetBencodedMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGetBencodedMessage();
  void testDoReceivedAction();

  class MockDHTMessageFactory2:public MockDHTMessageFactory {
    virtual SharedHandle<DHTResponseMessage>
    createAnnouncePeerReplyMessage(const SharedHandle<DHTNode>& remoteNode,
                                   const std::string& transactionID)
    {
      return SharedHandle<DHTResponseMessage>
        (new MockDHTResponseMessage
         (localNode_, remoteNode, "announce_peer", transactionID));
    }
  };
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTAnnouncePeerMessageTest);

void DHTAnnouncePeerMessageTest::testGetBencodedMessage()
{
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char infoHash[DHT_ID_LENGTH];
  util::generateRandomData(infoHash, DHT_ID_LENGTH);

  std::string token = "token";
  uint16_t port = 6881;

  DHTAnnouncePeerMessage msg(localNode, remoteNode, infoHash, port, token, transactionID);
  msg.setVersion("A200");
  std::string msgbody = msg.getBencodedMessage();

  Dict dict;
  dict.put("t", transactionID);
  dict.put("v", "A200");
  dict.put("y", "q");
  dict.put("q", "announce_peer");
  SharedHandle<Dict> aDict = Dict::g();
  aDict->put("id", String::g(localNode->getID(), DHT_ID_LENGTH));
  aDict->put("info_hash", String::g(infoHash, DHT_ID_LENGTH));
  aDict->put("port", Integer::g(port));
  aDict->put("token", token);
  dict.put("a", aDict);

  CPPUNIT_ASSERT_EQUAL(util::percentEncode(bencode2::encode(&dict)),
                       util::percentEncode(msgbody));
}

void DHTAnnouncePeerMessageTest::testDoReceivedAction()
{
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char infoHash[DHT_ID_LENGTH];
  util::generateRandomData(infoHash, DHT_ID_LENGTH);

  std::string token = "token";
  uint16_t port = 6882;

  DHTPeerAnnounceStorage peerAnnounceStorage;
  MockDHTMessageFactory2 factory;
  factory.setLocalNode(localNode);
  MockDHTMessageDispatcher dispatcher;

  DHTAnnouncePeerMessage msg(localNode, remoteNode, infoHash, port, token, transactionID);
  msg.setPeerAnnounceStorage(&peerAnnounceStorage);
  msg.setMessageFactory(&factory);
  msg.setMessageDispatcher(&dispatcher);

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher.messageQueue_.size());
  SharedHandle<MockDHTResponseMessage> m
    (dynamic_pointer_cast<MockDHTResponseMessage>
     (dispatcher.messageQueue_[0].message_));
  CPPUNIT_ASSERT(*localNode == *m->getLocalNode());
  CPPUNIT_ASSERT(*remoteNode == *m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(std::string("announce_peer"), m->getMessageType());
  CPPUNIT_ASSERT_EQUAL(transactionID, m->getTransactionID());
  std::vector<SharedHandle<Peer> > peers;
  peerAnnounceStorage.getPeers(peers, infoHash);
  CPPUNIT_ASSERT_EQUAL((size_t)1, peers.size());
  {
    SharedHandle<Peer> peer = peers[0];
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->getIPAddress());
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, peer->getPort());
  }
}

} // namespace aria2
