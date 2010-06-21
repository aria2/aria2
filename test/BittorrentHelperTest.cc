#include "bittorrent_helper.h"

#include <cstring>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "DownloadContext.h"
#include "util.h"
#include "RecoverableException.h"
#include "AnnounceTier.h"
#include "FixedNumberRandomizer.h"
#include "FileEntry.h"
#include "array_fun.h"
#include "messageDigest.h"
#include "a2netcompat.h"
#include "bencode2.h"
#include "TestUtil.h"
#include "base32.h"

namespace aria2 {

namespace bittorrent {

class BittorrentHelperTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BittorrentHelperTest);
  CPPUNIT_TEST(testGetInfoHash);
  CPPUNIT_TEST(testGetPieceHash);
  CPPUNIT_TEST(testGetFileEntries);
  CPPUNIT_TEST(testGetTotalLength);
  CPPUNIT_TEST(testGetFileEntriesSingle);
  CPPUNIT_TEST(testGetTotalLengthSingle);
  CPPUNIT_TEST(testGetFileModeMulti);
  CPPUNIT_TEST(testGetFileModeSingle);
  CPPUNIT_TEST(testGetNameMulti);
  CPPUNIT_TEST(testGetNameSingle);
  CPPUNIT_TEST(testOverrideName);
  CPPUNIT_TEST(testGetAnnounceTier);
  CPPUNIT_TEST(testGetAnnounceTierAnnounceList);
  CPPUNIT_TEST(testGetPieceLength);
  CPPUNIT_TEST(testGetInfoHashAsString);
  CPPUNIT_TEST(testGetPeerId);
  CPPUNIT_TEST(testComputeFastSet);
  CPPUNIT_TEST(testGetFileEntries_multiFileUrlList);
  CPPUNIT_TEST(testGetFileEntries_singleFileUrlList);
  CPPUNIT_TEST(testGetFileEntries_singleFileUrlListEndsWithSlash);
  CPPUNIT_TEST(testLoadFromMemory);
  CPPUNIT_TEST(testLoadFromMemory_somethingMissing);
  CPPUNIT_TEST(testLoadFromMemory_overrideName);
  CPPUNIT_TEST(testLoadFromMemory_multiFileDirTraversal);
  CPPUNIT_TEST(testLoadFromMemory_singleFileDirTraversal);
  CPPUNIT_TEST(testGetNodes);
  CPPUNIT_TEST(testGetBasePath);
  CPPUNIT_TEST(testSetFileFilter_single);
  CPPUNIT_TEST(testSetFileFilter_multi);
  CPPUNIT_TEST(testUTF8Torrent);
  CPPUNIT_TEST(testEtc);
  CPPUNIT_TEST(testCreatecompact);
  CPPUNIT_TEST(testCheckBitfield);
  CPPUNIT_TEST(testMetadata);
  CPPUNIT_TEST(testParseMagnet);
  CPPUNIT_TEST(testParseMagnet_base32);
  CPPUNIT_TEST(testMetadata2Torrent);
  CPPUNIT_TEST(testTorrent2Magnet);
  CPPUNIT_TEST(testExtractPeerFromList);
  CPPUNIT_TEST(testExtract2PeersFromList);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {
  }

  void testGetInfoHash();
  void testGetPieceHash();
  void testGetFileEntries();
  void testGetTotalLength();
  void testGetFileEntriesSingle();
  void testGetTotalLengthSingle();
  void testGetFileModeMulti();
  void testGetFileModeSingle();
  void testGetNameMulti();
  void testGetNameSingle();
  void testOverrideName();
  void testGetAnnounceTier();
  void testGetAnnounceTierAnnounceList();
  void testGetPieceLength();
  void testGetInfoHashAsString();
  void testGetPeerId();
  void testComputeFastSet();
  void testGetFileEntries_multiFileUrlList();
  void testGetFileEntries_singleFileUrlList();
  void testGetFileEntries_singleFileUrlListEndsWithSlash();
  void testLoadFromMemory();
  void testLoadFromMemory_somethingMissing();
  void testLoadFromMemory_overrideName();
  void testLoadFromMemory_multiFileDirTraversal();
  void testLoadFromMemory_singleFileDirTraversal();
  void testGetNodes();
  void testGetBasePath();
  void testSetFileFilter_single();
  void testSetFileFilter_multi();
  void testUTF8Torrent();
  void testEtc();
  void testCreatecompact();
  void testCheckBitfield();
  void testMetadata();
  void testParseMagnet();
  void testParseMagnet_base32();
  void testMetadata2Torrent();
  void testTorrent2Magnet();
  void testExtractPeerFromList();
  void testExtract2PeersFromList();
};


