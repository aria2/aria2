#include "DefaultBtMessageFactory.h"
#include "Peer.h"
#include "PeerMessageUtil.h"
#include "BtRegistry.h"
#include "MockBtContext.h"
#include "MockExtensionMessageFactory.h"
#include "BtExtendedMessage.h"
#include <cppunit/extensions/HelperMacros.h>

class DefaultBtMessageFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtMessageFactoryTest);
  CPPUNIT_TEST(testCreateBtMessage_BtExtendedMessage);
  CPPUNIT_TEST_SUITE_END();
private:
  MockBtContextHandle _btContext;
  PeerHandle _peer;
public:
  DefaultBtMessageFactoryTest():_btContext(0), _peer(0) {}

  void setUp()
  {
    BtRegistry::unregisterAll();
    MockBtContextHandle btContext = new MockBtContext();
    unsigned char infohash[20];
    memset(infohash, 0, sizeof(infohash));
    btContext->setInfoHash(infohash);
    _btContext = btContext;

    _peer = new Peer("192.168.0.1", 6969);
    _peer->setExtendedMessagingEnabled(true);

    MockExtensionMessageFactoryHandle exmsgFactory = new MockExtensionMessageFactory();
    BtRegistry::registerPeerObjectCluster(_btContext->getInfoHashAsString(),
					  new PeerObjectCluster());
    PeerObjectHandle peerObject = new PeerObject();
    peerObject->extensionMessageFactory = exmsgFactory;

    PEER_OBJECT_CLUSTER(_btContext)->registerHandle(_peer->getId(), peerObject);
  }

  void tearDown()
  {
    BtRegistry::unregisterAll();
  }

  void testCreateBtMessage_BtExtendedMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtMessageFactoryTest);

void DefaultBtMessageFactoryTest::testCreateBtMessage_BtExtendedMessage()
{
  
  DefaultBtMessageFactory factory;
  factory.setBtContext(_btContext);
  factory.setPeer(_peer);
  
  // payload:{4:name3:foo}->11bytes
  string payload = "4:name3:foo";
  char msg[17];// 6+11bytes
  PeerMessageUtil::createPeerMessageString((unsigned char*)msg, sizeof(msg), 13, 20);
  msg[5] = 1; // Set dummy extended message ID 1
  memcpy(msg+6, payload.c_str(), payload.size());
  
  BtExtendedMessageHandle m = factory.createBtMessage((const unsigned char*)msg+4, sizeof(msg));

  try {
    // disable extended messaging
    _peer->setExtendedMessagingEnabled(false);
    factory.createBtMessage((const unsigned char*)msg+4, sizeof(msg));
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e << endl;
    delete e;
  }
}
