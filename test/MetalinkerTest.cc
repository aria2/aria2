#include "Metalinker.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

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
  MetalinkerHandle metalinker(new Metalinker());
  MetalinkEntryHandle entry1(new MetalinkEntry());
  entry1->version = "0.5.2";
  entry1->language = "en-US";
  entry1->os = "Linux-x86";
  MetalinkEntryHandle entry2(new MetalinkEntry());
  entry2->version = "0.5.1";
  entry2->language = "ja-JP";
  entry2->os = "Linux-m68k";
  metalinker->entries.push_back(entry1);
  metalinker->entries.push_back(entry2);

  string version;
  string language;
  string os;

  version = "0.5.1";
  language = "ja-JP";
  os = "Linux-m68k";
  MetalinkEntries entries = metalinker->queryEntry(version,
						 language,
						 os);
  CPPUNIT_ASSERT_EQUAL((size_t)1, entries.size());
  CPPUNIT_ASSERT_EQUAL(string("0.5.1"), entries.at(0)->version);
  CPPUNIT_ASSERT_EQUAL(string("ja-JP"), entries.at(0)->language);
  CPPUNIT_ASSERT_EQUAL(string("Linux-m68k"), entries.at(0)->os);

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
  CPPUNIT_ASSERT_EQUAL(string("0.5.2"), entries.at(0)->version);
  CPPUNIT_ASSERT_EQUAL(string("en-US"), entries.at(0)->language);
  CPPUNIT_ASSERT_EQUAL(string("Linux-x86"), entries.at(0)->os);
}