CPPUNIT_TEST_SUITE_REGISTRATION(BittorrentHelperTest);

void BittorrentHelperTest::testGetInfoHash() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);

  std::string correctHash = "248d0a1cd08284299de78d5c1ed359bb46717d8c";

  CPPUNIT_ASSERT_EQUAL(correctHash, bittorrent::getInfoHashString(dctx));
}

void BittorrentHelperTest::testGetPieceHash() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);

  CPPUNIT_ASSERT_EQUAL(util::toHex("AAAAAAAAAAAAAAAAAAAA", 20),
                       dctx->getPieceHash(0));
  CPPUNIT_ASSERT_EQUAL(util::toHex("BBBBBBBBBBBBBBBBBBBB", 20),
                       dctx->getPieceHash(1));
  CPPUNIT_ASSERT_EQUAL(util::toHex("CCCCCCCCCCCCCCCCCCCC", 20),
                       dctx->getPieceHash(2));
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       dctx->getPieceHash(3));

  CPPUNIT_ASSERT_EQUAL(MessageDigestContext::SHA1, dctx->getPieceHashAlgo());
}

void BittorrentHelperTest::testGetFileEntries() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);
  // This is multi-file torrent.
  std::vector<SharedHandle<FileEntry> > fileEntries = dctx->getFileEntries();
  // There are 2 file entries.
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntries.size());
  std::vector<SharedHandle<FileEntry> >::iterator itr = fileEntries.begin();

  SharedHandle<FileEntry> fileEntry1 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-test/aria2/src/aria2c"),
                       fileEntry1->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-test/aria2/src/aria2c"),
                       fileEntry1->getOriginalName());
  itr++;
  SharedHandle<FileEntry> fileEntry2 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-test/aria2-0.2.2.tar.bz2"),
                       fileEntry2->getPath());
}

void BittorrentHelperTest::testGetFileEntriesSingle() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("single.torrent", dctx);
  // This is multi-file torrent.
  std::vector<SharedHandle<FileEntry> > fileEntries = dctx->getFileEntries();
  // There is 1 file entry.
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries.size());
  std::vector<SharedHandle<FileEntry> >::iterator itr = fileEntries.begin();

  SharedHandle<FileEntry> fileEntry1 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-0.8.2.tar.bz2"),
                       fileEntry1->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.8.2.tar.bz2"),
                       fileEntry1->getOriginalName());
}

void BittorrentHelperTest::testGetTotalLength() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);

  CPPUNIT_ASSERT_EQUAL((uint64_t)384ULL, dctx->getTotalLength());
}

void BittorrentHelperTest::testGetTotalLengthSingle() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("single.torrent", dctx);

  CPPUNIT_ASSERT_EQUAL((uint64_t)384ULL, dctx->getTotalLength());
}

void BittorrentHelperTest::testGetFileModeMulti() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);

  CPPUNIT_ASSERT_EQUAL(MULTI, getTorrentAttrs(dctx)->mode);
}

void BittorrentHelperTest::testGetFileModeSingle() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("single.torrent", dctx);

  CPPUNIT_ASSERT_EQUAL(SINGLE, getTorrentAttrs(dctx)->mode);
}

void BittorrentHelperTest::testGetNameMulti() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-test"), getTorrentAttrs(dctx)->name);
}

void BittorrentHelperTest::testGetNameSingle() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("single.torrent", dctx);

  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-0.8.2.tar.bz2"),
                       dctx->getBasePath());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.8.2.tar.bz2"),
                       getTorrentAttrs(dctx)->name);
}

void BittorrentHelperTest::testOverrideName()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx, "aria2-override.name");
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-override.name"),
                       dctx->getBasePath());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-override.name"),
                       getTorrentAttrs(dctx)->name);
}


