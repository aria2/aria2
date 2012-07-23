#include "DefaultDiskWriter.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DefaultDiskWriterTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultDiskWriterTest);
  CPPUNIT_TEST(testSize);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testSize();
};


CPPUNIT_TEST_SUITE_REGISTRATION( DefaultDiskWriterTest );

void DefaultDiskWriterTest::testSize()
{
  DefaultDiskWriter dw(A2_TEST_DIR"/4096chunk.txt");
  dw.enableReadOnly();
  dw.openExistingFile();
  CPPUNIT_ASSERT_EQUAL((int64_t)4096LL, dw.size());
}

} // namespace aria2
