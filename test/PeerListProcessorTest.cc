#include "PeerListProcessor.h"

#include <cstdlib>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "Peer.h"
#include "bencode.h"
#include "TimeA2.h"
#include "PeerMessageUtil.h"

namespace aria2 {

class PeerListProcessorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PeerListProcessorTest);
  CPPUNIT_TEST(testExtractPeerFromList);
  CPPUNIT_TEST(testExtract2PeersFromList);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testExtractPeerFromList();
  void testExtract2PeersFromList();
};


CPPUNIT_TEST_SUITE_REGISTRATION( PeerListProcessorTest );

void PeerListProcessorTest::testExtractPeerFromList() {
  PeerListProcessor proc;
  std::string peersString = "d5:peersld2:ip11:192.168.0.17:peer id20:aria2-000000000000004:porti2006eeee";

  const bencode::BDE dict = bencode::decode(peersString);
  
  std::deque<SharedHandle<Peer> > peers;
  proc.extractPeerFromList(dict["peers"], std::back_inserter(peers));
  CPPUNIT_ASSERT_EQUAL((size_t)1, peers.size());
  SharedHandle<Peer> peer = *peers.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->ipaddr);
  CPPUNIT_ASSERT_EQUAL((uint16_t)2006, peer->port);
}

void PeerListProcessorTest::testExtract2PeersFromList() {
  PeerListProcessor proc;
  std::string peersString = "d5:peersld2:ip11:192.168.0.17:peer id20:aria2-000000000000004:porti65535eed2:ip11:192.168.0.27:peer id20:aria2-000000000000004:porti2007eeee";

  const bencode::BDE dict = bencode::decode(peersString);

  std::deque<SharedHandle<Peer> > peers;
  proc.extractPeerFromList(dict["peers"], std::back_inserter(peers));
  CPPUNIT_ASSERT_EQUAL((size_t)2, peers.size());
  SharedHandle<Peer> peer = *peers.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->ipaddr);
  CPPUNIT_ASSERT_EQUAL((uint16_t)65535, peer->port);

  peer = *(peers.begin()+1);
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), peer->ipaddr);
  CPPUNIT_ASSERT_EQUAL((uint16_t)2007, peer->port);
}

} // namespace aria2