void BittorrentHelperTest::testGetAnnounceTier() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("single.torrent", dctx);
  SharedHandle<TorrentAttribute> attrs = getTorrentAttrs(dctx);
  // There is 1 tier.
  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->announceList.size());

  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->announceList[0].size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/announce.php"),
                       attrs->announceList[0][0]);
}

void BittorrentHelperTest::testGetAnnounceTierAnnounceList() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);
  SharedHandle<TorrentAttribute> attrs = getTorrentAttrs(dctx);
  // There are 3 tiers.
  CPPUNIT_ASSERT_EQUAL((size_t)3, attrs->announceList.size());

  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->announceList[0].size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker1"),
                       attrs->announceList[0][0]);

  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->announceList[1].size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker2"),
                       attrs->announceList[1][0]);

  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->announceList[2].size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker3"),
                       attrs->announceList[2][0]);
}

void BittorrentHelperTest::testGetPieceLength() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);

  CPPUNIT_ASSERT_EQUAL((size_t)128, dctx->getPieceLength());
}

void BittorrentHelperTest::testGetInfoHashAsString() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);

  CPPUNIT_ASSERT_EQUAL(std::string("248d0a1cd08284299de78d5c1ed359bb46717d8c"),
                       getInfoHashString(dctx));
}

void BittorrentHelperTest::testGetPeerId() {
  std::string peerId = generatePeerId("aria2-");
  CPPUNIT_ASSERT(peerId.find("aria2-") == 0);
  CPPUNIT_ASSERT_EQUAL((size_t)20, peerId.size());
}

void BittorrentHelperTest::testComputeFastSet()
{
  std::string ipaddr = "192.168.0.1";
  unsigned char infoHash[20];
  memset(infoHash, 0, sizeof(infoHash));
  infoHash[0] = 0xff;
  int fastSetSize = 10;
  size_t numPieces = 1000;
  {
    std::vector<size_t> fastSet;
    computeFastSet(fastSet, ipaddr, numPieces, infoHash, fastSetSize);
    size_t ans[] = { 686, 459, 278, 200, 404, 834, 64, 203, 760, 950 };
    std::vector<size_t> ansSet(vbegin(ans), vend(ans));
    CPPUNIT_ASSERT(std::equal(fastSet.begin(), fastSet.end(), ansSet.begin()));
  }
  ipaddr = "10.0.0.1";
  {
    std::vector<size_t> fastSet;
    computeFastSet(fastSet, ipaddr, numPieces, infoHash, fastSetSize);
    size_t ans[] = { 568, 188, 466, 452, 550, 662, 109, 226, 398, 11 };
    std::vector<size_t> ansSet(vbegin(ans), vend(ans));
    CPPUNIT_ASSERT(std::equal(fastSet.begin(), fastSet.end(), ansSet.begin()));
  }
  // See when pieces < fastSetSize
  numPieces = 9;
  {
    std::vector<size_t> fastSet;
    computeFastSet(fastSet, ipaddr, numPieces, infoHash, fastSetSize);
    size_t ans[] = { 8, 6, 7, 5, 1, 4, 0, 2, 3 };
    std::vector<size_t> ansSet(vbegin(ans), vend(ans));
    CPPUNIT_ASSERT(std::equal(fastSet.begin(), fastSet.end(), ansSet.begin()));
  }
}

void BittorrentHelperTest::testGetFileEntries_multiFileUrlList() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("url-list-multiFile.torrent", dctx);
  // This is multi-file torrent.
  const std::vector<SharedHandle<FileEntry> >& fileEntries =
    dctx->getFileEntries();
  // There are 2 file entries.
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntries.size());
  std::vector<SharedHandle<FileEntry> >::const_iterator itr =
    fileEntries.begin();

  const SharedHandle<FileEntry>& fileEntry1 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-test@/aria2@/src@/aria2c@"),
                       fileEntry1->getPath());
  const std::deque<std::string>& uris1 = fileEntry1->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)2, uris1.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/dist/aria2-test%40/aria2%40/src%40/aria2c%40"),
                       uris1[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/dist/aria2-test%40/aria2%40/src%40/aria2c%40"),
                       uris1[1]);

  ++itr;
  const SharedHandle<FileEntry>& fileEntry2 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-test@/aria2-0.2.2.tar.bz2"),
                       fileEntry2->getPath());
  const std::deque<std::string>& uris2 = fileEntry2->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)2, uris2.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/dist/aria2-test%40/aria2-0.2.2.tar.bz2"),
                       uris2[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/dist/aria2-test%40/aria2-0.2.2.tar.bz2"),
                       uris2[1]);
}

