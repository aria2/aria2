#include "DefaultBtContext.h"
#include "Util.h"
#include "RecoverableException.h"
#include "AnnounceTier.h"
#include "FixedNumberRandomizer.h"
#include "FileEntry.h"
#include "array_fun.h"
#include <cstring>
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DefaultBtContextTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtContextTest);
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
  CPPUNIT_TEST(testGetAnnounceTier);
  CPPUNIT_TEST(testGetAnnounceTierAnnounceList);
  CPPUNIT_TEST(testGetPieceLength);
  CPPUNIT_TEST(testGetInfoHashAsString);
  CPPUNIT_TEST(testGetPeerId);
  CPPUNIT_TEST(testComputeFastSet);
  CPPUNIT_TEST(testGetFileEntries_multiFileUrlList);
  CPPUNIT_TEST(testGetFileEntries_singleFileUrlList);
  CPPUNIT_TEST(testLoadFromMemory);
  CPPUNIT_TEST(testLoadFromMemory_somethingMissing);
  CPPUNIT_TEST(testGetNodes);
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
  void testGetAnnounceTier();
  void testGetAnnounceTierAnnounceList();
  void testGetPieceLength();
  void testGetInfoHashAsString();
  void testGetPeerId();
  void testComputeFastSet();
  void testGetFileEntries_multiFileUrlList();
  void testGetFileEntries_singleFileUrlList();
  void testLoadFromMemory();
  void testLoadFromMemory_somethingMissing();
  void testGetNodes();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtContextTest);

void DefaultBtContextTest::testGetInfoHash() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  std::string correctHash = "248d0a1cd08284299de78d5c1ed359bb46717d8c";

  CPPUNIT_ASSERT_EQUAL((size_t)20, btContext.getInfoHashLength());
  CPPUNIT_ASSERT_EQUAL(correctHash, Util::toHex(btContext.getInfoHash(),
						btContext.getInfoHashLength()));
}

void DefaultBtContextTest::testGetPieceHash() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  CPPUNIT_ASSERT_EQUAL(Util::toHex((const unsigned char*)"AAAAAAAAAAAAAAAAAAAA", 20),
		       btContext.getPieceHash(0));
  CPPUNIT_ASSERT_EQUAL(Util::toHex((const unsigned char*)"BBBBBBBBBBBBBBBBBBBB", 20),
		       btContext.getPieceHash(1));
  CPPUNIT_ASSERT_EQUAL(Util::toHex((const unsigned char*)"CCCCCCCCCCCCCCCCCCCC", 20),
		       btContext.getPieceHash(2));
  CPPUNIT_ASSERT_EQUAL(std::string(""),
		       btContext.getPieceHash(-1));
  CPPUNIT_ASSERT_EQUAL(std::string(""),
		       btContext.getPieceHash(3));
}

void DefaultBtContextTest::testGetFileEntries() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");
  // This is multi-file torrent.
  std::deque<SharedHandle<FileEntry> > fileEntries = btContext.getFileEntries();
  // There are 2 file entries.
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntries.size());
  std::deque<SharedHandle<FileEntry> >::iterator itr = fileEntries.begin();

  SharedHandle<FileEntry> fileEntry1 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("aria2/src/aria2c"),
		       fileEntry1->getPath());
  itr++;
  SharedHandle<FileEntry> fileEntry2 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.2.2.tar.bz2"),
		       fileEntry2->getPath());
}

void DefaultBtContextTest::testGetFileEntriesSingle() {
  DefaultBtContext btContext;
  btContext.load("single.torrent");
  // This is multi-file torrent.
  std::deque<SharedHandle<FileEntry> > fileEntries = btContext.getFileEntries();
  // There is 1 file entry.
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries.size());
  std::deque<SharedHandle<FileEntry> >::iterator itr = fileEntries.begin();

  SharedHandle<FileEntry> fileEntry1 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.8.2.tar.bz2"),
		       fileEntry1->getPath());
}

