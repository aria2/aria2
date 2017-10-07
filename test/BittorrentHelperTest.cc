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
#include "a2netcompat.h"
#include "bencode2.h"
#include "TestUtil.h"
#include "base32.h"
#include "Option.h"
#include "prefs.h"

namespace aria2 {

namespace bittorrent {

class BittorrentHelperTest : public CppUnit::TestFixture {

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
  CPPUNIT_TEST(testGetPeerAgent);
  CPPUNIT_TEST(testComputeFastSet);
  CPPUNIT_TEST(testGetFileEntries_multiFileUrlList);
  CPPUNIT_TEST(testGetFileEntries_singleFileUrlList);
  CPPUNIT_TEST(testGetFileEntries_singleFileUrlListEndsWithSlash);
  CPPUNIT_TEST(testLoadFromMemory);
  CPPUNIT_TEST(testLoadFromMemory_somethingMissing);
  CPPUNIT_TEST(testLoadFromMemory_overrideName);
  CPPUNIT_TEST(testLoadFromMemory_multiFileDirTraversal);
  CPPUNIT_TEST(testLoadFromMemory_singleFileDirTraversal);
  CPPUNIT_TEST(testLoadFromMemory_multiFileNonUtf8Path);
  CPPUNIT_TEST(testLoadFromMemory_singleFileNonUtf8Path);
  CPPUNIT_TEST(testGetNodes);
  CPPUNIT_TEST(testGetBasePath);
  CPPUNIT_TEST(testSetFileFilter_single);
  CPPUNIT_TEST(testSetFileFilter_multi);
  CPPUNIT_TEST(testUTF8Torrent);
  CPPUNIT_TEST(testEtc);
  CPPUNIT_TEST(testCheckBitfield);
  CPPUNIT_TEST(testMetadata);
  CPPUNIT_TEST(testParseMagnet);
  CPPUNIT_TEST(testParseMagnet_base32);
  CPPUNIT_TEST(testMetadata2Torrent);
  CPPUNIT_TEST(testTorrent2Magnet);
  CPPUNIT_TEST(testExtractPeerFromString);
  CPPUNIT_TEST(testExtractPeerFromList);
  CPPUNIT_TEST(testExtract2PeersFromList);
  CPPUNIT_TEST(testPackcompact);
  CPPUNIT_TEST(testUnpackcompact);
  CPPUNIT_TEST(testRemoveAnnounceUri);
  CPPUNIT_TEST(testAddAnnounceUri);
  CPPUNIT_TEST(testAdjustAnnounceUri);
  CPPUNIT_TEST_SUITE_END();

public:
  std::shared_ptr<Option> option_;

  void setUp()
  {
    option_.reset(new Option());
    option_->put(PREF_DIR, ".");
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
  void testGetPeerAgent();
  void testComputeFastSet();
  void testGetFileEntries_multiFileUrlList();
  void testGetFileEntries_singleFileUrlList();
  void testGetFileEntries_singleFileUrlListEndsWithSlash();
  void testLoadFromMemory();
  void testLoadFromMemory_somethingMissing();
  void testLoadFromMemory_overrideName();
  void testLoadFromMemory_multiFileDirTraversal();
  void testLoadFromMemory_singleFileDirTraversal();
  void testLoadFromMemory_multiFileNonUtf8Path();
  void testLoadFromMemory_singleFileNonUtf8Path();
  void testGetNodes();
  void testGetBasePath();
  void testSetFileFilter_single();
  void testSetFileFilter_multi();
  void testUTF8Torrent();
  void testEtc();
  void testCheckBitfield();
  void testMetadata();
  void testParseMagnet();
  void testParseMagnet_base32();
  void testMetadata2Torrent();
  void testTorrent2Magnet();
  void testExtractPeerFromString();
  void testExtractPeerFromList();
  void testExtract2PeersFromList();
  void testPackcompact();
  void testUnpackcompact();
  void testRemoveAnnounceUri();
  void testAddAnnounceUri();
  void testAdjustAnnounceUri();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BittorrentHelperTest);

void BittorrentHelperTest::testGetInfoHash()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", dctx, option_);

  std::string correctHash = "248d0a1cd08284299de78d5c1ed359bb46717d8c";

  CPPUNIT_ASSERT_EQUAL(correctHash, bittorrent::getInfoHashString(dctx));
}

void BittorrentHelperTest::testGetPieceHash()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", dctx, option_);

