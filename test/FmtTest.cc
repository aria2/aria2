#include "fmt.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class FmtTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(FmtTest);
  CPPUNIT_TEST(testFmt);
  CPPUNIT_TEST_SUITE_END();

public:
  void testFmt();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FmtTest);

void FmtTest::testFmt()
{
  int major = 1;
  int minor = 0;
  int release = 7;
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.7-beta"),
                       fmt("aria2-%d.%d.%d-%s", major, minor, release, "beta"));
}

} // namespace aria2
