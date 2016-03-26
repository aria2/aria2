#include "DHTPeerAnnounceStorage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "DownloadContext.h"
#include "DHTConstants.h"
#include "Peer.h"
#include "FileEntry.h"
#include "bittorrent_helper.h"

namespace aria2 {

class DHTPeerAnnounceStorageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTPeerAnnounceStorageTest);
  CPPUNIT_TEST(testAddAnnounce);
  CPPUNIT_TEST_SUITE_END();

public:
  void testAddAnnounce();
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

  std::vector<std::shared_ptr<Peer>> peers;
  storage.getPeers(peers, infohash2);

  CPPUNIT_ASSERT_EQUAL((size_t)2, peers.size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.3"), peers[0]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.4"), peers[1]->getIPAddress());
}

} // namespace aria2
