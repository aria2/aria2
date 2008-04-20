#include "MessageDigestHelper.h"
#include "Util.h"
#include "DefaultDiskWriter.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class MessageDigestHelperTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MessageDigestHelperTest);
  CPPUNIT_TEST(testDigestDiskWriter);
  CPPUNIT_TEST(testDigestFilename);
  CPPUNIT_TEST(testDigestData);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testDigestDiskWriter();
  void testDigestFilename();
  void testDigestData();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MessageDigestHelperTest );

void MessageDigestHelperTest::testDigestDiskWriter() {
  SharedHandle<DefaultDiskWriter> diskio(new DefaultDiskWriter());
  diskio->openExistingFile("4096chunk.txt");
  CPPUNIT_ASSERT_EQUAL(std::string("608cabc0f2fa18c260cafd974516865c772363d5"),
		       MessageDigestHelper::digest("sha1", diskio, 0, 4096));

  CPPUNIT_ASSERT_EQUAL(std::string("7a4a9ae537ebbbb826b1060e704490ad0f365ead"),
		       MessageDigestHelper::digest("sha1", diskio, 5, 100));
}

void MessageDigestHelperTest::testDigestFilename()
{
  CPPUNIT_ASSERT_EQUAL(std::string("608cabc0f2fa18c260cafd974516865c772363d5"),
		       MessageDigestHelper::digest("sha1", "4096chunk.txt"));
}

void MessageDigestHelperTest::testDigestData()
{
  std::string data = "aria2";
  CPPUNIT_ASSERT_EQUAL(std::string("f36003f22b462ffa184390533c500d8989e9f681"),
		       MessageDigestHelper::digest("sha1", data.c_str(), data.size()));
}

} // namespace aria2