  CPPUNIT_ASSERT_EQUAL(std::string("AAAAAAAAAAAAAAAAAAAA"),
                       dctx->getPieceHash(0));
  CPPUNIT_ASSERT_EQUAL(std::string("BBBBBBBBBBBBBBBBBBBB"),
                       dctx->getPieceHash(1));
  CPPUNIT_ASSERT_EQUAL(std::string("CCCCCCCCCCCCCCCCCCCC"),
                       dctx->getPieceHash(2));
  CPPUNIT_ASSERT_EQUAL(std::string(""), dctx->getPieceHash(3));

  CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), dctx->getPieceHashType());
}

void BittorrentHelperTest::testGetFileEntries()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  option_->put(PREF_MAX_CONNECTION_PER_SERVER, "10");
  load(A2_TEST_DIR "/test.torrent", dctx, option_);
  // This is multi-file torrent.
  std::vector<std::shared_ptr<FileEntry>> fileEntries = dctx->getFileEntries();
  // There are 2 file entries.
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntries.size());
  std::vector<std::shared_ptr<FileEntry>>::iterator itr = fileEntries.begin();

  std::shared_ptr<FileEntry> fileEntry1 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-test/aria2/src/aria2c"),
                       fileEntry1->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-test/aria2/src/aria2c"),
                       fileEntry1->getOriginalName());
  CPPUNIT_ASSERT_EQUAL(10, fileEntry1->getMaxConnectionPerServer());
  itr++;
  std::shared_ptr<FileEntry> fileEntry2 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-test/aria2-0.2.2.tar.bz2"),
                       fileEntry2->getPath());
  CPPUNIT_ASSERT_EQUAL(10, fileEntry2->getMaxConnectionPerServer());
}

void BittorrentHelperTest::testGetFileEntriesSingle()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  option_->put(PREF_MAX_CONNECTION_PER_SERVER, "10");
  load(A2_TEST_DIR "/single.torrent", dctx, option_);
  // This is multi-file torrent.
  std::vector<std::shared_ptr<FileEntry>> fileEntries = dctx->getFileEntries();
  // There is 1 file entry.
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries.size());
  std::vector<std::shared_ptr<FileEntry>>::iterator itr = fileEntries.begin();

  std::shared_ptr<FileEntry> fileEntry1 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-0.8.2.tar.bz2"),
                       fileEntry1->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.8.2.tar.bz2"),
                       fileEntry1->getOriginalName());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.8.2.tar.bz2"),
                       fileEntry1->getSuffixPath());
  CPPUNIT_ASSERT_EQUAL(10, fileEntry1->getMaxConnectionPerServer());
}

void BittorrentHelperTest::testGetTotalLength()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", dctx, option_);

  CPPUNIT_ASSERT_EQUAL((int64_t)384LL, dctx->getTotalLength());
}

void BittorrentHelperTest::testGetTotalLengthSingle()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/single.torrent", dctx, option_);

  CPPUNIT_ASSERT_EQUAL((int64_t)384LL, dctx->getTotalLength());
}

void BittorrentHelperTest::testGetFileModeMulti()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", dctx, option_);

  CPPUNIT_ASSERT_EQUAL(BT_FILE_MODE_MULTI, getTorrentAttrs(dctx)->mode);
}

void BittorrentHelperTest::testGetFileModeSingle()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/single.torrent", dctx, option_);

  CPPUNIT_ASSERT_EQUAL(BT_FILE_MODE_SINGLE, getTorrentAttrs(dctx)->mode);
}

void BittorrentHelperTest::testGetNameMulti()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", dctx, option_);

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-test"), getTorrentAttrs(dctx)->name);
}

void BittorrentHelperTest::testGetNameSingle()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/single.torrent", dctx, option_);

  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-0.8.2.tar.bz2"),
                       dctx->getBasePath());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.8.2.tar.bz2"),
                       getTorrentAttrs(dctx)->name);
}

void BittorrentHelperTest::testOverrideName()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", dctx, option_, "aria2-override.name");
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-override.name"),
                       dctx->getBasePath());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-override.name"),
                       getTorrentAttrs(dctx)->name);
}

void BittorrentHelperTest::testGetAnnounceTier()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/single.torrent", dctx, option_);
  auto attrs = getTorrentAttrs(dctx);
  // There is 1 tier.
  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->announceList.size());

  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->announceList[0].size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/announce.php"),
                       attrs->announceList[0][0]);
}

void BittorrentHelperTest::testGetAnnounceTierAnnounceList()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", dctx, option_);
  auto attrs = getTorrentAttrs(dctx);
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

