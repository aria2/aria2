#include "MetalinkHelper.h"
#include "MetalinkEntry.h"
#include "Option.h"
#include "prefs.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

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
  std::deque<SharedHandle<MetalinkEntry> > result;
  MetalinkHelper::parseAndQuery(result, "test.xml", &option);
  CPPUNIT_ASSERT_EQUAL((size_t)5, result.size());
}

void MetalinkHelperTest::testParseAndQuery_version()
{
  Option option;
  option.put(PREF_METALINK_VERSION, "0.5.1");
  std::deque<SharedHandle<MetalinkEntry> > result;
  MetalinkHelper::parseAndQuery(result, "test.xml", &option);
  CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
  SharedHandle<MetalinkEntry> entry = result.front();
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.1.tar.bz2"), entry->getPath());
}

} // namespace aria2
