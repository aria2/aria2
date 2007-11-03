#include "MetalinkHelper.h"
#include "MetalinkEntry.h"
#include "Option.h"
#include "prefs.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class MetalinkHelperTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkHelperTest);
  CPPUNIT_TEST(testParseAndQuery);
  CPPUNIT_TEST(testParseAndQuery_version);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void tearDown() {}

  void testParseAndQuery();
  void testParseAndQuery_version();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MetalinkHelperTest );

void MetalinkHelperTest::testParseAndQuery()
{
  Option option;
  MetalinkEntries entries = MetalinkHelper::parseAndQuery("test.xml", &option);
  CPPUNIT_ASSERT_EQUAL((size_t)5, entries.size());
}

void MetalinkHelperTest::testParseAndQuery_version()
{
  Option option;
  option.put(PREF_METALINK_VERSION, "0.5.1");
  MetalinkEntries entries = MetalinkHelper::parseAndQuery("test.xml", &option);
  CPPUNIT_ASSERT_EQUAL((size_t)1, entries.size());
  MetalinkEntryHandle entry = entries.front();
  CPPUNIT_ASSERT_EQUAL(string("aria2-0.5.1.tar.bz2"), entry->getPath());
}