void BittorrentHelperTest::testGetPieceLength()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", dctx, option_);

  CPPUNIT_ASSERT_EQUAL(128, dctx->getPieceLength());
}

void BittorrentHelperTest::testGetInfoHashAsString()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", dctx, option_);

  CPPUNIT_ASSERT_EQUAL(std::string("248d0a1cd08284299de78d5c1ed359bb46717d8c"),
                       getInfoHashString(dctx));
}

void BittorrentHelperTest::testGetPeerId()
{
  std::string peerId = generatePeerId("aria2-");
  CPPUNIT_ASSERT(peerId.find("aria2-") == 0);
  CPPUNIT_ASSERT_EQUAL((size_t)20, peerId.size());
}

void BittorrentHelperTest::testGetPeerAgent()
{
  std::string peerAgent = generateStaticPeerAgent("aria2/-1.-1.-1");
  CPPUNIT_ASSERT_EQUAL(std::string("aria2/-1.-1.-1"), peerAgent);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2/-1.-1.-1"),
                       bittorrent::getStaticPeerAgent());
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
    auto fastSet = computeFastSet(ipaddr, numPieces, infoHash, fastSetSize);
    size_t ans[] = {686, 459, 278, 200, 404, 834, 64, 203, 760, 950};
    CPPUNIT_ASSERT(std::equal(fastSet.begin(), fastSet.end(), std::begin(ans)));
  }
  ipaddr = "10.0.0.1";
  {
    auto fastSet = computeFastSet(ipaddr, numPieces, infoHash, fastSetSize);
    size_t ans[] = {568, 188, 466, 452, 550, 662, 109, 226, 398, 11};
    CPPUNIT_ASSERT(std::equal(fastSet.begin(), fastSet.end(), std::begin(ans)));
  }
  // See when pieces < fastSetSize
  numPieces = 9;
  {
    auto fastSet = computeFastSet(ipaddr, numPieces, infoHash, fastSetSize);
    size_t ans[] = {8, 6, 7, 5, 1, 4, 0, 2, 3};
    CPPUNIT_ASSERT(std::equal(fastSet.begin(), fastSet.end(), std::begin(ans)));
  }
}

void BittorrentHelperTest::testGetFileEntries_multiFileUrlList()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/url-list-multiFile.torrent", dctx, option_);
  // This is multi-file torrent.
  const std::vector<std::shared_ptr<FileEntry>>& fileEntries =
      dctx->getFileEntries();
  // There are 2 file entries.
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntries.size());
  std::vector<std::shared_ptr<FileEntry>>::const_iterator itr =
      fileEntries.begin();

  const std::shared_ptr<FileEntry>& fileEntry1 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-test@/aria2@/src@/aria2c@"),
                       fileEntry1->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-test@/aria2@/src@/aria2c@"),
                       fileEntry1->getSuffixPath());
  const std::deque<std::string>& uris1 = fileEntry1->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)2, uris1.size());
  CPPUNIT_ASSERT_EQUAL(
      std::string(
          "http://localhost/dist/aria2-test%40/aria2%40/src%40/aria2c%40"),
      uris1[0]);
  CPPUNIT_ASSERT_EQUAL(
      std::string("http://mirror/dist/aria2-test%40/aria2%40/src%40/aria2c%40"),
      uris1[1]);

  ++itr;
  const std::shared_ptr<FileEntry>& fileEntry2 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2-test@/aria2-0.2.2.tar.bz2"),
                       fileEntry2->getPath());
  const std::deque<std::string>& uris2 = fileEntry2->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)2, uris2.size());
  CPPUNIT_ASSERT_EQUAL(
      std::string("http://localhost/dist/aria2-test%40/aria2-0.2.2.tar.bz2"),
      uris2[0]);
  CPPUNIT_ASSERT_EQUAL(
      std::string("http://mirror/dist/aria2-test%40/aria2-0.2.2.tar.bz2"),
      uris2[1]);
}

void BittorrentHelperTest::testGetFileEntries_singleFileUrlList()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/url-list-singleFile.torrent", dctx, option_);
  // This is single-file torrent.
  const std::vector<std::shared_ptr<FileEntry>>& fileEntries =
      dctx->getFileEntries();
  // There are 1 file entries.
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries.size());

  const std::shared_ptr<FileEntry>& fileEntry1 = fileEntries.front();
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2.tar.bz2"), fileEntry1->getPath());
  const std::deque<std::string>& uris1 = fileEntry1->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)1, uris1.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/dist/aria2.tar.bz2"),
                       uris1[0]);
}

