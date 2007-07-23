#include "DefaultBtContext.h"
#include "Util.h"
#include "Exception.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

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
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtContextTest);

void DefaultBtContextTest::testGetInfoHash() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  string correctHash = "248d0a1cd08284299de78d5c1ed359bb46717d8c";

  CPPUNIT_ASSERT_EQUAL((int32_t)20, btContext.getInfoHashLength());
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
  CPPUNIT_ASSERT_EQUAL(string(""),
		       btContext.getPieceHash(-1));
  CPPUNIT_ASSERT_EQUAL(string(""),
		       btContext.getPieceHash(3));
}

void DefaultBtContextTest::testGetFileEntries() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");
  // This is multi-file torrent.
  FileEntries fileEntries = btContext.getFileEntries();
  // There are 2 file entries.
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntries.size());
  FileEntries::iterator itr = fileEntries.begin();

  FileEntryHandle fileEntry1 = *itr;
  CPPUNIT_ASSERT_EQUAL(string("aria2/src/aria2c"),
		       fileEntry1->getPath());
  itr++;
  FileEntryHandle fileEntry2 = *itr;
  CPPUNIT_ASSERT_EQUAL(string("aria2-0.2.2.tar.bz2"),
		       fileEntry2->getPath());
}

void DefaultBtContextTest::testGetFileEntriesSingle() {
  DefaultBtContext btContext;
  btContext.load("single.torrent");
  // This is multi-file torrent.
  FileEntries fileEntries = btContext.getFileEntries();
  // There is 1 file entry.
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries.size());
  FileEntries::iterator itr = fileEntries.begin();

  FileEntryHandle fileEntry1 = *itr;
  CPPUNIT_ASSERT_EQUAL(string("aria2-0.8.2.tar.bz2"),
		       fileEntry1->getPath());
}

void DefaultBtContextTest::testGetTotalLength() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  CPPUNIT_ASSERT_EQUAL((long long int)384,
		       btContext.getTotalLength());
}

void DefaultBtContextTest::testGetTotalLengthSingle() {
  DefaultBtContext btContext;
  btContext.load("single.torrent");

  CPPUNIT_ASSERT_EQUAL((long long int)384,
		       btContext.getTotalLength());
}

void DefaultBtContextTest::testGetFileModeMulti() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  CPPUNIT_ASSERT_EQUAL(BtContext::MULTI,
		       btContext.getFileMode());
}

void DefaultBtContextTest::testGetFileModeSingle() {
  DefaultBtContext btContext;
  btContext.load("single.torrent");

  CPPUNIT_ASSERT_EQUAL(BtContext::SINGLE,
		       btContext.getFileMode());
}

void DefaultBtContextTest::testGetNameMulti() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  CPPUNIT_ASSERT_EQUAL(string("aria2-test"),
		       btContext.getName());
}

void DefaultBtContextTest::testGetNameSingle() {
  DefaultBtContext btContext;
  btContext.load("single.torrent");

  CPPUNIT_ASSERT_EQUAL(string("aria2-0.8.2.tar.bz2"),
		       btContext.getName());
}

void DefaultBtContextTest::testGetAnnounceTier() {
  DefaultBtContext btContext;
  btContext.load("single.torrent");

  AnnounceTiers tiers = btContext.getAnnounceTiers();
  
  // There is 1 tier.
  CPPUNIT_ASSERT_EQUAL((size_t)1,
		       tiers.size());

  AnnounceTiers::iterator itr = tiers.begin();
  AnnounceTierHandle tier1 = *itr;
  CPPUNIT_ASSERT_EQUAL((size_t)1,
		       tier1->urls.size());
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com/announce.php"),
		       tier1->urls.at(0));

}

void DefaultBtContextTest::testGetAnnounceTierAnnounceList() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  AnnounceTiers tiers = btContext.getAnnounceTiers();
  
  // There are 3 tiers.
  CPPUNIT_ASSERT_EQUAL((size_t)3,
		       tiers.size());

  AnnounceTierHandle tier1 = tiers.at(0);
  CPPUNIT_ASSERT_EQUAL((size_t)1,
		       tier1->urls.size());
  CPPUNIT_ASSERT_EQUAL(string("http://tracker1"),
		       tier1->urls.at(0));

  AnnounceTierHandle tier2 = tiers.at(1);
  CPPUNIT_ASSERT_EQUAL((size_t)1,
		       tier2->urls.size());
  CPPUNIT_ASSERT_EQUAL(string("http://tracker2"),
		       tier2->urls.at(0));

  AnnounceTierHandle tier3 = tiers.at(2);
  CPPUNIT_ASSERT_EQUAL((size_t)1,
		       tier3->urls.size());
  CPPUNIT_ASSERT_EQUAL(string("http://tracker3"),
		       tier3->urls.at(0));
  
}

void DefaultBtContextTest::testGetPieceLength() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  CPPUNIT_ASSERT_EQUAL((int32_t)128,
		       btContext.getPieceLength());
}

void DefaultBtContextTest::testGetInfoHashAsString() {
  DefaultBtContext btContext;
  btContext.load("test.torrent");

  CPPUNIT_ASSERT_EQUAL(string("248d0a1cd08284299de78d5c1ed359bb46717d8c"),
		       btContext.getInfoHashAsString());
}

void DefaultBtContextTest::testGetPeerId() {
  DefaultBtContext btContext;
  Util::torrentUrlencode(btContext.getPeerId(), 20);
}
