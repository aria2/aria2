#include "DHTAnnouncePeerMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "Exception.h"
#include "Util.h"
#include "MockDHTMessageFactory.h"
#include "MockDHTMessageDispatcher.h"
#include "MockDHTMessage.h"
#include "DHTPeerAnnounceStorage.h"
#include "bencode.h"

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
    virtual SharedHandle<DHTMessage>
    createAnnouncePeerReplyMessage(const SharedHandle<DHTNode>& remoteNode,
				   const std::string& transactionID)
    {
      return SharedHandle<DHTMessage>(new MockDHTMessage(_localNode, remoteNode, "announce_peer", transactionID));
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

  std::string msgbody = msg.getBencodedMessage();

  BDE dict = BDE::dict();
  dict["t"] = transactionID;
  dict["y"] = BDE("q");
  dict["q"] = BDE("announce_peer");
  BDE aDict = BDE::dict();
  aDict["id"] = BDE(localNode->getID(), DHT_ID_LENGTH);
  aDict["info_hash"] = BDE(infoHash, DHT_ID_LENGTH);
  aDict["port"] = port;
  aDict["token"] = token;
  dict["a"] = aDict;

  CPPUNIT_ASSERT_EQUAL(util::urlencode(bencode::encode(dict)),
		       util::urlencode(msgbody));
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
  msg.setPeerAnnounceStorage(WeakHandle<DHTPeerAnnounceStorage>
			     (&peerAnnounceStorage));
  msg.setMessageFactory(WeakHandle<DHTMessageFactory>(&factory));
  msg.setMessageDispatcher(WeakHandle<DHTMessageDispatcher>(&dispatcher));

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher._messageQueue.size());
  SharedHandle<MockDHTMessage> m
    (dynamic_pointer_cast<MockDHTMessage>(dispatcher._messageQueue[0]._message));
  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(std::string("announce_peer"), m->getMessageType());
  CPPUNIT_ASSERT_EQUAL(transactionID, m->getTransactionID());
  std::deque<SharedHandle<Peer> > peers;
  peerAnnounceStorage.getPeers(peers, infoHash);
  CPPUNIT_ASSERT_EQUAL((size_t)1, peers.size());
  {
    SharedHandle<Peer> peer = peers[0];
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->ipaddr);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, peer->port);
  }
}

} // namespace aria2