void BittorrentHelperTest::testGetFileEntries_singleFileUrlListEndsWithSlash()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/url-list-singleFileEndsWithSlash.torrent", dctx, option_);
  // This is single-file torrent.
  const std::vector<std::shared_ptr<FileEntry>>& fileEntries =
      dctx->getFileEntries();
  // There are 1 file entries.
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries.size());

  const std::shared_ptr<FileEntry>& fileEntry1 = fileEntries.front();
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2@.tar.bz2"), fileEntry1->getPath());
  const std::deque<std::string>& uris1 = fileEntry1->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)1, uris1.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/dist/aria2%40.tar.bz2"),
                       uris1[0]);
}

void BittorrentHelperTest::testLoadFromMemory_multiFileNonUtf8Path()
{
  auto path = List::g();
  path->append("path");
  path->append(fromHex("90a28a") + "E");
  auto file = Dict::g();
  file->put("length", Integer::g(1_k));
  file->put("path", std::move(path));
  auto files = List::g();
  files->append(std::move(file));
  auto info = Dict::g();
  info->put("files", std::move(files));
  info->put("piece length", Integer::g(1_k));
  info->put("pieces", "01234567890123456789");
  info->put("name", fromHex("1b") + "$B%O%m!<" + fromHex("1b") + "(B");
  Dict dict;
  dict.put("info", std::move(info));
  auto dctx = std::make_shared<DownloadContext>();
  loadFromMemory(bencode2::encode(&dict), dctx, option_, "default");

  auto& fe = dctx->getFirstFileEntry();
  CPPUNIT_ASSERT_EQUAL(
      std::string("./%1B%24B%25O%25m%21%3C%1B%28B/path/%90%A2%8AE"),
      fe->getPath());
  CPPUNIT_ASSERT_EQUAL(
      std::string("%1B%24B%25O%25m%21%3C%1B%28B/path/%90%A2%8AE"),
      fe->getSuffixPath());
  CPPUNIT_ASSERT_EQUAL(std::string("./%1B%24B%25O%25m%21%3C%1B%28B"),
                       dctx->getBasePath());
}

void BittorrentHelperTest::testLoadFromMemory_singleFileNonUtf8Path()
{
  auto info = Dict::g();
  info->put("piece length", Integer::g(1_k));
  info->put("pieces", "01234567890123456789");
  info->put("name", fromHex("90a28a") + "E");
  info->put("length", Integer::g(1_k));
  Dict dict;
  dict.put("info", std::move(info));
  auto dctx = std::make_shared<DownloadContext>();
  loadFromMemory(bencode2::encode(&dict), dctx, option_, "default");

  const std::shared_ptr<FileEntry>& fe = dctx->getFirstFileEntry();
  CPPUNIT_ASSERT_EQUAL(std::string("./%90%A2%8AE"), fe->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("%90%A2%8AE"), fe->getSuffixPath());
}

void BittorrentHelperTest::testLoadFromMemory()
{
  std::string memory = "d8:announce36:http://aria.rednoah.com/"
                       "announce.php13:announce-listll16:http://tracker1 "
                       "el15:http://tracker2el15:http://"
                       "tracker3ee7:comment17:REDNOAH.COM RULES13:creation "
                       "datei1123456789e4:infod5:filesld6:lengthi284e4:pathl5:"
                       "aria23:src6:aria2ceed6:lengthi100e4:pathl19:aria2-0.2."
                       "2.tar.bz2eee4:name10:aria2-test12:piece "
                       "lengthi128e6:pieces60:"
                       "AAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCC"
                       "CCCCCee";

  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  loadFromMemory(memory, dctx, option_, "default");

  std::string correctHash = "248d0a1cd08284299de78d5c1ed359bb46717d8c";

  CPPUNIT_ASSERT_EQUAL(correctHash, getInfoHashString(dctx));
}

void BittorrentHelperTest::testLoadFromMemory_somethingMissing()
{
  // pieces missing
  try {
    std::string memory = "d8:announce36:http://aria.rednoah.com/"
                         "announce.php4:infod4:name13:aria2.tar.bz26:"
                         "lengthi262144eee";
    std::shared_ptr<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, option_, "default");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    // OK
  }
}