void BittorrentHelperTest::testGetFileEntries_singleFileUrlList() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("url-list-singleFile.torrent", dctx);
  // This is single-file torrent.
  const std::vector<SharedHandle<FileEntry> >& fileEntries =
    dctx->getFileEntries();
  // There are 1 file entries.
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries.size());

  const SharedHandle<FileEntry>& fileEntry1 = fileEntries.front();
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2.tar.bz2"),
                       fileEntry1->getPath());
  const std::deque<std::string>& uris1 = fileEntry1->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)1, uris1.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/dist/aria2.tar.bz2"),
                       uris1[0]);
}

void BittorrentHelperTest::testGetFileEntries_singleFileUrlListEndsWithSlash() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("url-list-singleFileEndsWithSlash.torrent", dctx);
  // This is single-file torrent.
  const std::vector<SharedHandle<FileEntry> >& fileEntries =
    dctx->getFileEntries();
  // There are 1 file entries.
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries.size());

  const SharedHandle<FileEntry>& fileEntry1 = fileEntries.front();
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2@.tar.bz2"),
                       fileEntry1->getPath());
  const std::deque<std::string>& uris1 = fileEntry1->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)1, uris1.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/dist/aria2%40.tar.bz2"),
                       uris1[0]);
}

void BittorrentHelperTest::testLoadFromMemory()
{
  std::string memory = "d8:announce36:http://aria.rednoah.com/announce.php13:announce-listll16:http://tracker1 el15:http://tracker2el15:http://tracker3ee7:comment17:REDNOAH.COM RULES13:creation datei1123456789e4:infod5:filesld6:lengthi284e4:pathl5:aria23:src6:aria2ceed6:lengthi100e4:pathl19:aria2-0.2.2.tar.bz2eee4:name10:aria2-test12:piece lengthi128e6:pieces60:AAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCee";

  SharedHandle<DownloadContext> dctx(new DownloadContext());
  loadFromMemory(memory, dctx, "default");

  std::string correctHash = "248d0a1cd08284299de78d5c1ed359bb46717d8c";

  CPPUNIT_ASSERT_EQUAL(correctHash, getInfoHashString(dctx));
}

void BittorrentHelperTest::testLoadFromMemory_somethingMissing()
{
  // pieces missing
  try {
    std::string memory = "d8:announce36:http://aria.rednoah.com/announce.php4:infod4:name13:aria2.tar.bz26:lengthi262144eee";
    SharedHandle<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, "default");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    // OK
  }
}

void BittorrentHelperTest::testLoadFromMemory_overrideName()
{
  std::string memory = "d8:announce36:http://aria.rednoah.com/announce.php13:announce-listll16:http://tracker1 el15:http://tracker2el15:http://tracker3ee7:comment17:REDNOAH.COM RULES13:creation datei1123456789e4:infod5:filesld6:lengthi284e4:pathl5:aria23:src6:aria2ceed6:lengthi100e4:pathl19:aria2-0.2.2.tar.bz2eee4:name10:aria2-test12:piece lengthi128e6:pieces60:AAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCee";

  SharedHandle<DownloadContext> dctx(new DownloadContext());
  loadFromMemory(memory, dctx, "default", "aria2-override.name");

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-override.name"),
                       getTorrentAttrs(dctx)->name);
}

