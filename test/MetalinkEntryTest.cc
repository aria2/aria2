#include "MetalinkEntry.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class MetalinkEntryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkEntryTest);
  CPPUNIT_TEST(testDropUnsupportedResource);
  CPPUNIT_TEST(testReorderResourcesByPreference);
  CPPUNIT_TEST(testCheck);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }
  void tearDown() {
  }

  void testDropUnsupportedResource();
  void testReorderResourcesByPreference();
  void testCheck();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MetalinkEntryTest );

MetalinkEntry* createTestEntry() {
  MetalinkEntry* entry = new MetalinkEntry();
  MetalinkResource* res1 = new MetalinkResource();
  res1->url = "ftp://myhost/aria2.tar.bz2";
  res1->type = MetalinkResource::TYPE_FTP;
  res1->location = "RO";
  res1->preference = 50;
  MetalinkResource* res2 = new MetalinkResource();
  res2->url = "http://myhost/aria2.tar.bz2";
  res2->type = MetalinkResource::TYPE_HTTP;
  res2->location = "AT";
  res2->preference = 100;
  MetalinkResource* res3 = new MetalinkResource();
  res3->url = "http://myhost/aria2.torrent";
  res3->type = MetalinkResource::TYPE_BITTORRENT;
  res3->location = "al";
  res3->preference = 60;
  MetalinkResource* res4 = new MetalinkResource();
  res4->url = "http://myhost/aria2.ext";
  res4->type = MetalinkResource::TYPE_NOT_SUPPORTED;
  res4->location = "ad";
  res4->preference = 10;
  MetalinkResource* res5 = new MetalinkResource();
  res5->url = "https://myhost/aria2.tar.bz2";
  res5->type = MetalinkResource::TYPE_HTTPS;
  res5->location = "jp";
  res5->preference = 90;

  entry->resources.push_back(res1);
  entry->resources.push_back(res2);
  entry->resources.push_back(res3);
  entry->resources.push_back(res4);
  entry->resources.push_back(res5);
  return entry;
}

void MetalinkEntryTest::testDropUnsupportedResource() {
  MetalinkEntry* entry = createTestEntry();

  entry->dropUnsupportedResource();

  CPPUNIT_ASSERT_EQUAL(4, (int)entry->resources.size());
  
  CPPUNIT_ASSERT_EQUAL((int)MetalinkResource::TYPE_FTP,
		       entry->resources.at(0)->type);
  CPPUNIT_ASSERT_EQUAL((int)MetalinkResource::TYPE_HTTP,
		       entry->resources.at(1)->type);
  CPPUNIT_ASSERT_EQUAL((int)MetalinkResource::TYPE_BITTORRENT,
		       entry->resources.at(2)->type);
  CPPUNIT_ASSERT_EQUAL((int)MetalinkResource::TYPE_HTTPS,
		       entry->resources.at(3)->type);
}

void MetalinkEntryTest::testReorderResourcesByPreference() {
  MetalinkEntry* entry = createTestEntry();
  
  entry->reorderResourcesByPreference();

  CPPUNIT_ASSERT_EQUAL(100, entry->resources.at(0)->preference);
  CPPUNIT_ASSERT_EQUAL(90, entry->resources.at(1)->preference);
  CPPUNIT_ASSERT_EQUAL(60, entry->resources.at(2)->preference);
  CPPUNIT_ASSERT_EQUAL(50, entry->resources.at(3)->preference);
  CPPUNIT_ASSERT_EQUAL(10, entry->resources.at(4)->preference);
}

void MetalinkEntryTest::testCheck() {
  MetalinkEntry entry;
  string filename = "4096chunk.txt";
  CPPUNIT_ASSERT(entry.check(filename));
  entry.md5 = "82a7348c2e03731109d0cf45a7325b88";
  CPPUNIT_ASSERT(entry.check(filename));
  entry.md5 = "00000000000000000000000000000000";
  CPPUNIT_ASSERT(!entry.check(filename));
  entry.sha1 = "608cabc0f2fa18c260cafd974516865c772363d5";
  CPPUNIT_ASSERT(entry.check(filename));
}