void BittorrentHelperTest::testLoadFromMemory_overrideName()
{
  std::string memory = "d8:announce36:http://aria.rednoah.com/"
                       "announce.php13:announce-listll16:http://tracker1 "
                       "el15:http://tracker2el15:http://"
                       "tracker3ee7:comment17:REDNOAH.COM RULES13:creation "
                       "datei1123456789e4:infod5:filesld6:lengthi284e4:pathl5:"
                       "aria23:src6:aria2ceed6:lengthi100e4:pathl19:aria2-0.2."
                       "2.tar.bz2eee4:name10:aria2-test12:piece "
                       "lengthi128e6:pieces60:"
                       "AAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCC"
                       "CCCCCee";

  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  loadFromMemory(memory, dctx, option_, "default", "aria2-override.name");

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-override.name"),
                       getTorrentAttrs(dctx)->name);
}

void BittorrentHelperTest::testLoadFromMemory_multiFileDirTraversal()
{
  std::string memory = "d8:announce27:http://example.com/"
                       "announce4:infod5:filesld6:lengthi262144e4:pathl7:../"
                       "dir14:dir28:file.imgeee4:name14:../name1/name212:piece "
                       "lengthi262144e6:pieces20:00000000000000000000ee";

  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  try {
    loadFromMemory(memory, dctx, option_, "default");
    CPPUNIT_FAIL("Exception must be thrown.");
  }
  catch (RecoverableException& e) {
    // success
  }
}

void BittorrentHelperTest::testLoadFromMemory_singleFileDirTraversal()
{
  std::string memory = "d8:announce27:http://example.com/"
                       "announce4:infod4:name14:../name1/"
                       "name26:lengthi262144e12:piece "
                       "lengthi262144e6:pieces20:00000000000000000000ee";

  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  try {
    loadFromMemory(memory, dctx, option_, "default");
  }
  catch (RecoverableException& e) {
    // success
  }
}

void BittorrentHelperTest::testGetNodes()
{
  {
    std::string memory = "d5:nodesl"
                         "l11:192.168.0.1i6881ee"
                         "l11:192.168.0.2i6882ee"
                         "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
                         "12:piece lengthi262144e"
                         "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
                         "ee";
    std::shared_ptr<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, option_, "default");

    auto attrs = getTorrentAttrs(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)2, attrs->nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), attrs->nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, attrs->nodes[0].second);
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), attrs->nodes[1].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, attrs->nodes[1].second);
  }
  {
    // empty hostname
    std::string memory = "d5:nodesl"
                         "l1: i6881ee"
                         "l11:192.168.0.2i6882ee"
                         "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
                         "12:piece lengthi262144e"
                         "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
                         "ee";
    std::shared_ptr<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, option_, "default");

    auto attrs = getTorrentAttrs(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), attrs->nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, attrs->nodes[0].second);
  }
  {
    // bad port
    std::string memory = "d5:nodesl"
                         "l11:192.168.0.11:xe"
                         "l11:192.168.0.2i6882ee"
                         "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
                         "12:piece lengthi262144e"
                         "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
                         "ee";
    std::shared_ptr<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, option_, "default");

    auto attrs = getTorrentAttrs(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), attrs->nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, attrs->nodes[0].second);
  }
  {
    // port missing
    std::string memory = "d5:nodesl"
                         "l11:192.168.0.1e"
                         "l11:192.168.0.2i6882ee"
                         "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
                         "12:piece lengthi262144e"
                         "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
                         "ee";
    std::shared_ptr<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, option_, "default");

    auto attrs = getTorrentAttrs(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), attrs->nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, attrs->nodes[0].second);
  }
  {
    // nodes is not a list
    std::string memory = "d5:nodes"
                         "l11:192.168.0.1e"
                         "4:infod4:name13:aria2.tar.bz26:lengthi262144e"
                         "12:piece lengthi262144e"
                         "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
                         "ee";
    std::shared_ptr<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, option_, "default");

    auto attrs = getTorrentAttrs(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)0, attrs->nodes.size());
  }
  {
    // the element of node is not Data
    std::string memory = "d5:nodesl"
                         "ll11:192.168.0.1i6881eee"
                         "l11:192.168.0.2i6882ee"
                         "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
                         "12:piece lengthi262144e"
                         "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
                         "ee";
    std::shared_ptr<DownloadContext> dctx(new DownloadContext());
    loadFromMemory(memory, dctx, option_, "default");

    auto attrs = getTorrentAttrs(dctx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, attrs->nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), attrs->nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, attrs->nodes[0].second);
  }
}