void BittorrentHelperTest::testLoadFromMemory_multiFileDirTraversal()
{
  std::string memory =
    "d8:announce27:http://example.com/announce4:infod5:filesld6:lengthi262144e4:pathl7:../dir14:dir28:file.imgeee4:name14:../name1/name212:piece lengthi262144e6:pieces20:00000000000000000000ee";

  SharedHandle<DownloadContext> dctx(new DownloadContext());
  dctx->setDir("/tmp");
  try {
    loadFromMemory(memory, dctx, "default");
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(RecoverableException& e) {
    // success
  }
}

void BittorrentHelperTest::testLoadFromMemory_singleFileDirTraversal()
{
  std::string memory =
    "d8:announce27:http://example.com/announce4:infod4:name14:../name1/name26:lengthi262144e12:piece lengthi262144e6:pieces20:00000000000000000000ee";

  SharedHandle<DownloadContext> dctx(new DownloadContext());
  dctx->setDir("/tmp");
  try {
    loadFromMemory(memory, dctx, "default");
  } catch(RecoverableException& e) {
    // success
  }
}

void BittorrentHelperTest::testGetNodes()
{
  {
    std::string memory =
      "d5:nodesl"
      "l11:192.168.0.1i6881ee"
      "l11:192.168.0.2i6882ee"
      "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
      "12:piece lengthi262144e"
      "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
      "ee";
    SharedHandle<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, "default");

    SharedHandle<TorrentAttribute> attrs = getTorrentAttrs(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)2, attrs->nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), attrs->nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, attrs->nodes[0].second);
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), attrs->nodes[1].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, attrs->nodes[1].second);
  }
  {
    // empty hostname
    std::string memory =
      "d5:nodesl"
      "l1: i6881ee"
      "l11:192.168.0.2i6882ee"
      "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
      "12:piece lengthi262144e"
      "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
      "ee";
    SharedHandle<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, "default");

    SharedHandle<TorrentAttribute> attrs = getTorrentAttrs(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), attrs->nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, attrs->nodes[0].second);
  }
  {
    // bad port 
    std::string memory =
      "d5:nodesl"
      "l11:192.168.0.11:xe"
      "l11:192.168.0.2i6882ee"
      "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
      "12:piece lengthi262144e"
      "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
      "ee";
    SharedHandle<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, "default");

    SharedHandle<TorrentAttribute> attrs = getTorrentAttrs(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), attrs->nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, attrs->nodes[0].second);
  }
  {
    // port missing
    std::string memory =
      "d5:nodesl"
      "l11:192.168.0.1e"
      "l11:192.168.0.2i6882ee"
      "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
      "12:piece lengthi262144e"
      "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
      "ee";
    SharedHandle<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, "default");

    SharedHandle<TorrentAttribute> attrs = getTorrentAttrs(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), attrs->nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, attrs->nodes[0].second);
  }
  {
    // nodes is not a list
    std::string memory =
      "d5:nodes"
      "l11:192.168.0.1e"
      "4:infod4:name13:aria2.tar.bz26:lengthi262144e"
      "12:piece lengthi262144e"
      "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
      "ee";
    SharedHandle<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, "default");

    SharedHandle<TorrentAttribute> attrs = getTorrentAttrs(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)0, attrs->nodes.size());
  }
  {
    // the element of node is not Data
    std::string memory =
      "d5:nodesl"
      "ll11:192.168.0.1i6881eee"
      "l11:192.168.0.2i6882ee"
      "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
      "12:piece lengthi262144e"
      "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
      "ee";
    SharedHandle<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, "default");

    SharedHandle<TorrentAttribute> attrs = getTorrentAttrs(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), attrs->nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, attrs->nodes[0].second);
  }
}

void BittorrentHelperTest::testGetBasePath()
{
  SharedHandle<DownloadContext> singleCtx(new DownloadContext());
  load("single.torrent", singleCtx);
  singleCtx->setFilePathWithIndex(1, "new-path");
  CPPUNIT_ASSERT_EQUAL(std::string("new-path"), singleCtx->getBasePath());

  SharedHandle<DownloadContext> multiCtx(new DownloadContext());
  multiCtx->setDir("downloads");
  load("test.torrent", multiCtx);
  CPPUNIT_ASSERT_EQUAL(std::string("downloads/aria2-test"),
                       multiCtx->getBasePath());
}

void BittorrentHelperTest::testSetFileFilter_single()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("single.torrent", dctx);

  CPPUNIT_ASSERT(dctx->getFirstFileEntry()->isRequested());

  dctx->setFileFilter(util::parseIntRange(""));
  CPPUNIT_ASSERT(dctx->getFirstFileEntry()->isRequested());

  dctx->setFileFilter(util::parseIntRange("1"));
  CPPUNIT_ASSERT(dctx->getFirstFileEntry()->isRequested());

  // For single file torrent, file is always selected whatever range
  // is passed.
  dctx->setFileFilter(util::parseIntRange("2"));
  CPPUNIT_ASSERT(dctx->getFirstFileEntry()->isRequested());
}

