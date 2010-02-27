#include "MetalinkEntry.h"

#include <cppunit/extensions/HelperMacros.h>

#include "MetalinkResource.h"

namespace aria2 {

class MetalinkEntryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkEntryTest);
  CPPUNIT_TEST(testDropUnsupportedResource);
  CPPUNIT_TEST(testReorderResourcesByPriority);
  CPPUNIT_TEST(testSetLocationPriority);
  CPPUNIT_TEST(testSetProtocolPriority);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }
  void tearDown() {
  }

  void testDropUnsupportedResource();
  void testReorderResourcesByPriority();
  void testSetLocationPriority();
  void testSetProtocolPriority();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MetalinkEntryTest );

SharedHandle<MetalinkEntry> createTestEntry() {
  SharedHandle<MetalinkEntry> entry(new MetalinkEntry());
  SharedHandle<MetalinkResource> res1(new MetalinkResource());
  res1->url = "ftp://myhost/aria2.tar.bz2";
  res1->type = MetalinkResource::TYPE_FTP;
  res1->location = "ro";
  res1->priority = 50;
  SharedHandle<MetalinkResource> res2(new MetalinkResource());
  res2->url = "http://myhost/aria2.tar.bz2";
  res2->type = MetalinkResource::TYPE_HTTP;
  res2->location = "at";
  res2->priority = 1;
  SharedHandle<MetalinkResource> res3(new MetalinkResource());
  res3->url = "http://myhost/aria2.torrent";
  res3->type = MetalinkResource::TYPE_BITTORRENT;
  res3->location = "al";
  res3->priority = 40;
  SharedHandle<MetalinkResource> res4(new MetalinkResource());
  res4->url = "http://myhost/aria2.ext";
  res4->type = MetalinkResource::TYPE_NOT_SUPPORTED;
  res4->location = "ad";
  res4->priority = 90;
  SharedHandle<MetalinkResource> res5(new MetalinkResource());
  res5->url = "https://myhost/aria2.tar.bz2";
  res5->type = MetalinkResource::TYPE_HTTPS;
  res5->location = "jp";
  res5->priority = 10;

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
  
  std::vector<SharedHandle<MetalinkResource> >::const_iterator itr =
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

void MetalinkEntryTest::testReorderResourcesByPriority() {
  SharedHandle<MetalinkEntry> entry(createTestEntry());
  
  entry->reorderResourcesByPriority();

  CPPUNIT_ASSERT_EQUAL(1, entry->resources.at(0)->priority);
  CPPUNIT_ASSERT_EQUAL(10, entry->resources.at(1)->priority);
  CPPUNIT_ASSERT_EQUAL(40, entry->resources.at(2)->priority);
  CPPUNIT_ASSERT_EQUAL(50, entry->resources.at(3)->priority);
  CPPUNIT_ASSERT_EQUAL(90, entry->resources.at(4)->priority);
}

void MetalinkEntryTest::testSetLocationPriority()
{
  SharedHandle<MetalinkEntry> entry(createTestEntry());

  const char* locationsSrc[] = { "jp", "al", "ro" };

  std::vector<std::string> locations(&locationsSrc[0], &locationsSrc[3]);

  entry->setLocationPriority(locations, -100);

  CPPUNIT_ASSERT_EQUAL(std::string("ro"), entry->resources[0]->location);
  CPPUNIT_ASSERT_EQUAL(-50, entry->resources[0]->priority);
  CPPUNIT_ASSERT_EQUAL(std::string("at"), entry->resources[1]->location);
  CPPUNIT_ASSERT_EQUAL(1, entry->resources[1]->priority);
  CPPUNIT_ASSERT_EQUAL(std::string("al"), entry->resources[2]->location);
  CPPUNIT_ASSERT_EQUAL(-60, entry->resources[2]->priority);
  CPPUNIT_ASSERT_EQUAL(std::string("ad"), entry->resources[3]->location);
  CPPUNIT_ASSERT_EQUAL(90, entry->resources[3]->priority);
  CPPUNIT_ASSERT_EQUAL(std::string("jp"), entry->resources[4]->location);
  CPPUNIT_ASSERT_EQUAL(-90, entry->resources[4]->priority);
}

void MetalinkEntryTest::testSetProtocolPriority()
{
  SharedHandle<MetalinkEntry> entry(createTestEntry());
  entry->setProtocolPriority("http", -1);
  CPPUNIT_ASSERT_EQUAL(50, entry->resources[0]->priority); // ftp
  CPPUNIT_ASSERT_EQUAL(0, entry->resources[1]->priority); // http, -1
  CPPUNIT_ASSERT_EQUAL(40, entry->resources[2]->priority); // bittorrent
  CPPUNIT_ASSERT_EQUAL(90, entry->resources[3]->priority); // not supported
  CPPUNIT_ASSERT_EQUAL(10, entry->resources[4]->priority); // https
}

} // namespace aria2