void BittorrentHelperTest::testGetBasePath()
{
  std::shared_ptr<DownloadContext> singleCtx(new DownloadContext());
  load(A2_TEST_DIR "/single.torrent", singleCtx, option_);
  singleCtx->setFilePathWithIndex(1, "new-path");
  CPPUNIT_ASSERT_EQUAL(std::string("new-path"), singleCtx->getBasePath());

  option_->put(PREF_DIR, "downloads");
  std::shared_ptr<DownloadContext> multiCtx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", multiCtx, option_);
  CPPUNIT_ASSERT_EQUAL(std::string("downloads/aria2-test"),
                       multiCtx->getBasePath());
}

void BittorrentHelperTest::testSetFileFilter_single()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/single.torrent", dctx, option_);

  CPPUNIT_ASSERT(dctx->getFirstFileEntry()->isRequested());

  dctx->setFileFilter(SegList<int>());
  CPPUNIT_ASSERT(dctx->getFirstFileEntry()->isRequested());

  dctx->setFileFilter(util::parseIntSegments("1,2"));
  CPPUNIT_ASSERT(dctx->getFirstFileEntry()->isRequested());

  // For single file torrent, file is always selected whatever range
  // is passed.
  dctx->setFileFilter(util::parseIntSegments("2,3"));
  CPPUNIT_ASSERT(dctx->getFirstFileEntry()->isRequested());
}

void BittorrentHelperTest::testSetFileFilter_multi()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", dctx, option_);

  CPPUNIT_ASSERT(dctx->getFileEntries()[0]->isRequested());
  CPPUNIT_ASSERT(dctx->getFileEntries()[1]->isRequested());

  dctx->setFileFilter(SegList<int>());
  CPPUNIT_ASSERT(dctx->getFileEntries()[0]->isRequested());
  CPPUNIT_ASSERT(dctx->getFileEntries()[1]->isRequested());

  dctx->setFileFilter(util::parseIntSegments("2,3"));
  CPPUNIT_ASSERT(!dctx->getFileEntries()[0]->isRequested());
  CPPUNIT_ASSERT(dctx->getFileEntries()[1]->isRequested());

  dctx->setFileFilter(util::parseIntSegments("3,4"));
  CPPUNIT_ASSERT(!dctx->getFileEntries()[0]->isRequested());
  CPPUNIT_ASSERT(!dctx->getFileEntries()[1]->isRequested());

  dctx->setFileFilter(util::parseIntSegments("1,2"));
  CPPUNIT_ASSERT(dctx->getFileEntries()[0]->isRequested());
  CPPUNIT_ASSERT(dctx->getFileEntries()[1]->isRequested());
}

void BittorrentHelperTest::testUTF8Torrent()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/utf8.torrent", dctx, option_);
  CPPUNIT_ASSERT_EQUAL(std::string("name in utf-8"),
                       getTorrentAttrs(dctx)->name);
  CPPUNIT_ASSERT_EQUAL(std::string("./name in utf-8/path in utf-8"),
                       dctx->getFirstFileEntry()->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("This is utf8 comment."),
                       getTorrentAttrs(dctx)->comment);
}

void BittorrentHelperTest::testEtc()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", dctx, option_);
  CPPUNIT_ASSERT_EQUAL(std::string("REDNOAH.COM RULES"),
                       getTorrentAttrs(dctx)->comment);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), getTorrentAttrs(dctx)->createdBy);
  CPPUNIT_ASSERT_EQUAL((time_t)1123456789, getTorrentAttrs(dctx)->creationDate);
}

