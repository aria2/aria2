#include "DHTAnnouncePeerMessage.h"
#include "DHTNode.h"
#include "DHTUtil.h"
#include "BencodeVisitor.h"
#include "Dictionary.h"
#include "Data.h"
#include "Exception.h"
#include "Util.h"
#include "MockDHTMessageFactory.h"
#include "MockDHTMessageDispatcher.h"
#include "MockDHTMessage.h"
#include "DHTPeerAnnounceStorage.h"
#include <cppunit/extensions/HelperMacros.h>

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
      return new MockDHTMessage(_localNode, remoteNode, "announce_peer", transactionID);
    }
  };
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTAnnouncePeerMessageTest);

void DHTAnnouncePeerMessageTest::testGetBencodedMessage()
{
  SharedHandle<DHTNode> localNode = new DHTNode();
  SharedHandle<DHTNode> remoteNode = new DHTNode();

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char infoHash[DHT_ID_LENGTH];
  DHTUtil::generateRandomData(infoHash, DHT_ID_LENGTH);

  std::string token = "token";
  uint16_t port = 6881;

  DHTAnnouncePeerMessage msg(localNode, remoteNode, infoHash, port, token, transactionID);

  std::string msgbody = msg.getBencodedMessage();

  SharedHandle<Dictionary> cm = new Dictionary();
  cm->put("t", new Data(transactionID));
  cm->put("y", new Data("q"));
  cm->put("q", new Data("announce_peer"));
  Dictionary* a = new Dictionary();
  cm->put("a", a);
  a->put("id", new Data(localNode->getID(), DHT_ID_LENGTH));
  a->put("info_hash", new Data(infoHash, DHT_ID_LENGTH));
  a->put("port", new Data(Util::uitos(port), true));
  a->put("token", new Data(token));

  BencodeVisitor v;
  cm->accept(&v);

  CPPUNIT_ASSERT_EQUAL(Util::urlencode(v.getBencodedData()),
		       Util::urlencode(msgbody));
}

void DHTAnnouncePeerMessageTest::testDoReceivedAction()
{
  SharedHandle<DHTNode> localNode = new DHTNode();
  SharedHandle<DHTNode> remoteNode = new DHTNode();
  remoteNode->setIPAddress("192.168.0.1");
  remoteNode->setPort(6881);

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char infoHash[DHT_ID_LENGTH];
  DHTUtil::generateRandomData(infoHash, DHT_ID_LENGTH);

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

  CPPUNIT_ASSERT_EQUAL((size_t)1, dispatcher._messageQueue.size());
  SharedHandle<MockDHTMessage> m = dispatcher._messageQueue[0]._message;
  CPPUNIT_ASSERT(localNode == m->getLocalNode());
  CPPUNIT_ASSERT(remoteNode == m->getRemoteNode());
  CPPUNIT_ASSERT_EQUAL(std::string("announce_peer"), m->getMessageType());
  CPPUNIT_ASSERT_EQUAL(transactionID, m->getTransactionID());
  std::deque<SharedHandle<Peer> > peers = peerAnnounceStorage.getPeers(infoHash);
  CPPUNIT_ASSERT_EQUAL((size_t)1, peers.size());
  {
    SharedHandle<Peer> peer = peers[0];
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->ipaddr);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, peer->port);
  }
}

} // namespace aria2
