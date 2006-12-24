#include "BtAllowedFastMessage.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtAllowedFastMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtAllowedFastMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testOnSendComplete);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreate();
  void testGetMessage();
  void testDoReceivedAction();
  void testOnSendComplete();
  void testToString();
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtAllowedFastMessageTest);

void BtAllowedFastMessageTest::testCreate() {
  unsigned char msg[9];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, 17);
  PeerMessageUtil::setIntParam(&msg[5], 12345);
  BtAllowedFastMessageHandle pm = BtAllowedFastMessage::create(&msg[4], 5);
  CPPUNIT_ASSERT_EQUAL(17, pm->getId());
  CPPUNIT_ASSERT_EQUAL((uint32_t)12345, pm->getIndex());

  // case: payload size is wrong
  try {
    unsigned char msg[10];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 6, 17);
    BtAllowedFastMessage::create(&msg[4], 6);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[9];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, 18);
    BtAllowedFastMessage::create(&msg[4], 5);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void BtAllowedFastMessageTest::testGetMessage() {
  BtAllowedFastMessage msg;
  msg.setIndex(12345);
  unsigned char data[9];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 5, 17);
  PeerMessageUtil::setIntParam(&data[5], 12345);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 9) == 0);
}

void BtAllowedFastMessageTest::testDoReceivedAction() {
  BtAllowedFastMessage msg;
  msg.setIndex(1);
  PeerHandle peer = new Peer("localhost", 6969, 16*1024, 256*1024*1024);
  peer->setFastExtensionEnabled(true);
  msg.setPeer(peer);
  CPPUNIT_ASSERT(!peer->isInPeerAllowedIndexSet(1));
  msg.doReceivedAction();
  CPPUNIT_ASSERT(peer->isInPeerAllowedIndexSet(1));

  peer->setFastExtensionEnabled(false);
  try {
    msg.doReceivedAction();
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {}
}

void BtAllowedFastMessageTest::testOnSendComplete() {
  BtAllowedFastMessage msg;
  msg.setIndex(1);
  PeerHandle peer = new Peer("localhost", 6969, 16*1024, 256*1024*1024);
  peer->setFastExtensionEnabled(true);
  msg.setPeer(peer);
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(1));
  msg.onSendComplete();
  CPPUNIT_ASSERT(peer->isInAmAllowedIndexSet(1));
}

void BtAllowedFastMessageTest::testToString() {
  BtAllowedFastMessage msg;
  msg.setIndex(1);
  CPPUNIT_ASSERT_EQUAL(string("allowed fast index=1"), msg.toString());
}
