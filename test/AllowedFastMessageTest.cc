#include "AllowedFastMessage.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class AllowedFastMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(AllowedFastMessageTest);
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


CPPUNIT_TEST_SUITE_REGISTRATION(AllowedFastMessageTest);

void AllowedFastMessageTest::testCreate() {
  char msg[9];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, 17);
  PeerMessageUtil::setIntParam(&msg[5], 12345);
  AllowedFastMessage* pm = AllowedFastMessage::create(&msg[4], 5);
  CPPUNIT_ASSERT_EQUAL(17, pm->getId());
  CPPUNIT_ASSERT_EQUAL(12345, pm->getIndex());

  // case: payload size is wrong
  try {
    char msg[10];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 6, 17);
    AllowedFastMessage::create(&msg[4], 6);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    char msg[9];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 5, 18);
    AllowedFastMessage::create(&msg[4], 5);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void AllowedFastMessageTest::testGetMessage() {
  AllowedFastMessage msg;
  msg.setIndex(12345);
  char data[9];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 5, 17);
  PeerMessageUtil::setIntParam(&data[5], 12345);
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 9) == 0);
}