void DefaultBtContextTest::testGetTotalLength() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  CPPUNIT_ASSERT_EQUAL(384ULL, btContext.getTotalLength());
}

void DefaultBtContextTest::testGetTotalLengthSingle() {
  DefaultBtContext btContext;
  btContext.load("single.torrent");

  CPPUNIT_ASSERT_EQUAL(384ULL, btContext.getTotalLength());
}

void DefaultBtContextTest::testGetFileModeMulti() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  CPPUNIT_ASSERT_EQUAL(BtContext::MULTI, btContext.getFileMode());
}

void DefaultBtContextTest::testGetFileModeSingle() {
  DefaultBtContext btContext;
  btContext.load("single.torrent");

  CPPUNIT_ASSERT_EQUAL(BtContext::SINGLE, btContext.getFileMode());
}

void DefaultBtContextTest::testGetNameMulti() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-test"), btContext.getName());
}

void DefaultBtContextTest::testGetNameSingle() {
  DefaultBtContext btContext;
  btContext.load("single.torrent");

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.8.2.tar.bz2"), btContext.getName());
}

void DefaultBtContextTest::testGetAnnounceTier() {
  DefaultBtContext btContext;
  btContext.load("single.torrent");

  std::deque<SharedHandle<AnnounceTier> > tiers = btContext.getAnnounceTiers();
  
  // There is 1 tier.
  CPPUNIT_ASSERT_EQUAL((size_t)1, tiers.size());

  std::deque<SharedHandle<AnnounceTier> >::iterator itr = tiers.begin();
  SharedHandle<AnnounceTier> tier1 = *itr;
  CPPUNIT_ASSERT_EQUAL((size_t)1, tier1->urls.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/announce.php"),
		       tier1->urls.at(0));

}

void DefaultBtContextTest::testGetAnnounceTierAnnounceList() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  std::deque<SharedHandle<AnnounceTier> > tiers = btContext.getAnnounceTiers();
  
  // There are 3 tiers.
  CPPUNIT_ASSERT_EQUAL((size_t)3, tiers.size());

  SharedHandle<AnnounceTier> tier1 = tiers.at(0);
  CPPUNIT_ASSERT_EQUAL((size_t)1, tier1->urls.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker1"),
		       tier1->urls.at(0));

  SharedHandle<AnnounceTier> tier2 = tiers.at(1);
  CPPUNIT_ASSERT_EQUAL((size_t)1, tier2->urls.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker2"), tier2->urls.at(0));

  SharedHandle<AnnounceTier> tier3 = tiers.at(2);
  CPPUNIT_ASSERT_EQUAL((size_t)1, tier3->urls.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker3"), tier3->urls.at(0));
}

void DefaultBtContextTest::testGetPieceLength() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  CPPUNIT_ASSERT_EQUAL((size_t)128, btContext.getPieceLength());
}

void DefaultBtContextTest::testGetInfoHashAsString() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  CPPUNIT_ASSERT_EQUAL(std::string("248d0a1cd08284299de78d5c1ed359bb46717d8c"),
		       btContext.getInfoHashAsString());
}

void DefaultBtContextTest::testGetPeerId() {
  DefaultBtContext btContext;
  btContext.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  CPPUNIT_ASSERT_EQUAL(std::string("%2daria2%2dAAAAAAAAAAAAA"), Util::torrentUrlencode(btContext.getPeerId(), 20));
}

