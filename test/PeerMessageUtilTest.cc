#include "common.h"
#include "PeerMessageUtil.h"
#include "a2netcompat.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class PeerMessageUtilTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PeerMessageUtilTest);
  CPPUNIT_TEST(testCreateCompact);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreateCompact();
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

void PeerMessageUtilTest::testCreateCompact()
{
  unsigned char compact[6];
  // Note: PeerMessageUtil::createcompact() on linux can handle IPv4-mapped
  // addresses like `ffff::127.0.0.1', but on cygwin, it doesn't.
  CPPUNIT_ASSERT(PeerMessageUtil::createcompact(compact, "127.0.0.1", 6881));

  std::pair<std::string, uint16_t> p = PeerMessageUtil::unpackcompact(compact);
  CPPUNIT_ASSERT_EQUAL(std::string("127.0.0.1"), p.first);
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, p.second);
}


} // namespace aria2
