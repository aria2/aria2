#include "DHTPeerAnnounceEntry.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "FileEntry.h"
#include "Peer.h"

namespace aria2 {

class DHTPeerAnnounceEntryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTPeerAnnounceEntryTest);
  CPPUNIT_TEST(testRemoveStalePeerAddrEntry);
  CPPUNIT_TEST(testEmpty);
  CPPUNIT_TEST(testAddPeerAddrEntry);
  CPPUNIT_TEST(testGetPeers);
  CPPUNIT_TEST_SUITE_END();
public:
  void testRemoveStalePeerAddrEntry();
  void testEmpty();
  void testAddPeerAddrEntry();
  void testGetPeers();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTPeerAnnounceEntryTest);

void DHTPeerAnnounceEntryTest::testRemoveStalePeerAddrEntry()
{
  unsigned char infohash[DHT_ID_LENGTH];
  memset(infohash, 0xff, DHT_ID_LENGTH);
  DHTPeerAnnounceEntry entry(infohash);

  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.1", 6881));
  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.2", 6882, Timer(0)));
  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.3", 6883));
  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.4", 6884, Timer(0)));

  entry.removeStalePeerAddrEntry(10);

  CPPUNIT_ASSERT_EQUAL((size_t)2, entry.countPeerAddrEntry());

  const std::vector<PeerAddrEntry>& peerAddrEntries =
    entry.getPeerAddrEntries();
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peerAddrEntries[0].getIPAddress());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.3"), peerAddrEntries[1].getIPAddress());
}


void DHTPeerAnnounceEntryTest::testEmpty()
{
  unsigned char infohash[DHT_ID_LENGTH];
  memset(infohash, 0xff, DHT_ID_LENGTH);
  {
    DHTPeerAnnounceEntry entry(infohash);
    entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.1", 6881));
    CPPUNIT_ASSERT(!entry.empty());
  }
  {
    DHTPeerAnnounceEntry entry(infohash);
    CPPUNIT_ASSERT(entry.empty());
  }
}

void DHTPeerAnnounceEntryTest::testAddPeerAddrEntry()
{
  unsigned char infohash[DHT_ID_LENGTH];
  memset(infohash, 0xff, DHT_ID_LENGTH);

  DHTPeerAnnounceEntry entry(infohash);
  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.1", 6881, Timer(0)));
  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.1", 6882));

  CPPUNIT_ASSERT_EQUAL((size_t)2, entry.countPeerAddrEntry());
  
  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.1", 6881));

  CPPUNIT_ASSERT_EQUAL((size_t)2, entry.countPeerAddrEntry());
  CPPUNIT_ASSERT(0 != entry.getPeerAddrEntries()[0].getLastUpdated().getTime());
}

void DHTPeerAnnounceEntryTest::testGetPeers()
{
  unsigned char infohash[DHT_ID_LENGTH];
  memset(infohash, 0xff, DHT_ID_LENGTH);

  DHTPeerAnnounceEntry entry(infohash);
  {
    std::vector<SharedHandle<Peer> > peers;
    entry.getPeers(peers);
    CPPUNIT_ASSERT_EQUAL((size_t)0, peers.size());
  }

  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.1", 6881, Timer(0)));
  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.2", 6882));

  {
    std::vector<SharedHandle<Peer> > peers;
    entry.getPeers(peers);
    CPPUNIT_ASSERT_EQUAL((size_t)2, peers.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peers[0]->getIPAddress());
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, peers[0]->getPort());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), peers[1]->getIPAddress()); 
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, peers[1]->getPort());
  }
}

} // namespace aria2
