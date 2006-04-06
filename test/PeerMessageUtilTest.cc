#include "PeerMessageUtil.h"
#include <netinet/in.h>
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class PeerMessageUtilTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PeerMessageUtilTest);
  CPPUNIT_TEST(testCreatePeerMessageKeepAlive);
  CPPUNIT_TEST(testCreatePeerMessageChoke);
  CPPUNIT_TEST(testCreatePeerMessageUnchoke);
  CPPUNIT_TEST(testCreatePeerMessageInterested);
  CPPUNIT_TEST(testCreatePeerMessageNotInterested);
  CPPUNIT_TEST(testCreatePeerMessageHave);
  CPPUNIT_TEST(testCreatePeerMessageBitfield);
  CPPUNIT_TEST(testCreatePeerMessageRequest);
  CPPUNIT_TEST(testCreatePeerMessagePiece);
  CPPUNIT_TEST(testCreatePeerMessageCancel);
  CPPUNIT_TEST(testCheckIntegrityHave);
  CPPUNIT_TEST(testCheckIntegrityBitfield);
  CPPUNIT_TEST(testCheckIntegrityRequest);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreatePeerMessageKeepAlive();
  void testCreatePeerMessageChoke();
  void testCreatePeerMessageUnchoke();
  void testCreatePeerMessageInterested();
  void testCreatePeerMessageNotInterested();
  void testCreatePeerMessageHave();
  void testCreatePeerMessageBitfield();
  void testCreatePeerMessageRequest();
  void testCreatePeerMessagePiece();
  void testCreatePeerMessageCancel();

  void testCheckIntegrityHave();
  void testCheckIntegrityBitfield();
  void testCheckIntegrityRequest();
};


CPPUNIT_TEST_SUITE_REGISTRATION( PeerMessageUtilTest );

void setIntParam(char* dest, int param) {
  int nParam = htonl(param);
  memcpy(dest, &nParam, 4);
}

void createNLengthMessage(char* msg, int msgLen, int payloadLen, int id) {
  memset(msg, 0, msgLen);
  setIntParam(msg, payloadLen);
  msg[4] = (char)id;
}

void PeerMessageUtilTest::testCreatePeerMessageKeepAlive() {
  char msg[4];
  memset(msg, 0, sizeof(msg));
  PeerMessage* pm = PeerMessageUtil::createPeerMessage(NULL, 0);
  CPPUNIT_ASSERT_EQUAL((int)PeerMessage::KEEP_ALIVE, pm->getId());
}