void BittorrentHelperTest::testSetFileFilter_multi()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);

  CPPUNIT_ASSERT(dctx->getFileEntries()[0]->isRequested());
  CPPUNIT_ASSERT(dctx->getFileEntries()[1]->isRequested());

  dctx->setFileFilter(util::parseIntRange(""));
  CPPUNIT_ASSERT(dctx->getFileEntries()[0]->isRequested());
  CPPUNIT_ASSERT(dctx->getFileEntries()[1]->isRequested());

  dctx->setFileFilter(util::parseIntRange("2"));
  CPPUNIT_ASSERT(!dctx->getFileEntries()[0]->isRequested());
  CPPUNIT_ASSERT(dctx->getFileEntries()[1]->isRequested());

  dctx->setFileFilter(util::parseIntRange("3"));
  CPPUNIT_ASSERT(!dctx->getFileEntries()[0]->isRequested());
  CPPUNIT_ASSERT(!dctx->getFileEntries()[1]->isRequested());

  dctx->setFileFilter(util::parseIntRange("1,2"));
  CPPUNIT_ASSERT(dctx->getFileEntries()[0]->isRequested());
  CPPUNIT_ASSERT(dctx->getFileEntries()[1]->isRequested());
}

void BittorrentHelperTest::testUTF8Torrent()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("utf8.torrent", dctx);
  CPPUNIT_ASSERT_EQUAL(std::string("name in utf-8"),
                       getTorrentAttrs(dctx)->name);
  CPPUNIT_ASSERT_EQUAL(std::string("./name in utf-8/path in utf-8"),
                       dctx->getFirstFileEntry()->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("This is utf8 comment."),
                       getTorrentAttrs(dctx)->comment);
}

void BittorrentHelperTest::testEtc()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);
  CPPUNIT_ASSERT_EQUAL(std::string("REDNOAH.COM RULES"),
                       getTorrentAttrs(dctx)->comment);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"),
                       getTorrentAttrs(dctx)->createdBy);
  CPPUNIT_ASSERT_EQUAL((time_t)1123456789,
                       getTorrentAttrs(dctx)->creationDate);
}

void BittorrentHelperTest::testCreatecompact()
{
  unsigned char compact[6];
  // Note: bittorrent::createcompact() on linux can handle IPv4-mapped
  // addresses like `ffff::127.0.0.1', but on cygwin, it doesn't.
  CPPUNIT_ASSERT(createcompact(compact, "127.0.0.1", 6881));

  std::pair<std::string, uint16_t> p = unpackcompact(compact);
  CPPUNIT_ASSERT_EQUAL(std::string("127.0.0.1"), p.first);
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, p.second);
}

void BittorrentHelperTest::testCheckBitfield()
{
  unsigned char bitfield[] = { 0xff, 0xe0 };
  checkBitfield(bitfield, sizeof(bitfield), 11);
  try {
    checkBitfield(bitfield, sizeof(bitfield), 17);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // success
  }
  // Change last byte
  bitfield[1] = 0xf0;
  try {
    checkBitfield(bitfield, sizeof(bitfield), 11);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // success
  }
}

void BittorrentHelperTest::testMetadata() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);
  std::string torrentData = readFile("test.torrent");
  SharedHandle<ValueBase> tr = bencode2::decode(torrentData);
  SharedHandle<ValueBase> infoDic = asDict(tr)->get("info");
  std::string metadata = bencode2::encode(infoDic);
  SharedHandle<TorrentAttribute> attrs = getTorrentAttrs(dctx);
  CPPUNIT_ASSERT(metadata == attrs->metadata);
  CPPUNIT_ASSERT_EQUAL(metadata.size(), attrs->metadataSize);
}

