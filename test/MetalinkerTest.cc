#include "Metalinker.h"
#include "MetalinkEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class MetalinkerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkerTest);
  CPPUNIT_TEST(testQueryEntry);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }
  void tearDown() {
  }

  void testQueryEntry();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MetalinkerTest );

void MetalinkerTest::testQueryEntry() {
  SharedHandle<Metalinker> metalinker(new Metalinker());
  SharedHandle<MetalinkEntry> entry1(new MetalinkEntry());
  entry1->version = "0.5.2";
  entry1->language = "en-US";
  entry1->os = "Linux-x86";
  SharedHandle<MetalinkEntry> entry2(new MetalinkEntry());
  entry2->version = "0.5.1";
  entry2->language = "ja-JP";
  entry2->os = "Linux-m68k";
  metalinker->entries.push_back(entry1);
  metalinker->entries.push_back(entry2);

  std::string version;
  std::string language;
  std::string os;

  version = "0.5.1";
  language = "ja-JP";
  os = "Linux-m68k";
  std::deque<SharedHandle<MetalinkEntry> > entries =
    metalinker->queryEntry(version, language, os);
  CPPUNIT_ASSERT_EQUAL((size_t)1, entries.size());
  CPPUNIT_ASSERT_EQUAL(std::string("0.5.1"), entries.at(0)->version);
  CPPUNIT_ASSERT_EQUAL(std::string("ja-JP"), entries.at(0)->language);
  CPPUNIT_ASSERT_EQUAL(std::string("Linux-m68k"), entries.at(0)->os);

  version = "0.6.0";
  language = "";
  os = "";
  CPPUNIT_ASSERT_EQUAL((size_t)0,
		       metalinker->queryEntry(version, language, os).size());

  version = "0.5.2";
  language = "";
  os = "";
  entries = metalinker->queryEntry(version, language, os);
  CPPUNIT_ASSERT_EQUAL((size_t)1, entries.size());
  CPPUNIT_ASSERT_EQUAL(std::string("0.5.2"), entries.at(0)->version);
  CPPUNIT_ASSERT_EQUAL(std::string("en-US"), entries.at(0)->language);
  CPPUNIT_ASSERT_EQUAL(std::string("Linux-x86"), entries.at(0)->os);
}

} // namespace aria2
