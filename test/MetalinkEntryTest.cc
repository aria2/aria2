#include "MetalinkEntry.h"
#include "MetalinkResource.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class MetalinkEntryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkEntryTest);
  CPPUNIT_TEST(testDropUnsupportedResource);
  CPPUNIT_TEST(testReorderResourcesByPreference);
  CPPUNIT_TEST(testSetLocationPreference);
  CPPUNIT_TEST(testSetProtocolPreference);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }
  void tearDown() {
  }

  void testDropUnsupportedResource();
  void testReorderResourcesByPreference();
  void testSetLocationPreference();
  void testSetProtocolPreference();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MetalinkEntryTest );

SharedHandle<MetalinkEntry> createTestEntry() {
  SharedHandle<MetalinkEntry> entry(new MetalinkEntry());
  SharedHandle<MetalinkResource> res1(new MetalinkResource());
  res1->url = "ftp://myhost/aria2.tar.bz2";
  res1->type = MetalinkResource::TYPE_FTP;
  res1->location = "RO";
  res1->preference = 50;
  SharedHandle<MetalinkResource> res2(new MetalinkResource());
  res2->url = "http://myhost/aria2.tar.bz2";
  res2->type = MetalinkResource::TYPE_HTTP;
  res2->location = "AT";
  res2->preference = 100;
  SharedHandle<MetalinkResource> res3(new MetalinkResource());
  res3->url = "http://myhost/aria2.torrent";
  res3->type = MetalinkResource::TYPE_BITTORRENT;
  res3->location = "AL";
  res3->preference = 60;
  SharedHandle<MetalinkResource> res4(new MetalinkResource());
  res4->url = "http://myhost/aria2.ext";
  res4->type = MetalinkResource::TYPE_NOT_SUPPORTED;
  res4->location = "AD";
  res4->preference = 10;
  SharedHandle<MetalinkResource> res5(new MetalinkResource());
  res5->url = "https://myhost/aria2.tar.bz2";
  res5->type = MetalinkResource::TYPE_HTTPS;
  res5->location = "JP";
  res5->preference = 90;

  entry->resources.push_back(res1);
  entry->resources.push_back(res2);
  entry->resources.push_back(res3);
  entry->resources.push_back(res4);
  entry->resources.push_back(res5);
  return entry;
}

void MetalinkEntryTest::testDropUnsupportedResource() {
  SharedHandle<MetalinkEntry> entry(createTestEntry());

  entry->dropUnsupportedResource();
#if defined ENABLE_SSL && defined ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL((size_t)4, entry->resources.size());
#elif defined ENABLE_SSL || defined ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL((size_t)3, entry->resources.size());
#else
  CPPUNIT_ASSERT_EQUAL((size_t)2, entry->resources.size());
#endif // ENABLE_MESSAGE_DIGEST
  
  std::deque<SharedHandle<MetalinkResource> >::const_iterator itr =
    entry->resources.begin();
  CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP,
		       (*itr++)->type);
  CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_HTTP,
		       (*itr++)->type);
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_BITTORRENT,
		       (*itr++)->type);
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_SSL
  CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_HTTPS,
		       (*itr++)->type);
#endif // ENABLE_SSL
}

void MetalinkEntryTest::testReorderResourcesByPreference() {
  SharedHandle<MetalinkEntry> entry(createTestEntry());
  
  entry->reorderResourcesByPreference();

  CPPUNIT_ASSERT_EQUAL(100, entry->resources.at(0)->preference);
  CPPUNIT_ASSERT_EQUAL(90, entry->resources.at(1)->preference);
  CPPUNIT_ASSERT_EQUAL(60, entry->resources.at(2)->preference);
  CPPUNIT_ASSERT_EQUAL(50, entry->resources.at(3)->preference);
  CPPUNIT_ASSERT_EQUAL(10, entry->resources.at(4)->preference);
}

void MetalinkEntryTest::testSetLocationPreference()
{
  SharedHandle<MetalinkEntry> entry(createTestEntry());

  const char* locationsSrc[] = { "jp", "al", "RO" };

  std::deque<std::string> locations(&locationsSrc[0], &locationsSrc[3]);

  entry->setLocationPreference(locations, 100);

  CPPUNIT_ASSERT_EQUAL(std::string("RO"), entry->resources[0]->location);
  CPPUNIT_ASSERT_EQUAL(150, entry->resources[0]->preference);
  CPPUNIT_ASSERT_EQUAL(std::string("AT"), entry->resources[1]->location);
  CPPUNIT_ASSERT_EQUAL(100, entry->resources[1]->preference);
  CPPUNIT_ASSERT_EQUAL(std::string("AL"), entry->resources[2]->location);
  CPPUNIT_ASSERT_EQUAL(160, entry->resources[2]->preference);
  CPPUNIT_ASSERT_EQUAL(std::string("AD"), entry->resources[3]->location);
  CPPUNIT_ASSERT_EQUAL(10, entry->resources[3]->preference);
  CPPUNIT_ASSERT_EQUAL(std::string("JP"), entry->resources[4]->location);
  CPPUNIT_ASSERT_EQUAL(190, entry->resources[4]->preference);
}

void MetalinkEntryTest::testSetProtocolPreference()
{
  SharedHandle<MetalinkEntry> entry(createTestEntry());
  entry->setProtocolPreference("http", 1);
  CPPUNIT_ASSERT_EQUAL(50, entry->resources[0]->preference); // ftp
  CPPUNIT_ASSERT_EQUAL(101, entry->resources[1]->preference); // http, +1
  CPPUNIT_ASSERT_EQUAL(60, entry->resources[2]->preference); // bittorrent
  CPPUNIT_ASSERT_EQUAL(10, entry->resources[3]->preference); // not supported
  CPPUNIT_ASSERT_EQUAL(90, entry->resources[4]->preference); // https
}

} // namespace aria2
