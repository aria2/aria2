#include "common.h"
#include "PeerMessageUtil.h"
#include "a2netcompat.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class PeerMessageUtilTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PeerMessageUtilTest);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

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
