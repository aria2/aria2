#include "BtSuggestPieceMessage.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtSuggestPieceMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtSuggestPieceMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreate();
  void testGetMessage();
  void testToString();
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtSuggestPieceMessageTest);

void BtSuggestPieceMessageTest::testCreate() {
  unsigned char msg[9];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, 13);
  PeerMessageUtil::setIntParam(&msg[5], 12345);
  BtSuggestPieceMessageHandle pm = BtSuggestPieceMessage::create(&msg[4], 5);
  CPPUNIT_ASSERT_EQUAL((uint8_t)13, pm->getId());
  CPPUNIT_ASSERT_EQUAL(12345, pm->getIndex());

  // case: payload size is wrong
  try {
    unsigned char msg[10];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 6, 13);
    BtSuggestPieceMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[9];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, 14);
    BtSuggestPieceMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void BtSuggestPieceMessageTest::testGetMessage() {
  BtSuggestPieceMessage msg;
  msg.setIndex(12345);
  unsigned char data[9];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 5, 13);
  PeerMessageUtil::setIntParam(&data[5], 12345);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 9) == 0);
}

void BtSuggestPieceMessageTest::testToString() {
  BtSuggestPieceMessage msg;
  msg.setIndex(12345);

  CPPUNIT_ASSERT_EQUAL(string("suggest piece index=12345"),
		       msg.toString());
}
