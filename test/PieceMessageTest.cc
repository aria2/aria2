#include "PieceMessage.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class PieceMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PieceMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessageHeader);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreate();
  void testGetMessageHeader();
};


CPPUNIT_TEST_SUITE_REGISTRATION(PieceMessageTest);

void PieceMessageTest::testCreate() {
  char msg[13+2];
  char data[2];
  memset(data, 0xff, sizeof(data));
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 11, 7);
  PeerMessageUtil::setIntParam(&msg[5], 12345);
  PeerMessageUtil::setIntParam(&msg[9], 256);
  memcpy(&msg[13], data, sizeof(data));
  PieceMessage* pm = PieceMessage::create(&msg[4], 11);
  CPPUNIT_ASSERT_EQUAL(7, pm->getId());
  CPPUNIT_ASSERT_EQUAL(12345, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL(256, pm->getBegin());
  CPPUNIT_ASSERT(memcmp(data, pm->getBlock(), sizeof(data)) == 0);
  CPPUNIT_ASSERT_EQUAL(2, pm->getBlockLength());

  // case: payload size is wrong
  try {
    char msg[13];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 9, 7);
    PieceMessage::create(&msg[4], 9);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    char msg[13+2];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 11, 8);
    PieceMessage::create(&msg[4], 11);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void PieceMessageTest::testGetMessageHeader() {
  PieceMessage msg;
  msg.setIndex(12345);
  msg.setBegin(256);
  msg.setBlockLength(1024);
  char data[13];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 9+1024, 7);
  PeerMessageUtil::setIntParam(&data[5], 12345);
  PeerMessageUtil::setIntParam(&data[9], 256);
  CPPUNIT_ASSERT(memcmp(msg.getMessageHeader(), data, 13) == 0);
}
