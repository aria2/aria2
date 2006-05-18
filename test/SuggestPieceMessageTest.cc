#include "SuggestPieceMessage.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class SuggestPieceMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SuggestPieceMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreate();
  void testGetMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(SuggestPieceMessageTest);

void SuggestPieceMessageTest::testCreate() {
  char msg[9];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, 13);
  PeerMessageUtil::setIntParam(&msg[5], 12345);
  SuggestPieceMessage* pm = SuggestPieceMessage::create(&msg[4], 5);
  CPPUNIT_ASSERT_EQUAL(13, pm->getId());
  CPPUNIT_ASSERT_EQUAL(12345, pm->getIndex());

  // case: payload size is wrong
  try {
    char msg[10];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 6, 13);
    SuggestPieceMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    char msg[9];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, 14);
    SuggestPieceMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void SuggestPieceMessageTest::testGetMessage() {
  SuggestPieceMessage msg;
  msg.setIndex(12345);
  char data[9];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 5, 13);
  PeerMessageUtil::setIntParam(&data[5], 12345);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 9) == 0);
}