void DefaultBtContextTest::testComputeFastSet()
{
  std::string ipaddr = "192.168.0.1";
  unsigned char infoHash[20];
  memset(infoHash, 0, sizeof(infoHash));
  infoHash[0] = 0xff;
  
  int fastSetSize = 10;

  DefaultBtContext btContext;
  btContext.setInfoHash(infoHash);
  btContext.setNumPieces(1000);

  {
    std::deque<size_t> fastSet;
    btContext.computeFastSet(fastSet, ipaddr, fastSetSize);
    size_t ans[] = { 686, 459, 278, 200, 404, 834, 64, 203, 760, 950 };
    std::deque<size_t> ansSet(&ans[0], &ans[arrayLength(ans)]);
    CPPUNIT_ASSERT(std::equal(fastSet.begin(), fastSet.end(), ansSet.begin()));
  }
  ipaddr = "10.0.0.1";
  {
    std::deque<size_t> fastSet;
    btContext.computeFastSet(fastSet, ipaddr, fastSetSize);
    size_t ans[] = { 568, 188, 466, 452, 550, 662, 109, 226, 398, 11 };
    std::deque<size_t> ansSet(&ans[0], &ans[arrayLength(ans)]);
    CPPUNIT_ASSERT(std::equal(fastSet.begin(), fastSet.end(), ansSet.begin()));
  }
  // See when pieces < fastSetSize
  btContext.setNumPieces(9);
  {
    std::deque<size_t> fastSet;
    btContext.computeFastSet(fastSet, ipaddr, fastSetSize);
    size_t ans[] = { 8, 6, 7, 5, 1, 4, 0, 2, 3 };
    std::deque<size_t> ansSet(&ans[0], &ans[arrayLength(ans)]);
    CPPUNIT_ASSERT(std::equal(fastSet.begin(), fastSet.end(), ansSet.begin()));
  }
}

void DefaultBtContextTest::testGetFileEntries_multiFileUrlList() {
  DefaultBtContext btContext;
  btContext.load("url-list-multiFile.torrent");
  // This is multi-file torrent.
  std::deque<SharedHandle<FileEntry> > fileEntries = btContext.getFileEntries();
  // There are 2 file entries.
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntries.size());
  std::deque<SharedHandle<FileEntry> >::iterator itr = fileEntries.begin();

  SharedHandle<FileEntry> fileEntry1 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("aria2/src/aria2c"),
		       fileEntry1->getPath());
  std::deque<std::string> uris1 = fileEntry1->getAssociatedUris();
  CPPUNIT_ASSERT_EQUAL((size_t)2, uris1.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/dist/aria2-test/aria2/src/aria2c"),
		       uris1[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/dist/aria2-test/aria2/src/aria2c"),
		       uris1[1]);

  itr++;
  SharedHandle<FileEntry> fileEntry2 = *itr;
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.2.2.tar.bz2"),
		       fileEntry2->getPath());
  std::deque<std::string> uris2 = fileEntry2->getAssociatedUris();
  CPPUNIT_ASSERT_EQUAL((size_t)2, uris2.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/dist/aria2-test/aria2-0.2.2.tar.bz2"),
		       uris2[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/dist/aria2-test/aria2-0.2.2.tar.bz2"),
		       uris2[1]);
}

void DefaultBtContextTest::testGetFileEntries_singleFileUrlList() {
  DefaultBtContext btContext;
  btContext.load("url-list-singleFile.torrent");
  // This is multi-file torrent.
  std::deque<SharedHandle<FileEntry> > fileEntries = btContext.getFileEntries();
  // There are 1 file entries.
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries.size());

  SharedHandle<FileEntry> fileEntry1 = fileEntries.front();
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"),
		       fileEntry1->getPath());
  std::deque<std::string> uris1 = fileEntry1->getAssociatedUris();
  CPPUNIT_ASSERT_EQUAL((size_t)1, uris1.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/dist/aria2.tar.bz2"),
		       uris1[0]);
}

void DefaultBtContextTest::testLoadFromMemory()
{
  std::string memory = "d8:announce36:http://aria.rednoah.com/announce.php13:announce-listll16:http://tracker1 el15:http://tracker2el15:http://tracker3ee7:comment17:REDNOAH.COM RULES13:creation datei1123456789e4:infod5:filesld6:lengthi284e4:pathl5:aria23:src6:aria2ceed6:lengthi100e4:pathl19:aria2-0.2.2.tar.bz2eee4:name10:aria2-test12:piece lengthi128e6:pieces60:AAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCee";

  DefaultBtContext btContext;
  btContext.loadFromMemory(memory, "default");

  std::string correctHash = "248d0a1cd08284299de78d5c1ed359bb46717d8c";

  CPPUNIT_ASSERT_EQUAL((size_t)20, btContext.getInfoHashLength());
  CPPUNIT_ASSERT_EQUAL(correctHash, Util::toHex(btContext.getInfoHash(),
						btContext.getInfoHashLength()));
}