void PeerMessageUtilTest::testCreatePeerMessageChoke() {
  char msg[5];
  createNLengthMessage(msg, sizeof(msg), 1, 0);
  PeerMessage* pm = PeerMessageUtil::createPeerMessage(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL((int)PeerMessage::CHOKE, pm->getId());

  try {
    char msg[6];
    createNLengthMessage(msg, sizeof(msg), 2, 0);
    PeerMessageUtil::createPeerMessage(&msg[4], 2);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(...) {
  }
}

void PeerMessageUtilTest::testCreatePeerMessageUnchoke() {
  char msg[5];
  createNLengthMessage(msg, sizeof(msg), 1, 1);
  PeerMessage* pm = PeerMessageUtil::createPeerMessage(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL((int)PeerMessage::UNCHOKE, pm->getId());

  try {
    char msg[6];
    createNLengthMessage(msg, sizeof(msg), 2, 1);
    PeerMessageUtil::createPeerMessage(&msg[4], 2);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(...) {
  }
}
  
void PeerMessageUtilTest::testCreatePeerMessageInterested() {
  char msg[5];
  createNLengthMessage(msg, sizeof(msg), 1, 2);
  PeerMessage* pm = PeerMessageUtil::createPeerMessage(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL((int)PeerMessage::INTERESTED, pm->getId());

  try {
    char msg[6];
    createNLengthMessage(msg, sizeof(msg), 2, 2);
    PeerMessageUtil::createPeerMessage(&msg[4], 2);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(...) {
  }
}

void PeerMessageUtilTest::testCreatePeerMessageNotInterested() {
  char msg[5];
  createNLengthMessage(msg, sizeof(msg), 1, 3);
  PeerMessage* pm = PeerMessageUtil::createPeerMessage(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL((int)PeerMessage::NOT_INTERESTED, pm->getId());

  try {
    char msg[6];
    createNLengthMessage(msg, sizeof(msg), 2, 3);
    PeerMessageUtil::createPeerMessage(&msg[4], 2);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(...) {
  }
}

void PeerMessageUtilTest::testCreatePeerMessageHave() {
  char msg[9];
  createNLengthMessage(msg, sizeof(msg), 5, 4);
  setIntParam(&msg[5], 100);
  PeerMessage* pm = PeerMessageUtil::createPeerMessage(&msg[4], 5);
  CPPUNIT_ASSERT_EQUAL((int)PeerMessage::HAVE, pm->getId());
  CPPUNIT_ASSERT_EQUAL(100, pm->getIndex());

  try {
    char msg[8];
    createNLengthMessage(msg, sizeof(msg), 4, 4);
    PeerMessageUtil::createPeerMessage(&msg[4], 4);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(...) {}
  
  try {
    char msg[5];
    createNLengthMessage(msg, sizeof(msg), 1, 4);
    PeerMessageUtil::createPeerMessage(&msg[4], 1);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(...) {}
}

void PeerMessageUtilTest::testCreatePeerMessageBitfield() {
  int msgLen = 5+2;
  char* msg = new char[msgLen];
  createNLengthMessage(msg, msgLen, 3, 5);
  PeerMessage* pm = PeerMessageUtil::createPeerMessage(&msg[4], 3);
  CPPUNIT_ASSERT_EQUAL((int)PeerMessage::BITFIELD, pm->getId());
  CPPUNIT_ASSERT_EQUAL((unsigned char)0, pm->getBitfield()[0]);
  CPPUNIT_ASSERT_EQUAL((unsigned char)0, pm->getBitfield()[1]);
  CPPUNIT_ASSERT_EQUAL(2, pm->getBitfieldLength());

  try {
    int msgLen = 5;
    char* msg = new char[msgLen];
    createNLengthMessage(msg, msgLen, 1, 5);
    PeerMessageUtil::createPeerMessage(&msg[4], 1);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(...) {}
}

void PeerMessageUtilTest::testCreatePeerMessageRequest() {
  char msg[17];
  createNLengthMessage(msg, sizeof(msg), 13, 6);
  setIntParam(&msg[5], 1);
  setIntParam(&msg[9], 16*1024);
  setIntParam(&msg[13], 16*1024-1);
  PeerMessage* pm = PeerMessageUtil::createPeerMessage(&msg[4], 13);
  CPPUNIT_ASSERT_EQUAL((int)PeerMessage::REQUEST, pm->getId());
  CPPUNIT_ASSERT_EQUAL(1, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL(16*1024, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL(16*1024-1, pm->getLength());

  try {
    char msg[13];
    createNLengthMessage(msg, sizeof(msg), 9, 6);
    setIntParam(&msg[5], 1);
    setIntParam(&msg[9], 16*1024);
    PeerMessageUtil::createPeerMessage(&msg[4], 9);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(...) {}
}

void PeerMessageUtilTest::testCreatePeerMessagePiece() {
  char msg[23];
  createNLengthMessage(msg, sizeof(msg), 9+10, 7);
  setIntParam(&msg[5], 1);
  setIntParam(&msg[9], 16*1024);
  PeerMessage* pm = PeerMessageUtil::createPeerMessage(&msg[4], 19);
  CPPUNIT_ASSERT_EQUAL((int)PeerMessage::PIECE, pm->getId());
  CPPUNIT_ASSERT_EQUAL(1, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL(16*1024, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL(10, pm->getBlockLength());
  for(int i = 0; i < 10; i++) {
    CPPUNIT_ASSERT_EQUAL((char)0, pm->getBlock()[i]);
  }

  try {
    char msg[13];
    createNLengthMessage(msg, sizeof(msg), 9, 7);
    setIntParam(&msg[5], 1);
    setIntParam(&msg[9], 16*1024);
    PeerMessageUtil::createPeerMessage(&msg[4], 9);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(...) {}
} 

void PeerMessageUtilTest::testCreatePeerMessageCancel() {
  char msg[17];
  createNLengthMessage(msg, sizeof(msg), 13, 8);
  setIntParam(&msg[5], 1);
  setIntParam(&msg[9], 16*1024);
  setIntParam(&msg[13], 16*1024-1);
  PeerMessage* pm = PeerMessageUtil::createPeerMessage(&msg[4], 13);
  CPPUNIT_ASSERT_EQUAL((int)PeerMessage::CANCEL, pm->getId());
  CPPUNIT_ASSERT_EQUAL(1, pm->getIndex());
  CPPUNIT_ASSERT_EQUAL(16*1024, pm->getBegin());
  CPPUNIT_ASSERT_EQUAL(16*1024-1, pm->getLength());

  try {
    char msg[13];
    createNLengthMessage(msg, sizeof(msg), 9, 8);
    setIntParam(&msg[5], 1);
    setIntParam(&msg[9], 16*1024);
    PeerMessageUtil::createPeerMessage(&msg[4], 9);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(...) {}
}

void PeerMessageUtilTest::testCheckIntegrityHave() {
  PeerMessage* pm = new PeerMessage();
  pm->setId(PeerMessage::HAVE);
  pm->setIndex(119);
  try {
    PeerMessageUtil::checkIntegrity(pm, 256*1024, 120, 256*1024*120);
  } catch(Exception* ex) {
    cerr << ex->getMsg() << endl;
    CPPUNIT_FAIL("");
  } 
 
  pm->setIndex(120);
  try {
    PeerMessageUtil::checkIntegrity(pm, 256*1024, 120, 256*1024*120);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(...) {}
}


void PeerMessageUtilTest::testCheckIntegrityBitfield() {
  PeerMessage* pm = new PeerMessage();
  pm->setId(PeerMessage::BITFIELD);
  int bitfieldLength = 15;
  unsigned char* bitfield = new unsigned char[bitfieldLength];
  memset(bitfield, 0xff, bitfieldLength);
  pm->setBitfield(bitfield, bitfieldLength);
  try {
    PeerMessageUtil::checkIntegrity(pm, 256*1024, 120, 256*1024*120);
  } catch(Exception* ex) {
    cerr << ex->getMsg() << endl;
    CPPUNIT_FAIL("");
  }
  delete [] bitfield;
  bitfieldLength = 16;
  bitfield = new unsigned char[bitfieldLength];
  memset(bitfield, 0xff, bitfieldLength);
  pm->setBitfield(bitfield, bitfieldLength);
  try {
    PeerMessageUtil::checkIntegrity(pm, 256*1024, 120, 256*1024*120);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(Exception* ex) {
  }
  delete [] bitfield;
  bitfieldLength = 14;
  bitfield = new unsigned char[bitfieldLength];
  memset(bitfield, 0xff, bitfieldLength);
  pm->setBitfield(bitfield, bitfieldLength);
  try {
    PeerMessageUtil::checkIntegrity(pm, 256*1024, 120, 256*1024*120);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(Exception* ex) {
  }
  delete [] bitfield;
  bitfieldLength = 15;
  bitfield = new unsigned char[bitfieldLength];
  memset(bitfield, 0xff, bitfieldLength);
  bitfield[bitfieldLength-1] &= 0xfe;
  pm->setBitfield(bitfield, bitfieldLength);
  try {
    PeerMessageUtil::checkIntegrity(pm, 256*1024, 119, 256*1024*120);
  } catch(Exception* ex) {
    cerr << ex->getMsg() << endl;
    CPPUNIT_FAIL("");
  }
  delete [] bitfield;
  bitfieldLength = 15;
  bitfield = new unsigned char[bitfieldLength];
  memset(bitfield, 0xff, bitfieldLength);
  pm->setBitfield(bitfield, bitfieldLength);
  try {
    PeerMessageUtil::checkIntegrity(pm, 256*1024, 119, 256*1024*120);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(Exception* ex) {
  }  
  delete [] bitfield;
}

void PeerMessageUtilTest::testCheckIntegrityRequest() {
  PeerMessage* pm = new PeerMessage();
  pm->setId(PeerMessage::REQUEST);
  pm->setIndex(119);
  pm->setBegin(0);
  pm->setLength(16*1024);

  try {
    PeerMessageUtil::checkIntegrity(pm, 256*1024, 120, 256*1024*120);
  } catch(Exception* ex) {
    cerr << ex->getMsg() << endl;
    CPPUNIT_FAIL("");
  }

  pm->setBegin(256*1024);
  pm->setLength(16*1024);
  try {
    PeerMessageUtil::checkIntegrity(pm, 256*1024, 120, 256*1024*120);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(Exception* ex) {}

  pm->setBegin(0);
  pm->setLength(256*1024);
  try {
    PeerMessageUtil::checkIntegrity(pm, 256*1024, 120, 256*1024*120);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(Exception* ex) {}

  pm->setBegin(0);
  pm->setLength(5);
  try {
    PeerMessageUtil::checkIntegrity(pm, 256*1024, 120, 256*1024*120);
    CPPUNIT_FAIL("exception must be throwed.");
  } catch(Exception* ex) {}
}
