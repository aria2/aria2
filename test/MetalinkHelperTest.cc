#include "metalink_helper.h"

#include <cppunit/extensions/HelperMacros.h>

#include "MetalinkEntry.h"
#include "Option.h"
#include "prefs.h"
#include "MetalinkMetaurl.h"

namespace aria2 {

class MetalinkHelperTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkHelperTest);
  CPPUNIT_TEST(testParseAndQuery);
  CPPUNIT_TEST(testParseAndQuery_version);
  CPPUNIT_TEST(testGroupEntryByMetaurlName);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void testParseAndQuery();
  void testParseAndQuery_version();
  void testGroupEntryByMetaurlName();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MetalinkHelperTest );

void MetalinkHelperTest::testParseAndQuery()
{
  Option option;
  std::vector<SharedHandle<MetalinkEntry> > result;
  metalink::parseAndQuery(result, A2_TEST_DIR"/test.xml", &option);
  CPPUNIT_ASSERT_EQUAL((size_t)5, result.size());
}

void MetalinkHelperTest::testParseAndQuery_version()
{
  Option option;
  option.put(PREF_METALINK_VERSION, "0.5.1");
  std::vector<SharedHandle<MetalinkEntry> > result;
  metalink::parseAndQuery(result, A2_TEST_DIR"/test.xml", &option);
  CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
  SharedHandle<MetalinkEntry> entry = result.front();
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.1.tar.bz2"), entry->getPath());
}

void MetalinkHelperTest::testGroupEntryByMetaurlName()
{
  std::vector<SharedHandle<MetalinkEntry> > entries;

  SharedHandle<MetalinkEntry> e1(new MetalinkEntry());
  e1->version = "1";
  e1->sizeKnown = true;
  // no name
  e1->metaurls.push_back
    (SharedHandle<MetalinkMetaurl>
     (new MetalinkMetaurl("http://meta1", "torrent", "", 1)));

  SharedHandle<MetalinkEntry> e2(new MetalinkEntry());
  e2->version = "2";
  e2->sizeKnown = true;

  SharedHandle<MetalinkEntry> e3(new MetalinkEntry());
  e3->version = "3";
  e3->sizeKnown = true;
  e3->metaurls.push_back
    (SharedHandle<MetalinkMetaurl>
     (new MetalinkMetaurl("http://meta2", "torrent", "f3", 1)));

  SharedHandle<MetalinkEntry> e4(new MetalinkEntry());
  e4->version = "4";
  e4->sizeKnown = true;
  e4->metaurls.push_back
    (SharedHandle<MetalinkMetaurl>
     (new MetalinkMetaurl("http://meta1", "torrent", "f4", 1)));

  SharedHandle<MetalinkEntry> e5(new MetalinkEntry());
  e5->version = "5";
  // no size
  e5->metaurls.push_back
    (SharedHandle<MetalinkMetaurl>
     (new MetalinkMetaurl("http://meta1", "torrent", "f5", 1)));

  SharedHandle<MetalinkEntry> e6(new MetalinkEntry());
  e6->version = "6";
  e6->sizeKnown = true;
  e6->metaurls.push_back
    (SharedHandle<MetalinkMetaurl>
     (new MetalinkMetaurl("http://meta1", "torrent", "f6", 1)));

  entries.push_back(e1);
  entries.push_back(e2);
  entries.push_back(e3);
  entries.push_back(e4);
  entries.push_back(e5);
  entries.push_back(e6);

  std::vector<std::pair<std::string,
    std::vector<SharedHandle<MetalinkEntry> > > > result;
  metalink::groupEntryByMetaurlName(result, entries);

  CPPUNIT_ASSERT_EQUAL(std::string("http://meta1"), result[0].first);
  CPPUNIT_ASSERT_EQUAL(std::string("1"), result[0].second[0]->version);
  CPPUNIT_ASSERT_EQUAL(std::string(""), result[1].first);
  CPPUNIT_ASSERT_EQUAL(std::string("2"), result[1].second[0]->version);  
  CPPUNIT_ASSERT_EQUAL(std::string("http://meta2"), result[2].first);
  CPPUNIT_ASSERT_EQUAL(std::string("3"), result[2].second[0]->version);  
  CPPUNIT_ASSERT_EQUAL(std::string("http://meta1"), result[3].first);
  CPPUNIT_ASSERT_EQUAL(std::string("4"), result[3].second[0]->version);
  CPPUNIT_ASSERT_EQUAL(std::string("6"), result[3].second[1]->version);
}

} // namespace aria2