void BittorrentHelperTest::testCheckBitfield()
{
  unsigned char bitfield[] = {0xff, 0xe0};
  checkBitfield(bitfield, sizeof(bitfield), 11);
  try {
    checkBitfield(bitfield, sizeof(bitfield), 17);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (RecoverableException& e) {
    // success
  }
  // Change last byte
  bitfield[1] = 0xf0;
  try {
    checkBitfield(bitfield, sizeof(bitfield), 11);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (RecoverableException& e) {
    // success
  }
}

void BittorrentHelperTest::testMetadata()
{
  auto dctx = std::make_shared<DownloadContext>();
  load(A2_TEST_DIR "/test.torrent", dctx, option_);
  std::string torrentData = readFile(A2_TEST_DIR "/test.torrent");
  auto tr = bencode2::decode(torrentData);
  auto infoDic = downcast<Dict>(tr)->get("info");
  std::string metadata = bencode2::encode(infoDic);
  auto attrs = getTorrentAttrs(dctx);
  CPPUNIT_ASSERT(metadata == attrs->metadata);
  CPPUNIT_ASSERT_EQUAL(metadata.size(), attrs->metadataSize);
}

void BittorrentHelperTest::testParseMagnet()
{
  std::string magnet =
      "magnet:?xt=urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c&dn=aria2"
      "&tr=http://tracker1&tr=http://tracker2";
  auto attrs = bittorrent::parseMagnet(magnet);
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
  CPPUNIT_ASSERT_EQUAL(
      std::string("[METADATA]248d0a1cd08284299de78d5c1ed359bb46717d8c"),
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
  std::string base32InfoHash = base32::encode(fromHex(infoHash));
  std::string magnet = "magnet:?xt=urn:btih:" + base32InfoHash + "&dn=aria2";
  auto attrs = bittorrent::parseMagnet(magnet);
  CPPUNIT_ASSERT_EQUAL(std::string("248d0a1cd08284299de78d5c1ed359bb46717d8c"),
                       util::toHex(attrs->infoHash));
}

void BittorrentHelperTest::testMetadata2Torrent()
{
  TorrentAttribute attrs;
  std::string metadata = "METADATA";
  CPPUNIT_ASSERT_EQUAL(std::string("d4:infoMETADATAe"),
                       metadata2Torrent(metadata, &attrs));
  attrs.announceList.push_back(std::vector<std::string>());
  attrs.announceList[0].push_back("http://localhost/announce");
  CPPUNIT_ASSERT_EQUAL(std::string("d"
                                   "13:announce-list"
                                   "ll25:http://localhost/announceee"
                                   "4:infoMETADATA"
                                   "e"),
                       metadata2Torrent(metadata, &attrs));
}

void BittorrentHelperTest::testTorrent2Magnet()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext());
  load(A2_TEST_DIR "/test.torrent", dctx, option_);

  CPPUNIT_ASSERT_EQUAL(
      std::string("magnet:?xt=urn:btih:248D0A1CD08284299DE78D5C1ED359BB46717D8C"
                  "&dn=aria2-test"
                  "&tr=http%3A%2F%2Ftracker1"
                  "&tr=http%3A%2F%2Ftracker2"
                  "&tr=http%3A%2F%2Ftracker3"),
      torrent2Magnet(getTorrentAttrs(dctx)));
}

void BittorrentHelperTest::testExtractPeerFromString()
{
  std::string hextext = "100210354527354678541237324732171ae1";
  hextext += "20010db8bd0501d2288a1fc0000110ee1ae2";
  std::string peersstr = "36:" + fromHex(hextext);
  auto str = bencode2::decode(peersstr);
  std::deque<std::shared_ptr<Peer>> peers;
  extractPeer(str.get(), AF_INET6, std::back_inserter(peers));
  CPPUNIT_ASSERT_EQUAL((size_t)2, peers.size());
  CPPUNIT_ASSERT_EQUAL(std::string("1002:1035:4527:3546:7854:1237:3247:3217"),
                       peers[0]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, peers[0]->getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("2001:db8:bd05:1d2:288a:1fc0:1:10ee"),
                       peers[1]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6882, peers[1]->getPort());

  hextext = "c0a800011ae1";
  hextext += "c0a800021ae2";
  peersstr = "12:" + fromHex(hextext);
  str = bencode2::decode(peersstr);
  peers.clear();
  extractPeer(str.get(), AF_INET, std::back_inserter(peers));
  CPPUNIT_ASSERT_EQUAL((size_t)2, peers.size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peers[0]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, peers[0]->getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), peers[1]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6882, peers[1]->getPort());
}

void BittorrentHelperTest::testExtractPeerFromList()
{
  std::string peersString =
      "d5:peersld2:ip11:192.168.0.17:peer id20:aria2-00000000000000"
      "4:porti2006eeee";

  auto dict = bencode2::decode(peersString);

  std::deque<std::shared_ptr<Peer>> peers;
  extractPeer(downcast<Dict>(dict)->get("peers"), AF_INET,
              std::back_inserter(peers));
  CPPUNIT_ASSERT_EQUAL((size_t)1, peers.size());
  auto& peer = *peers.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)2006, peer->getPort());
}

void BittorrentHelperTest::testExtract2PeersFromList()
{
  std::string peersString =
      "d5:peersld2:ip11:192.168.0.17:peer id20:aria2-00000000000000"
      "4:porti65535eed2:ip11:192.168.0.27:peer id20:aria2-00000000000000"
      "4:porti2007eeee";

  auto dict = bencode2::decode(peersString);

  std::deque<std::shared_ptr<Peer>> peers;
  extractPeer(downcast<Dict>(dict)->get("peers"), AF_INET,
              std::back_inserter(peers));
  CPPUNIT_ASSERT_EQUAL((size_t)2, peers.size());
  auto& peer = *peers.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)65535, peer->getPort());

  peer = *(peers.begin() + 1);
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), peer->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)2007, peer->getPort());
}

