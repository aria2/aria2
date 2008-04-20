#include "DHTPeerAnnounceEntry.h"
#include "Exception.h"
#include "Util.h"
#include "MockBtContext.h"
#include "MockPeerStorage.h"
#include "BtRegistry.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHTPeerAnnounceEntryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTPeerAnnounceEntryTest);
  CPPUNIT_TEST(testRemoveStalePeerAddrEntry);
  CPPUNIT_TEST(testEmpty);
  CPPUNIT_TEST(testAddPeerAddrEntry);
  CPPUNIT_TEST(testGetPeers);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp()
  {
    BtRegistry::unregisterAll();
  }

  void tearDown()
  {
    BtRegistry::unregisterAll();
  }

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
  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.2", 6882, Time(0)));
  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.3", 6883));
  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.4", 6884, Time(0)));

  entry.removeStalePeerAddrEntry(10);

  CPPUNIT_ASSERT_EQUAL((size_t)2, entry.countPeerAddrEntry());

  const std::deque<PeerAddrEntry>& peerAddrEntries = entry.getPeerAddrEntries();
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
    entry.setBtContext(SharedHandle<BtContext>(new MockBtContext()));
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
  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.1", 6881, Time(0)));
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

  SharedHandle<MockBtContext> ctx(new MockBtContext());
  ctx->setInfoHash(infohash);
  SharedHandle<MockPeerStorage> peerStorage(new MockPeerStorage());
  {
    SharedHandle<Peer> activePeers[2];
    activePeers[0].reset(new Peer("192.168.0.3", 6883));
    activePeers[1].reset(new Peer("192.168.0.4", 6884));

    peerStorage->setActivePeers(std::deque<SharedHandle<Peer> >(&activePeers[0],
								&activePeers[2]));
  }
  BtRegistry::registerPeerStorage(ctx->getInfoHashAsString(), peerStorage);

  DHTPeerAnnounceEntry entry(infohash);
  {
    std::deque<SharedHandle<Peer> > peers = entry.getPeers();
    CPPUNIT_ASSERT_EQUAL((size_t)0, peers.size());
  }

  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.1", 6881, Time(0)));
  entry.addPeerAddrEntry(PeerAddrEntry("192.168.0.2", 6882));

  {
    std::deque<SharedHandle<Peer> > peers = entry.getPeers();
    CPPUNIT_ASSERT_EQUAL((size_t)2, peers.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peers[0]->ipaddr);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, peers[0]->port);
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), peers[1]->ipaddr); 
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, peers[1]->port);
  }
  entry.setBtContext(ctx);
  {
    std::deque<SharedHandle<Peer> > peers = entry.getPeers();
    CPPUNIT_ASSERT_EQUAL((size_t)4, peers.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peers[0]->ipaddr);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, peers[0]->port);
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), peers[1]->ipaddr); 
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, peers[1]->port);
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.3"), peers[2]->ipaddr);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6883, peers[2]->port);
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.4"), peers[3]->ipaddr); 
    CPPUNIT_ASSERT_EQUAL((uint16_t)6884, peers[3]->port);
  }
}

} // namespace aria2
