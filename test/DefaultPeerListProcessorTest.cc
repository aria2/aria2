#include "DefaultPeerListProcessor.h"
#include "MetaFileUtil.h"
#include "Exception.h"
#include "Dictionary.h"
#include "Peer.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DefaultPeerListProcessorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultPeerListProcessorTest);
  CPPUNIT_TEST(testExtractPeer);
  CPPUNIT_TEST(testExtract2Peers);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testExtractPeer();
  void testExtract2Peers();
};


CPPUNIT_TEST_SUITE_REGISTRATION( DefaultPeerListProcessorTest );

void DefaultPeerListProcessorTest::testExtractPeer() {
  DefaultPeerListProcessor proc;
  std::string peersString = "d5:peersld2:ip11:192.168.0.17:peer id20:aria2-000000000000004:porti2006eeee";

  Dictionary* dic = (Dictionary*)MetaFileUtil::bdecoding(peersString);
  
  CPPUNIT_ASSERT(proc.canHandle(dic->get("peers")));

  std::deque<SharedHandle<Peer> > peers;
  proc.extractPeer(peers, dic->get("peers"));
  CPPUNIT_ASSERT_EQUAL((size_t)1, peers.size());
  SharedHandle<Peer> peer = *peers.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->ipaddr);
  CPPUNIT_ASSERT_EQUAL((uint16_t)2006, peer->port);
}

void DefaultPeerListProcessorTest::testExtract2Peers() {
  DefaultPeerListProcessor proc;
  std::string peersString = "d5:peersld2:ip11:192.168.0.17:peer id20:aria2-000000000000004:porti2006eed2:ip11:192.168.0.27:peer id20:aria2-000000000000004:porti2007eeee";

  Dictionary* dic = (Dictionary*)MetaFileUtil::bdecoding(peersString);

  std::deque<SharedHandle<Peer> > peers;
  proc.extractPeer(peers, dic->get("peers"));
  CPPUNIT_ASSERT_EQUAL((size_t)2, peers.size());
  SharedHandle<Peer> peer = *peers.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->ipaddr);
  CPPUNIT_ASSERT_EQUAL((uint16_t)2006, peer->port);

  peer = *(peers.begin()+1);
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), peer->ipaddr);
  CPPUNIT_ASSERT_EQUAL((uint16_t)2007, peer->port);
}

} // namespace aria2
