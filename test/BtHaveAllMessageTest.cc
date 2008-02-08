#include "BtHaveAllMessage.h"
#include "PeerMessageUtil.h"
#include "Peer.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class BtHaveAllMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtHaveAllMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreate();
  void testGetMessage();
  void testDoReceivedAction();
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtHaveAllMessageTest);

void BtHaveAllMessageTest::testCreate() {
  unsigned char msg[5];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 14);
  SharedHandle<BtHaveAllMessage> pm = BtHaveAllMessage::create(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL((int8_t)14, pm->getId());

  // case: payload size is wrong
  try {
    unsigned char msg[6];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 2, 14);
    BtHaveAllMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[5];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 15);
    BtHaveAllMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {
  }
}

void BtHaveAllMessageTest::testGetMessage() {
  BtHaveAllMessage msg;
  unsigned char data[5];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 1, 14);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 5) == 0);
}

void BtHaveAllMessageTest::testDoReceivedAction() {
  BtHaveAllMessage msg;
  SharedHandle<Peer> peer = new Peer("host", 6969);
  peer->allocateBitfield(16*1024, 256*1024);
  peer->setFastExtensionEnabled(true);
  msg.setPeer(peer);
  
  msg.doReceivedAction();
  
  CPPUNIT_ASSERT(peer->isSeeder());
  
  peer->setFastExtensionEnabled(false);
  
  try {
    msg.doReceivedAction();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(...) {}
}

} // namespace aria2
