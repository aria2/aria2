#include "PeerConnection.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "Peer.h"
#include "SocketCore.h"

namespace aria2 {

class PeerConnectionTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PeerConnectionTest);
  CPPUNIT_TEST(testReserveBuffer);
  CPPUNIT_TEST_SUITE_END();

public:
  void testReserveBuffer();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PeerConnectionTest);

void PeerConnectionTest::testReserveBuffer()
{
  PeerConnection con(1, std::shared_ptr<Peer>(), std::shared_ptr<SocketCore>());
  con.presetBuffer((unsigned char*)"foo", 3);
  CPPUNIT_ASSERT_EQUAL((size_t)MAX_BUFFER_CAPACITY, con.getBufferCapacity());
  CPPUNIT_ASSERT_EQUAL((size_t)3, con.getBufferLength());

  constexpr size_t newLength = 128_k;
  con.reserveBuffer(newLength);

  CPPUNIT_ASSERT_EQUAL(newLength, con.getBufferCapacity());
  CPPUNIT_ASSERT_EQUAL((size_t)3, con.getBufferLength());
  CPPUNIT_ASSERT(memcmp("foo", con.getBuffer(), 3) == 0);
}

} // namespace aria2
