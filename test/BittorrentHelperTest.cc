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
#include "bencode.h"
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
  CPPUNIT_TEST(testLoadFromMemory_joinPathMulti);
  CPPUNIT_TEST(testLoadFromMemory_joinPathSingle);
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
  void testLoadFromMemory_joinPathMulti();
  void testLoadFromMemory_joinPathSingle();
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
};


CPPUNIT_TEST_SUITE_REGISTRATION(BittorrentHelperTest);

static const BDE& getAnnounceList(const SharedHandle<DownloadContext>& dctx)
{
  return dctx->getAttribute(BITTORRENT)[ANNOUNCE_LIST];
}

static const std::string& getMode(const SharedHandle<DownloadContext>& dctx)
{
  return dctx->getAttribute(BITTORRENT)[MODE].s();
}

static const std::string& getName(const SharedHandle<DownloadContext>& dctx)
{
  return dctx->getAttribute(BITTORRENT)[NAME].s();
}

static const BDE& getNodes(const SharedHandle<DownloadContext>& dctx)
{
  return dctx->getAttribute(BITTORRENT)[NODES];
}

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

  CPPUNIT_ASSERT_EQUAL(MULTI, getMode(dctx));
}

void BittorrentHelperTest::testGetFileModeSingle() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("single.torrent", dctx);

  CPPUNIT_ASSERT_EQUAL(SINGLE, getMode(dctx));
}

void BittorrentHelperTest::testGetNameMulti() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-test"), getName(dctx));
}

void BittorrentHelperTest::testGetNameSingle() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("single.torrent", dctx);

  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-0.8.2.tar.bz2"),
		       dctx->getBasePath());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.8.2.tar.bz2"), getName(dctx));
}

void BittorrentHelperTest::testOverrideName()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx, "aria2-override.name");
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-override.name"),
		       dctx->getBasePath());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-override.name"), getName(dctx));
}


void BittorrentHelperTest::testGetAnnounceTier() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("single.torrent", dctx);

  const BDE& announceList = getAnnounceList(dctx);
  
  // There is 1 tier.
  CPPUNIT_ASSERT_EQUAL((size_t)1, announceList.size());

  const BDE& tier = announceList[0]; 
  CPPUNIT_ASSERT_EQUAL((size_t)1, tier.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/announce.php"),
		       util::trim(tier[0].s()));

}

void BittorrentHelperTest::testGetAnnounceTierAnnounceList() {
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);

  const BDE& announceList = getAnnounceList(dctx);
  
  // There are 3 tiers.
  CPPUNIT_ASSERT_EQUAL((size_t)3, announceList.size());

  const BDE& tier1 = announceList[0];
  CPPUNIT_ASSERT_EQUAL((size_t)1, tier1.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker1"),
		       util::trim(tier1[0].s()));

  const BDE& tier2 = announceList[1];
  CPPUNIT_ASSERT_EQUAL((size_t)1, tier2.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker2"), tier2[0].s());

  const BDE& tier3 = announceList[2];
  CPPUNIT_ASSERT_EQUAL((size_t)1, tier3.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker3"), tier3[0].s());
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
    std::vector<size_t> ansSet(&ans[0], &ans[arrayLength(ans)]);
    CPPUNIT_ASSERT(std::equal(fastSet.begin(), fastSet.end(), ansSet.begin()));
  }
  ipaddr = "10.0.0.1";
  {
    std::vector<size_t> fastSet;
    computeFastSet(fastSet, ipaddr, numPieces, infoHash, fastSetSize);
    size_t ans[] = { 568, 188, 466, 452, 550, 662, 109, 226, 398, 11 };
    std::vector<size_t> ansSet(&ans[0], &ans[arrayLength(ans)]);
    CPPUNIT_ASSERT(std::equal(fastSet.begin(), fastSet.end(), ansSet.begin()));
  }
  // See when pieces < fastSetSize
  numPieces = 9;
  {
    std::vector<size_t> fastSet;
    computeFastSet(fastSet, ipaddr, numPieces, infoHash, fastSetSize);
    size_t ans[] = { 8, 6, 7, 5, 1, 4, 0, 2, 3 };
    std::vector<size_t> ansSet(&ans[0], &ans[arrayLength(ans)]);
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
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-test/aria2/src/aria2c"),
		       fileEntry1->getPath());
  const std::deque<std::string>& uris1 = fileEntry1->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)2, uris1.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/dist/aria2-test/aria2/src/aria2c"),
		       uris1[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/dist/aria2-test/aria2/src/aria2c"),
		       uris1[1]);

  ++itr;
  const SharedHandle<FileEntry>& fileEntry2 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-test/aria2-0.2.2.tar.bz2"),
		       fileEntry2->getPath());
  const std::deque<std::string>& uris2 = fileEntry2->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)2, uris2.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/dist/aria2-test/aria2-0.2.2.tar.bz2"),
		       uris2[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/dist/aria2-test/aria2-0.2.2.tar.bz2"),
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
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2.tar.bz2"),
		       fileEntry1->getPath());
  const std::deque<std::string>& uris1 = fileEntry1->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)1, uris1.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/dist/aria2.tar.bz2"),
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

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-override.name"), getName(dctx));
}

