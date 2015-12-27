#include "Peer.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class PeerTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PeerTest);
  CPPUNIT_TEST(testPeerAllowedIndexSet);
  CPPUNIT_TEST(testAmAllowedIndexSet);
  CPPUNIT_TEST(testCountSeeder);
  CPPUNIT_TEST_SUITE_END();

private:
  std::shared_ptr<Peer> peer;

public:
  void setUp()
  {
    peer.reset(new Peer("localhost", 6969));
    peer->allocateSessionResource(1_k, 1_m);
  }

  void testPeerAllowedIndexSet();
  void testAmAllowedIndexSet();
  void testCountSeeder();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PeerTest);

void PeerTest::testPeerAllowedIndexSet()
{
  CPPUNIT_ASSERT(!peer->isInPeerAllowedIndexSet(0));
  peer->addPeerAllowedIndex(0);
  CPPUNIT_ASSERT(peer->isInPeerAllowedIndexSet(0));
}

void PeerTest::testAmAllowedIndexSet()
{
  CPPUNIT_ASSERT(!peer->isInAmAllowedIndexSet(0));
  peer->addAmAllowedIndex(0);
  CPPUNIT_ASSERT(peer->isInAmAllowedIndexSet(0));
}

void PeerTest::testCountSeeder()
{
  std::vector<std::shared_ptr<Peer>> peers(5);
  peers[0].reset(new Peer("192.168.0.1", 7000));
  peers[1].reset(new Peer("192.168.0.2", 7000));
  peers[2].reset(new Peer("192.168.0.3", 7000));
  peers[3].reset(new Peer("192.168.0.4", 7000));
  peers[4].reset(new Peer("192.168.0.5", 7000));
  for (std::vector<std::shared_ptr<Peer>>::iterator i = peers.begin();
       i != peers.end(); ++i) {
    (*i)->allocateSessionResource(1_k, 8_k);
  }
  unsigned char bitfield[] = {0xff};
  peers[1]->setBitfield(bitfield, 1);
  peers[3]->setBitfield(bitfield, 1);
  peers[4]->setBitfield(bitfield, 1);
  CPPUNIT_ASSERT_EQUAL((size_t)3, countSeeder(peers.begin(), peers.end()));
}

} // namespace aria2
