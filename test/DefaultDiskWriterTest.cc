#include "DefaultDiskWriter.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class DefaultDiskWriterTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultDiskWriterTest);
  CPPUNIT_TEST(testSha1Sum);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testSha1Sum();
};


CPPUNIT_TEST_SUITE_REGISTRATION( DefaultDiskWriterTest );

void DefaultDiskWriterTest::testSha1Sum() {
  DefaultDiskWriter dw;
  dw.openExistingFile("4096chunk.txt");
  
  CPPUNIT_ASSERT_EQUAL(string("608cabc0f2fa18c260cafd974516865c772363d5"),
		       dw.sha1Sum(0, 4096));

  CPPUNIT_ASSERT_EQUAL(string("7a4a9ae537ebbbb826b1060e704490ad0f365ead"),
		       dw.sha1Sum(5, 100));

  dw.closeFile();
}
