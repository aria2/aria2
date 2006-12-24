#include "BtPortMessage.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtPortMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtPortMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreate();
  void testToString();
};


CPPUNIT_TEST_SUITE_REGISTRATION(BtPortMessageTest);

void BtPortMessageTest::testCreate() {
  unsigned char msg[7];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 3, 9);
  PeerMessageUtil::setShortIntParam(&msg[5], 12345);
  BtPortMessageHandle pm = BtPortMessage::create(&msg[4], 3);
  CPPUNIT_ASSERT_EQUAL(9, pm->getId());
  CPPUNIT_ASSERT_EQUAL((uint16_t)12345, pm->getPort());

  // case: payload size is wrong
  try {
    unsigned char msg[8];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 4, 9);
    BtPortMessage::create(&msg[4], 4);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[7];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 3, 10);
    BtPortMessage::create(&msg[4], 3);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}
void BtPortMessageTest::testToString() {
  BtPortMessage msg(1);
  CPPUNIT_ASSERT_EQUAL(string("port port=1"), msg.toString());
}
