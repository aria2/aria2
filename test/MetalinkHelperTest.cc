#include "metalink_helper.h"

#include <cppunit/extensions/HelperMacros.h>

#include "MetalinkEntry.h"
#include "Option.h"
#include "prefs.h"
#include "MetalinkMetaurl.h"
#include "a2functional.h"

namespace aria2 {

class MetalinkHelperTest : public CppUnit::TestFixture {

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

CPPUNIT_TEST_SUITE_REGISTRATION(MetalinkHelperTest);

void MetalinkHelperTest::testParseAndQuery()
{
  Option option;
  auto result = metalink::parseAndQuery(A2_TEST_DIR "/test.xml", &option);
  CPPUNIT_ASSERT_EQUAL((size_t)5, result.size());
}

void MetalinkHelperTest::testParseAndQuery_version()
{
  Option option;
  option.put(PREF_METALINK_VERSION, "0.5.1");
  auto result = metalink::parseAndQuery(A2_TEST_DIR "/test.xml", &option);
  CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
  auto& entry = result.front();
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.1.tar.bz2"), entry->getPath());
}

void MetalinkHelperTest::testGroupEntryByMetaurlName()
{
  std::vector<std::unique_ptr<MetalinkEntry>> entries;

  auto e1 = make_unique<MetalinkEntry>();
  e1->version = "1";
  e1->sizeKnown = true;
  // no name
  e1->metaurls.push_back(
      make_unique<MetalinkMetaurl>("http://meta1", "torrent", "", 1));

  auto e2 = make_unique<MetalinkEntry>();
  e2->version = "2";
  e2->sizeKnown = true;

  auto e3 = make_unique<MetalinkEntry>();
  e3->version = "3";
  e3->sizeKnown = true;
  e3->metaurls.push_back(
      make_unique<MetalinkMetaurl>("http://meta2", "torrent", "f3", 1));

  auto e4 = make_unique<MetalinkEntry>();
  e4->version = "4";
  e4->sizeKnown = true;
  e4->metaurls.push_back(
      make_unique<MetalinkMetaurl>("http://meta1", "torrent", "f4", 1));

  auto e5 = make_unique<MetalinkEntry>();
  e5->version = "5";
  // no size
  e5->metaurls.push_back(
      make_unique<MetalinkMetaurl>("http://meta1", "torrent", "f5", 1));

  auto e6 = make_unique<MetalinkEntry>();
  e6->version = "6";
  e6->sizeKnown = true;
  e6->metaurls.push_back(
      make_unique<MetalinkMetaurl>("http://meta1", "torrent", "f6", 1));

  entries.push_back(std::move(e1));
  entries.push_back(std::move(e2));
  entries.push_back(std::move(e3));
  entries.push_back(std::move(e4));
  entries.push_back(std::move(e5));
  entries.push_back(std::move(e6));

  auto result = metalink::groupEntryByMetaurlName(entries);

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
