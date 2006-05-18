#include "BitfieldMessage.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BitfieldMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BitfieldMessageTest);
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


CPPUNIT_TEST_SUITE_REGISTRATION(BitfieldMessageTest);

void BitfieldMessageTest::testCreate() {
  char msg[5+2];
  PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 3, 5);
  unsigned char bitfield[2];
  memset(bitfield, 0xff, sizeof(bitfield));
  memcpy(&msg[5], bitfield, sizeof(bitfield));
  BitfieldMessage* pm = BitfieldMessage::create(&msg[4], 3);
  CPPUNIT_ASSERT_EQUAL(5, pm->getId());
  CPPUNIT_ASSERT(memcmp(bitfield, pm->getBitfield(), sizeof(bitfield)) == 0);
  CPPUNIT_ASSERT_EQUAL(2, pm->getBitfieldLength());
  // case: payload size is wrong
  try {
    char msg[5];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 1, 5);
    BitfieldMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
  // case: id is wrong
  try {
    char msg[5+2];
    PeerMessageUtil::createPeerMessageString(msg, sizeof(msg), 3, 6);
    BitfieldMessage::create(&msg[4], 3);
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {
  }
}

void BitfieldMessageTest::testGetMessage() {
  BitfieldMessage msg;
  unsigned char bitfield[2];
  memset(bitfield, 0xff, sizeof(bitfield));
  msg.setBitfield(bitfield, sizeof(bitfield));
  char data[5+2];
  PeerMessageUtil::createPeerMessageString(data, sizeof(data), 3, 5);
  memcpy(&data[5], bitfield, sizeof(bitfield));
  CPPUNIT_ASSERT(memcmp(msg.getMessage(), data, 7) == 0);
}