void BittorrentHelperTest::testPackcompact()
{
  unsigned char compact[COMPACT_LEN_IPV6];
  CPPUNIT_ASSERT_EQUAL(
      (size_t)18,
      packcompact(compact, "1002:1035:4527:3546:7854:1237:3247:3217", 6881));
  CPPUNIT_ASSERT_EQUAL(std::string("100210354527354678541237324732171ae1"),
                       util::toHex(compact, 18));

  CPPUNIT_ASSERT_EQUAL((size_t)6, packcompact(compact, "192.168.0.1", 6881));
  CPPUNIT_ASSERT_EQUAL(std::string("c0a800011ae1"), util::toHex(compact, 6));

  CPPUNIT_ASSERT_EQUAL((size_t)0, packcompact(compact, "badaddr", 6881));
}

void BittorrentHelperTest::testUnpackcompact()
{
  unsigned char compact6[] = {0x10, 0x02, 0x10, 0x35, 0x45, 0x27,
                              0x35, 0x46, 0x78, 0x54, 0x12, 0x37,
                              0x32, 0x47, 0x32, 0x17, 0x1A, 0xE1};
  std::pair<std::string, uint16_t> p = unpackcompact(compact6, AF_INET6);
  CPPUNIT_ASSERT_EQUAL(std::string("1002:1035:4527:3546:7854:1237:3247:3217"),
                       p.first);
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, p.second);

  unsigned char compact[] = {0xC0, 0xa8, 0x00, 0x01, 0x1A, 0xE1};
  p = unpackcompact(compact, AF_INET);
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), p.first);
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, p.second);
}

void BittorrentHelperTest::testRemoveAnnounceUri()
{
  TorrentAttribute attrs;
  std::vector<std::string> tier1;
  tier1.push_back("http://host1/announce");
  std::vector<std::string> tier2;
  tier2.push_back("http://host2/announce");
  tier2.push_back("http://host3/announce");
  attrs.announceList.push_back(tier1);
  attrs.announceList.push_back(tier2);

  std::vector<std::string> removeUris;
  removeUris.push_back(tier1[0]);
  removeUris.push_back(tier2[0]);
  removeAnnounceUri(&attrs, removeUris);
  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs.announceList.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://host3/announce"),
                       attrs.announceList[0][0]);

  removeUris.clear();
  removeUris.push_back("*");

  removeAnnounceUri(&attrs, removeUris);
  CPPUNIT_ASSERT(attrs.announceList.empty());
}

void BittorrentHelperTest::testAddAnnounceUri()
{
  TorrentAttribute attrs;
  std::vector<std::string> addUris;
  addUris.push_back("http://host1/announce");
  addUris.push_back("http://host2/announce");
  addAnnounceUri(&attrs, addUris);
  CPPUNIT_ASSERT_EQUAL((size_t)2, attrs.announceList.size());

  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs.announceList[0].size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://host1/announce"),
                       attrs.announceList[0][0]);

  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs.announceList[1].size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://host2/announce"),
                       attrs.announceList[1][0]);
}

void BittorrentHelperTest::testAdjustAnnounceUri()
{
  TorrentAttribute attrs;
  std::vector<std::string> tier1;
  tier1.push_back("http://host1/announce");
  std::vector<std::string> tier2;
  tier2.push_back("http://host2/announce");
  tier2.push_back("http://host3/announce");
  attrs.announceList.push_back(tier1);
  attrs.announceList.push_back(tier2);

  std::shared_ptr<Option> option(new Option());
  option->put(PREF_BT_TRACKER, "http://host1/announce,http://host4/announce");
  option->put(PREF_BT_EXCLUDE_TRACKER,
              "http://host1/announce,http://host2/announce");
  adjustAnnounceUri(&attrs, option);

  CPPUNIT_ASSERT_EQUAL((size_t)3, attrs.announceList.size());

  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs.announceList[0].size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://host3/announce"),
                       attrs.announceList[0][0]);

  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs.announceList[1].size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://host1/announce"),
                       attrs.announceList[1][0]);

  CPPUNIT_ASSERT_EQUAL((size_t)1, attrs.announceList[2].size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://host4/announce"),
                       attrs.announceList[2][0]);
}

} // namespace bittorrent

} // namespace aria2
