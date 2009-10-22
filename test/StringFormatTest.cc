#include "StringFormat.h"
#include "Exception.h"
#include "util.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class StringFormatTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(StringFormatTest);
  CPPUNIT_TEST(testStr);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testStr();
};


CPPUNIT_TEST_SUITE_REGISTRATION(StringFormatTest);

void StringFormatTest::testStr()
{
  int major = 1;
  int minor = 0;
  int release = 7;
  StringFormat fmt("aria2-%d.%d.%d-%s", major, minor, release, "beta");

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.7-beta"), fmt.str());
}

} // namespace aria2