void DefaultBtContextTest::testLoadFromMemory_somethingMissing()
{
  // pieces missing
  try {
    std::string memory = "d8:announce36:http://aria.rednoah.com/announce.php4:infod4:name13:aria2.tar.bz26:lengthi262144eee";
    DefaultBtContext btContext;
    btContext.loadFromMemory(memory, "default");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

void DefaultBtContextTest::testGetNodes()
{
  {
    std::string memory =
      "d5:nodesl"
      "l11:192.168.0.1i6881ee"
      "l11:192.168.0.24:6882e"
      "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
      "12:piece lengthi262144e"
      "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
      "ee";
    DefaultBtContext btContext;
    btContext.loadFromMemory(memory, "default");

    const std::deque<std::pair<std::string, uint16_t> >& nodes =
      btContext.getNodes();
    CPPUNIT_ASSERT_EQUAL((size_t)2, nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, nodes[0].second);
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), nodes[1].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, nodes[1].second);
  }
  {
    // empty hostname
    std::string memory =
      "d5:nodesl"
      "l1: i6881ee"
      "l11:192.168.0.24:6882e"
      "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
      "12:piece lengthi262144e"
      "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
      "ee";
    DefaultBtContext btContext;
    btContext.loadFromMemory(memory, "default");

    const std::deque<std::pair<std::string, uint16_t> >& nodes =
      btContext.getNodes();
    CPPUNIT_ASSERT_EQUAL((size_t)1, nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, nodes[0].second);
  }
  {
    // bad port 
    std::string memory =
      "d5:nodesl"
      "l11:192.168.0.11:xe"
      "l11:192.168.0.24:6882e"
      "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
      "12:piece lengthi262144e"
      "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
      "ee";
    DefaultBtContext btContext;
    btContext.loadFromMemory(memory, "default");

    const std::deque<std::pair<std::string, uint16_t> >& nodes =
      btContext.getNodes();
    CPPUNIT_ASSERT_EQUAL((size_t)1, nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, nodes[0].second);
  }
  {
    // port missing
    std::string memory =
      "d5:nodesl"
      "l11:192.168.0.1e"
      "l11:192.168.0.24:6882e"
      "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
      "12:piece lengthi262144e"
      "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
      "ee";
    DefaultBtContext btContext;
    btContext.loadFromMemory(memory, "default");

    const std::deque<std::pair<std::string, uint16_t> >& nodes =
      btContext.getNodes();
    CPPUNIT_ASSERT_EQUAL((size_t)1, nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, nodes[0].second);
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
    DefaultBtContext btContext;
    btContext.loadFromMemory(memory, "default");

    const std::deque<std::pair<std::string, uint16_t> >& nodes =
      btContext.getNodes();
    CPPUNIT_ASSERT_EQUAL((size_t)0, nodes.size());
  }
  {
    // the element of node is not Data
    std::string memory =
      "d5:nodesl"
      "ll11:192.168.0.1i6881eee"
      "l11:192.168.0.24:6882e"
      "e4:infod4:name13:aria2.tar.bz26:lengthi262144e"
      "12:piece lengthi262144e"
      "6:pieces20:AAAAAAAAAAAAAAAAAAAA"
      "ee";
    DefaultBtContext btContext;
    btContext.loadFromMemory(memory, "default");

    const std::deque<std::pair<std::string, uint16_t> >& nodes =
      btContext.getNodes();
    CPPUNIT_ASSERT_EQUAL((size_t)1, nodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), nodes[0].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6882, nodes[0].second);
  }

}

} // namespace aria2
