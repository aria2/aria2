#include "PeerMessageUtil.h"
#include <netinet/in.h>
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class PeerMessageUtilTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PeerMessageUtilTest);
  CPPUNIT_TEST(testCheckIntegrityHave);
  CPPUNIT_TEST(testCheckIntegrityBitfield);
  CPPUNIT_TEST(testCheckIntegrityRequest);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreatePeerMessageUnchoke();
  void testCreatePeerMessageInterested();
  void testCreatePeerMessageNotInterested();
  void testCreatePeerMessageHave();
  void testCreatePeerMessageBitfield();
  void testCreatePeerMessageRequest();
  void testCreatePeerMessagePiece();
  void testCreatePeerMessageCancel();
  void testCreatePortMessage();
  void testCheckIntegrityHave();
  void testCheckIntegrityBitfield();
  void testCheckIntegrityRequest();
};


CPPUNIT_TEST_SUITE_REGISTRATION( PeerMessageUtilTest );

void setIntParam(char* dest, int param) {
  int nParam = htonl(param);
  memcpy(dest, &nParam, 4);
}

void setShortIntParam(char* dest, int param) {
  short int nParam = htons(param);
  memcpy(dest, &nParam, 2);
}

void createNLengthMessage(char* msg, int msgLen, int payloadLen, int id) {
  memset(msg, 0, msgLen);
  setIntParam(msg, payloadLen);
  msg[4] = (char)id;
}

void PeerMessageUtilTest::testCheckIntegrityHave() {
  HaveMessage* pm = new HaveMessage();
  pm->setIndex(119);
  pm->setPieces(120);
  try {
    pm->check();
  } catch(Exception* ex) {
    cerr << ex->getMsg() << endl;
    CPPUNIT_FAIL("");
  } 
 
  pm->setIndex(120);
  try {
    pm->check();
    CPPUNIT_FAIL("exception must be threw.");
  } catch(...) {}
}


void PeerMessageUtilTest::testCheckIntegrityBitfield() {
  BitfieldMessage* pm = new BitfieldMessage();
  int bitfieldLength = 15;
  unsigned char* bitfield = new unsigned char[bitfieldLength];
  memset(bitfield, 0xff, bitfieldLength);
  pm->setBitfield(bitfield, bitfieldLength);
  pm->setPieces(120);
  try {
    pm->check();
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
    pm->check();
    CPPUNIT_FAIL("exception must be threw.");
  } catch(Exception* ex) {
  }
  delete [] bitfield;
  bitfieldLength = 14;
  bitfield = new unsigned char[bitfieldLength];
  memset(bitfield, 0xff, bitfieldLength);
  pm->setBitfield(bitfield, bitfieldLength);
  try {
    pm->check();
    CPPUNIT_FAIL("exception must be threw.");
  } catch(Exception* ex) {
  }
  delete [] bitfield;
  bitfieldLength = 15;
  bitfield = new unsigned char[bitfieldLength];
  memset(bitfield, 0xff, bitfieldLength);
  bitfield[bitfieldLength-1] &= 0xfe;
  pm->setBitfield(bitfield, bitfieldLength);
  pm->setPieces(119);
  try {
    pm->check();
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
    pm->check();
    CPPUNIT_FAIL("exception must be threw.");
  } catch(Exception* ex) {
  }  
  delete [] bitfield;
}

void PeerMessageUtilTest::testCheckIntegrityRequest() {
  RequestMessage* pm = new RequestMessage();
  pm->setIndex(119);
  pm->setBegin(0);
  pm->setLength(16*1024);
  pm->setPieces(120);
  pm->setPieceLength(256*1024);
  try {
    pm->check();
  } catch(Exception* ex) {
    cerr << ex->getMsg() << endl;
    CPPUNIT_FAIL("");
  }

  pm->setBegin(256*1024);
  pm->setLength(16*1024);
  try {
    pm->check();
    CPPUNIT_FAIL("exception must be threw.");
  } catch(Exception* ex) {}

  pm->setBegin(0);
  pm->setLength(256*1024);
  try {
    pm->check();
    CPPUNIT_FAIL("exception must be threw.");
  } catch(Exception* ex) {}

  pm->setBegin(0);
  pm->setLength(5);
  try {
    pm->check();
    CPPUNIT_FAIL("exception must be threw.");
  } catch(Exception* ex) {}
}
