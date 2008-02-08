#include "DHTPeerAnnounceStorage.h"
#include "Exception.h"
#include "Util.h"
#include "MockBtContext.h"
#include "DHTConstants.h"
#include "Peer.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHTPeerAnnounceStorageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTPeerAnnounceStorageTest);
  CPPUNIT_TEST(testAddAnnounce);
  CPPUNIT_TEST(testRemovePeerAnnounce);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testAddAnnounce();
  void testRemovePeerAnnounce();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTPeerAnnounceStorageTest);

void DHTPeerAnnounceStorageTest::testAddAnnounce()
{
  unsigned char infohash1[DHT_ID_LENGTH];
  memset(infohash1, 0xff, DHT_ID_LENGTH);
  unsigned char infohash2[DHT_ID_LENGTH];
  memset(infohash2, 0xf0, DHT_ID_LENGTH);
  DHTPeerAnnounceStorage storage;

  storage.addPeerAnnounce(infohash1, "192.168.0.1", 6881);
  storage.addPeerAnnounce(infohash1, "192.168.0.2", 6882);
  storage.addPeerAnnounce(infohash2, "192.168.0.3", 6883);
  storage.addPeerAnnounce(infohash2, "192.168.0.4", 6884);
  
  std::deque<SharedHandle<Peer> > peers = storage.getPeers(infohash2);

  CPPUNIT_ASSERT_EQUAL((size_t)2, peers.size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.3"), peers[0]->ipaddr);
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.4"), peers[1]->ipaddr);
}

void DHTPeerAnnounceStorageTest::testRemovePeerAnnounce()
{
  unsigned char infohash1[DHT_ID_LENGTH];
  memset(infohash1, 0xff, DHT_ID_LENGTH);
  unsigned char infohash2[DHT_ID_LENGTH];
  memset(infohash2, 0xf0, DHT_ID_LENGTH);
  DHTPeerAnnounceStorage storage;

  SharedHandle<MockBtContext> ctx1 = new MockBtContext();
  ctx1->setInfoHash(infohash1);

  SharedHandle<MockBtContext> ctx2 = new MockBtContext();
  ctx2->setInfoHash(infohash2);

  storage.addPeerAnnounce(infohash1, "192.168.0.1", 6881);
  storage.addPeerAnnounce(ctx1);
  storage.addPeerAnnounce(ctx2);

  storage.removePeerAnnounce(ctx2);
  CPPUNIT_ASSERT(!storage.contains(infohash2));

  storage.removePeerAnnounce(ctx1);
  CPPUNIT_ASSERT(storage.contains(infohash1));
}

} // namespace aria2