void BittorrentHelperTest::testParseMagnet()
{
  std::string magnet =
    "magnet:?xt=urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c&dn=aria2"
    "&tr=http://tracker1&tr=http://tracker2";
  SharedHandle<TorrentAttribute> attrs = bittorrent::parseMagnet(magnet);
  CPPUNIT_ASSERT_EQUAL(std::string("248d0a1cd08284299de78d5c1ed359bb46717d8c"),
                       util::toHex(attrs->infoHash));
  CPPUNIT_ASSERT_EQUAL(std::string("[METADATA]aria2"), attrs->name);
  CPPUNIT_ASSERT_EQUAL((size_t)2, attrs->announceList.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker1"),
                       attrs->announceList[0][0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker2"),
                       attrs->announceList[1][0]);

  magnet = "magnet:?xt=urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c";
  attrs = bittorrent::parseMagnet(magnet);
  CPPUNIT_ASSERT_EQUAL
    (std::string("[METADATA]248d0a1cd08284299de78d5c1ed359bb46717d8c"),
     attrs->name);
  CPPUNIT_ASSERT(attrs->announceList.empty());

  magnet = "magnet:?xt=urn:sha1:7899bdb90a026c746f3cbc10839dd9b2a2a3e985&"
    "xt=urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c";
  attrs = bittorrent::parseMagnet(magnet);
  CPPUNIT_ASSERT_EQUAL(std::string("248d0a1cd08284299de78d5c1ed359bb46717d8c"),
                       util::toHex(attrs->infoHash));
}

void BittorrentHelperTest::testParseMagnet_base32()
{
  std::string infoHash = "248d0a1cd08284299de78d5c1ed359bb46717d8c";
  std::string base32InfoHash = base32::encode(util::fromHex(infoHash));
  std::string magnet = "magnet:?xt=urn:btih:"+base32InfoHash+"&dn=aria2";
  SharedHandle<TorrentAttribute> attrs = bittorrent::parseMagnet(magnet);
  CPPUNIT_ASSERT_EQUAL
    (std::string("248d0a1cd08284299de78d5c1ed359bb46717d8c"),
     util::toHex(attrs->infoHash));
}

void BittorrentHelperTest::testMetadata2Torrent()
{
  std::string metadata = "METADATA";
  SharedHandle<TorrentAttribute> attrs(new TorrentAttribute());
  CPPUNIT_ASSERT_EQUAL
    (std::string("d4:infoMETADATAe"), metadata2Torrent(metadata, attrs));
  attrs->announceList.push_back(std::vector<std::string>());
  attrs->announceList[0].push_back("http://localhost/announce");
  CPPUNIT_ASSERT_EQUAL
    (std::string("d"
                 "13:announce-list"
                 "ll25:http://localhost/announceee"
                 "4:infoMETADATA"
                 "e"),
     metadata2Torrent(metadata, attrs));
}

void BittorrentHelperTest::testTorrent2Magnet()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);
  
  CPPUNIT_ASSERT_EQUAL
    (std::string("magnet:?xt=urn:btih:248D0A1CD08284299DE78D5C1ED359BB46717D8C"
                 "&dn=aria2-test"
                 "&tr=http%3A%2F%2Ftracker1"
                 "&tr=http%3A%2F%2Ftracker2"
                 "&tr=http%3A%2F%2Ftracker3"),
     torrent2Magnet(getTorrentAttrs(dctx)));
}

void BittorrentHelperTest::testExtractPeerFromList()
{
  std::string peersString =
    "d5:peersld2:ip11:192.168.0.17:peer id20:aria2-00000000000000"
    "4:porti2006eeee";

  SharedHandle<ValueBase> dict = bencode2::decode(peersString);
  
  std::deque<SharedHandle<Peer> > peers;
  extractPeer(asDict(dict)->get("peers"), std::back_inserter(peers));
  CPPUNIT_ASSERT_EQUAL((size_t)1, peers.size());
  SharedHandle<Peer> peer = *peers.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)2006, peer->getPort());
}

void BittorrentHelperTest::testExtract2PeersFromList()
{
  std::string peersString =
    "d5:peersld2:ip11:192.168.0.17:peer id20:aria2-00000000000000"
    "4:porti65535eed2:ip11:192.168.0.27:peer id20:aria2-00000000000000"
    "4:porti2007eeee";

  SharedHandle<ValueBase> dict = bencode2::decode(peersString);

  std::deque<SharedHandle<Peer> > peers;
  extractPeer(asDict(dict)->get("peers"), std::back_inserter(peers));
  CPPUNIT_ASSERT_EQUAL((size_t)2, peers.size());
  SharedHandle<Peer> peer = *peers.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)65535, peer->getPort());

  peer = *(peers.begin()+1);
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), peer->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)2007, peer->getPort());
}

} // namespace bittorrent

} // namespace aria2
