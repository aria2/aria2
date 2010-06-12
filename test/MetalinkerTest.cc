#include "Metalinker.h"

#include <cppunit/extensions/HelperMacros.h>

#include "MetalinkEntry.h"

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
  entry1->languages.push_back("en-US");
  entry1->oses.push_back("Linux-x86");
  SharedHandle<MetalinkEntry> entry2(new MetalinkEntry());
  entry2->version = "0.5.1";
  entry2->languages.push_back("ja-JP");
  entry2->oses.push_back("Linux-m68k");
  metalinker->addEntry(entry1);
  metalinker->addEntry(entry2);

  std::string version;
  std::string language;
  std::string os;

  version = "0.5.1";
  language = "ja-JP";
  os = "Linux-m68k";
  {
    std::vector<SharedHandle<MetalinkEntry> > result;
    metalinker->queryEntry(result, version, language, os);
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    CPPUNIT_ASSERT_EQUAL(std::string("0.5.1"), result.at(0)->version);
    CPPUNIT_ASSERT_EQUAL(std::string("ja-JP"), result.at(0)->languages[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("Linux-m68k"), result.at(0)->oses[0]);
  }
  version = "0.6.0";
  language = "";
  os = "";
  {
    std::vector<SharedHandle<MetalinkEntry> > result;
    metalinker->queryEntry(result, version, language, os);
    CPPUNIT_ASSERT_EQUAL((size_t)0, result.size());
  }

  version = "0.5.2";
  language = "";
  os = "";
  {
    std::vector<SharedHandle<MetalinkEntry> > result;
    metalinker->queryEntry(result, version, language, os);
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    CPPUNIT_ASSERT_EQUAL(std::string("0.5.2"), result.at(0)->version);
    CPPUNIT_ASSERT_EQUAL(std::string("en-US"), result.at(0)->languages[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("Linux-x86"), result.at(0)->oses[0]);
  }
}

} // namespace aria2