void BittorrentHelperTest::testLoadFromMemory_joinPathMulti()
{
  std::string memory =
    "d8:announce27:http://example.com/announce4:infod5:filesld6:lengthi262144e4:pathl7:../dir14:dir28:file.imgeee4:name14:../name1/name212:piece lengthi262144e6:pieces20:00000000000000000000ee";

  SharedHandle<DownloadContext> dctx(new DownloadContext());
  dctx->setDir("/tmp");
  loadFromMemory(memory, dctx, "default");

  // remove ".." element
  CPPUNIT_ASSERT_EQUAL(std::string("../name1/name2"), getName(dctx));
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/name1/dir1/dir2/file.img"),
		       dctx->getFirstFileEntry()->getPath());
}

void BittorrentHelperTest::testLoadFromMemory_joinPathSingle()
{
  std::string memory =
    "d8:announce27:http://example.com/announce4:infod4:name14:../name1/name26:lengthi262144e12:piece lengthi262144e6:pieces20:00000000000000000000ee";

  SharedHandle<DownloadContext> dctx(new DownloadContext());
  dctx->setDir("/tmp");
  loadFromMemory(memory, dctx, "default");

  CPPUNIT_ASSERT_EQUAL(std::string("../name1/name2"), getName(dctx));
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/name1/name2"),
		       dctx->getFirstFileEntry()->getPath());
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

    const BDE& nodes = getNodes(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)2, nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), nodes[0][HOSTNAME].s());
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, (uint16_t)nodes[0][PORT].i());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), nodes[1][HOSTNAME].s());
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, (uint16_t)nodes[1][PORT].i());
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

    const BDE& nodes = getNodes(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), nodes[0][HOSTNAME].s());
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, (uint16_t)nodes[0][PORT].i());
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

    const BDE& nodes = getNodes(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), nodes[0][HOSTNAME].s());
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, (uint16_t)nodes[0][PORT].i());
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

    const BDE& nodes = getNodes(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), nodes[0][HOSTNAME].s());
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, (uint16_t)nodes[0][PORT].i());
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

    CPPUNIT_ASSERT_EQUAL((size_t)0, getNodes(dctx).size());
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

    const BDE& nodes = getNodes(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), nodes[0][HOSTNAME].s());
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, (uint16_t)nodes[0][PORT].i());
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
  CPPUNIT_ASSERT_EQUAL(std::string("name in utf-8"), getName(dctx));
  CPPUNIT_ASSERT_EQUAL(std::string("./name in utf-8/path in utf-8"),
		       dctx->getFirstFileEntry()->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("This is utf8 comment."),
		       dctx->getAttribute(BITTORRENT)[COMMENT].s());
}

void BittorrentHelperTest::testEtc()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  load("test.torrent", dctx);
  CPPUNIT_ASSERT_EQUAL(std::string("REDNOAH.COM RULES"),
		       dctx->getAttribute(BITTORRENT)[COMMENT].s());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"),
		       dctx->getAttribute(BITTORRENT)[CREATED_BY].s());
  CPPUNIT_ASSERT_EQUAL((int64_t)1123456789,
		       dctx->getAttribute(BITTORRENT)[CREATION_DATE].i());
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
  BDE tr = bencode::decode(torrentData);
  BDE infoDic = tr["info"];
  std::string metadata = bencode::encode(infoDic);
  const BDE& attrs = dctx->getAttribute(bittorrent::BITTORRENT);
  CPPUNIT_ASSERT(metadata == attrs[bittorrent::METADATA].s());
  CPPUNIT_ASSERT_EQUAL(metadata.size(),
		       (size_t)attrs[bittorrent::METADATA_SIZE].i());
}

void BittorrentHelperTest::testParseMagnet()
{
  std::string magnet =
    "magnet:?xt=urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c&dn=aria2"
    "&tr=http://tracker1&tr=http://tracker2";
  BDE attrs = bittorrent::parseMagnet(magnet);
  CPPUNIT_ASSERT_EQUAL(std::string("248d0a1cd08284299de78d5c1ed359bb46717d8c"),
		       util::toHex(attrs[bittorrent::INFO_HASH].s()));
  CPPUNIT_ASSERT_EQUAL(std::string("[METADATA]aria2"),
		       attrs[bittorrent::NAME].s());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker1"),
		       attrs[bittorrent::ANNOUNCE_LIST][0][0].s());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker2"),
		       attrs[bittorrent::ANNOUNCE_LIST][0][1].s());

  magnet = "magnet:?xt=urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c";
  attrs = bittorrent::parseMagnet(magnet);
  CPPUNIT_ASSERT_EQUAL
    (std::string("[METADATA]248d0a1cd08284299de78d5c1ed359bb46717d8c"),
     attrs[bittorrent::NAME].s());
  CPPUNIT_ASSERT(attrs[bittorrent::ANNOUNCE_LIST].size() == 0);
}

void BittorrentHelperTest::testParseMagnet_base32()
{
  std::string infoHash = "248d0a1cd08284299de78d5c1ed359bb46717d8c";
  std::string base32InfoHash = base32::encode(util::fromHex(infoHash));
  std::string magnet = "magnet:?xt=urn:btih:"+base32InfoHash+"&dn=aria2";
  BDE attrs = bittorrent::parseMagnet(magnet);
  CPPUNIT_ASSERT_EQUAL
    (std::string("248d0a1cd08284299de78d5c1ed359bb46717d8c"),
     util::toHex(attrs[bittorrent::INFO_HASH].s()));
}

} // namespace bittorrent

} // namespace aria2
