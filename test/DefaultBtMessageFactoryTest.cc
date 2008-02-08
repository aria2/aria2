#include "DefaultBtMessageFactory.h"
#include "Peer.h"
#include "PeerMessageUtil.h"
#include "BtRegistry.h"
#include "MockBtContext.h"
#include "MockExtensionMessageFactory.h"
#include "BtExtendedMessage.h"
#include "BtPortMessage.h"
#include "PeerObject.h"
#include "BtRequestFactory.h"
#include "BtMessageDispatcher.h"
#include "BtMessageReceiver.h"
#include "PeerConnection.h"
#include "Exception.h"
#include "FileEntry.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DefaultBtMessageFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtMessageFactoryTest);
  CPPUNIT_TEST(testCreateBtMessage_BtExtendedMessage);
  CPPUNIT_TEST(testCreatePortMessage);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<MockBtContext> _btContext;
  SharedHandle<Peer> _peer;
public:
  DefaultBtMessageFactoryTest():_btContext(0), _peer(0) {}

  void setUp()
  {
    BtRegistry::unregisterAll();
    SharedHandle<MockBtContext> btContext = new MockBtContext();
    unsigned char infohash[20];
    memset(infohash, 0, sizeof(infohash));
    btContext->setInfoHash(infohash);
    _btContext = btContext;

    _peer = new Peer("192.168.0.1", 6969);
    _peer->setExtendedMessagingEnabled(true);

    SharedHandle<MockExtensionMessageFactory> exmsgFactory =
      new MockExtensionMessageFactory();
    BtRegistry::registerPeerObjectCluster(_btContext->getInfoHashAsString(),
					  new PeerObjectCluster());
    SharedHandle<PeerObject> peerObject = new PeerObject();
    peerObject->extensionMessageFactory = exmsgFactory;

    PEER_OBJECT_CLUSTER(_btContext)->registerHandle(_peer->getId(), peerObject);
  }

  void tearDown()
  {
    BtRegistry::unregisterAll();
  }

  void testCreateBtMessage_BtExtendedMessage();
  void testCreatePortMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtMessageFactoryTest);

void DefaultBtMessageFactoryTest::testCreateBtMessage_BtExtendedMessage()
{
  
  DefaultBtMessageFactory factory;
  factory.setBtContext(_btContext);
  factory.setPeer(_peer);
  
  // payload:{4:name3:foo}->11bytes
  std::string payload = "4:name3:foo";
  char msg[17];// 6+11bytes
  PeerMessageUtil::createPeerMessageString((unsigned char*)msg, sizeof(msg), 13, 20);
  msg[5] = 1; // Set dummy extended message ID 1
  memcpy(msg+6, payload.c_str(), payload.size());
  
  SharedHandle<BtExtendedMessage> m =
    factory.createBtMessage((const unsigned char*)msg+4, sizeof(msg));

  try {
    // disable extended messaging
    _peer->setExtendedMessagingEnabled(false);
    factory.createBtMessage((const unsigned char*)msg+4, sizeof(msg));
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    delete e;
  }
}

void DefaultBtMessageFactoryTest::testCreatePortMessage()
{
  DefaultBtMessageFactory factory;
  factory.setBtContext(_btContext);
  factory.setPeer(_peer);

  {
    unsigned char data[7];
    PeerMessageUtil::createPeerMessageString(data, sizeof(data), 3, 9);
    PeerMessageUtil::setShortIntParam(&data[5], 6881);
    try {
      SharedHandle<BtPortMessage> m = factory.createBtMessage(&data[4], sizeof(data)-4);
      CPPUNIT_ASSERT(!m.isNull());
      CPPUNIT_ASSERT_EQUAL((uint16_t)6881, m->getPort());
    } catch(Exception* e) {
      std::cerr << *e << std::endl;
      std::string msg = e->getMsg();
      delete e;
      CPPUNIT_FAIL(msg);
    }
  }
  {
    SharedHandle<BtPortMessage> m = factory.createPortMessage(6881);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, m->getPort());
  }
}

} // namespace aria2
