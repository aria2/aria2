#include "Peer.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class PeerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PeerTest);
  CPPUNIT_TEST(testPeerAllowedIndexSet);
  CPPUNIT_TEST(testAmAllowedIndexSet);
  CPPUNIT_TEST(testGetId);
  CPPUNIT_TEST(testOperatorEqual);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Peer> peer;
public:
  PeerTest():peer(0) {}

  void setUp() {
    peer = new Peer("localhost", 6969);
    peer->allocateSessionResource(1024, 1024*1024);
  }

  void testPeerAllowedIndexSet();
  void testAmAllowedIndexSet();
  void testGetId();
  void testOperatorEqual();
};


CPPUNIT_TEST_SUITE_REGISTRATION(PeerTest);

void PeerTest::testPeerAllowedIndexSet() {
  CPPUNIT_ASSERT(!peer->isInPeerAllowedIndexSet(0));
  peer->addPeerAllowedIndex(0);
  CPPUNIT_ASSERT(peer->isInPeerAllowedIndexSet(0));
}

void PeerTest::testAmAllowedIndexSet() {
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(0));
  peer->addAmAllowedIndex(0);
  CPPUNIT_ASSERT(peer->isInAmAllowedIndexSet(0));
}

void PeerTest::testGetId() {
  CPPUNIT_ASSERT_EQUAL(std::string("f05897fc14a41cb3400e283e189158656d7184da"),
		       peer->getID());
}

void PeerTest::testOperatorEqual()
{
  CPPUNIT_ASSERT(Peer("localhost", 6881) == Peer("localhost", 6881));

  {
    Peer p1("localhost", 6881);
    Peer p2("localhsot", 0);
    p2.port = 6881;
    CPPUNIT_ASSERT(p1 != p2);
  }
}

} // namespace aria2
