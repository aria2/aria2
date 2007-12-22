#include "BtExtendedMessage.h"
#include "PeerMessageUtil.h"
#include "MockBtContext.h"
#include "MockExtensionMessageFactory.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtExtendedMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtExtendedMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp()
  {
    BtRegistry::unregisterAll();
  }

  void tearDown()
  {
    BtRegistry::unregisterAll();
  }

  void testCreate();
  void testGetMessage();
  void testDoReceivedAction();
  void testToString();
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtExtendedMessageTest);

void BtExtendedMessageTest::testCreate() {
  PeerHandle peer = new Peer("192.168.0.1", 6969, 16*1024, 256*1024);
  peer->setExtension("charlie", 1);
  MockBtContextHandle ctx = new MockBtContext();
  unsigned char infohash[20];
  memset(infohash, 0, sizeof(infohash));
  ctx->setInfoHash(infohash);
  MockExtensionMessageFactoryHandle exmsgFactory = new MockExtensionMessageFactory();
  

  BtRegistry::registerPeerObjectCluster(ctx->getInfoHashAsString(), new PeerObjectCluster());
  PeerObjectHandle peerObject = new PeerObject();
  peerObject->extensionMessageFactory = exmsgFactory;

  PEER_OBJECT_CLUSTER(ctx)->registerHandle(peer->getId(), peerObject);

  // payload:{4:name3:foo}->11bytes
  string payload = "4:name3:foo";
  char msg[17];// 6+11bytes
  PeerMessageUtil::createPeerMessageString((unsigned char*)msg, sizeof(msg), 13, 20);
  msg[5] = 1; // Set dummy extended message ID 1
  memcpy(msg+6, payload.c_str(), payload.size());
  BtExtendedMessageHandle pm = BtExtendedMessage::create(ctx,
							 peer,
							 &msg[4], 13);
  CPPUNIT_ASSERT_EQUAL((int8_t)20, pm->getId());
  
  // case: payload size is wrong
  try {
    char msg[5];
    PeerMessageUtil::createPeerMessageString((unsigned char*)msg, sizeof(msg), 1, 20);
    BtExtendedMessage::create(ctx, peer, &msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e << endl;
    delete e;
  }
  // case: id is wrong
  try {
    char msg[6];
    PeerMessageUtil::createPeerMessageString((unsigned char*)msg, sizeof(msg), 2, 21);
    BtExtendedMessage::create(ctx, peer, &msg[4], 2);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e << endl;
    delete e;
  }
}

void BtExtendedMessageTest::testGetMessage() {
  string payload = "4:name3:foo";
  uint8_t extendedMessageID = 1;
  MockExtensionMessageHandle exmsg =
    new MockExtensionMessage("charlie", extendedMessageID,
			     payload.c_str(),
			     payload.size());
  BtExtendedMessage msg(exmsg);

  char data[17];
  PeerMessageUtil::createPeerMessageString((unsigned char*)data, sizeof(data), 13, 20);
  *(data+5) = extendedMessageID;
  memcpy(data+6, payload.c_str(), payload.size());
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 17) == 0);
}

void BtExtendedMessageTest::testDoReceivedAction() {
  MockExtensionMessageHandle exmsg =
    new MockExtensionMessage("charlie", 1, "", 0);
  BtExtendedMessage msg(exmsg);
  msg.doReceivedAction();
  CPPUNIT_ASSERT(exmsg->_doReceivedActionCalled);
}
  
void BtExtendedMessageTest::testToString() {
  string payload = "4:name3:foo";
  uint8_t extendedMessageID = 1;
  MockExtensionMessageHandle exmsg =
    new MockExtensionMessage("charlie", extendedMessageID,
			     payload.c_str(),
			     payload.size());
  BtExtendedMessage msg(exmsg);
  CPPUNIT_ASSERT_EQUAL(string("extended charlie"), msg.toString());
}
