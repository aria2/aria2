#include "DHTPeerAnnounceStorage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "Util.h"
#include "DownloadContext.h"
#include "MockPeerStorage.h"
#include "DHTConstants.h"
#include "Peer.h"
#include "FileEntry.h"
#include "bittorrent_helper.h"

namespace aria2 {

class DHTPeerAnnounceStorageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTPeerAnnounceStorageTest);
  CPPUNIT_TEST(testAddAnnounce);
  CPPUNIT_TEST(testRemovePeerAnnounce);
  CPPUNIT_TEST_SUITE_END();
public:
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
  
  std::deque<SharedHandle<Peer> > peers;
  storage.getPeers(peers, infohash2);

  CPPUNIT_ASSERT_EQUAL((size_t)2, peers.size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.3"), peers[0]->ipaddr);
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.4"), peers[1]->ipaddr);
}

static SharedHandle<DownloadContext> createDownloadContext
(const unsigned char* infoHash)
{
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  BDE torrentAttrs = BDE::dict();
  torrentAttrs[bittorrent::INFO_HASH] =
    std::string(&infoHash[0], &infoHash[DHT_ID_LENGTH]);
  dctx->setAttribute(bittorrent::BITTORRENT, torrentAttrs);
  return dctx;
}

void DHTPeerAnnounceStorageTest::testRemovePeerAnnounce()
{
  unsigned char infoHash1[DHT_ID_LENGTH];
  memset(infoHash1, 0xff, DHT_ID_LENGTH);
  unsigned char infoHash2[DHT_ID_LENGTH];
  memset(infoHash2, 0xf0, DHT_ID_LENGTH);
  DHTPeerAnnounceStorage storage;

  SharedHandle<DownloadContext> ctx1 = createDownloadContext(infoHash1);
  SharedHandle<DownloadContext> ctx2 = createDownloadContext(infoHash2);

  SharedHandle<MockPeerStorage> peerStorage1(new MockPeerStorage());
  SharedHandle<MockPeerStorage> peerStorage2(new MockPeerStorage());

  storage.addPeerAnnounce(infoHash1, "192.168.0.1", 6881);
  storage.addPeerAnnounce(infoHash1, peerStorage1);
  storage.addPeerAnnounce(infoHash2, peerStorage2);

  storage.removeLocalPeerAnnounce(bittorrent::getInfoHash(ctx2));
  CPPUNIT_ASSERT(!storage.contains(infoHash2));

  storage.removeLocalPeerAnnounce(bittorrent::getInfoHash(ctx1));
  CPPUNIT_ASSERT(storage.contains(infoHash1));
}

} // namespace aria2
