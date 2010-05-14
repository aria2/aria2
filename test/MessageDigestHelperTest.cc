#include "MessageDigestHelper.h"

#include <cppunit/extensions/HelperMacros.h>

#include "util.h"
#include "DefaultDiskWriter.h"
#include "messageDigest.h"

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
  SharedHandle<DefaultDiskWriter> diskio
    (new DefaultDiskWriter("4096chunk.txt"));
  diskio->openExistingFile();
  CPPUNIT_ASSERT_EQUAL(std::string("608cabc0f2fa18c260cafd974516865c772363d5"),
                       MessageDigestHelper::digest
                       (MessageDigestContext::SHA1, diskio, 0, 4096));

  CPPUNIT_ASSERT_EQUAL(std::string("7a4a9ae537ebbbb826b1060e704490ad0f365ead"),
                       MessageDigestHelper::digest
                       (MessageDigestContext::SHA1, diskio, 5, 100));
}

void MessageDigestHelperTest::testDigestFilename()
{
  CPPUNIT_ASSERT_EQUAL(std::string("608cabc0f2fa18c260cafd974516865c772363d5"),
                       MessageDigestHelper::digest
                       (MessageDigestContext::SHA1, "4096chunk.txt"));
}

void MessageDigestHelperTest::testDigestData()
{
  std::string data = "aria2";
  CPPUNIT_ASSERT_EQUAL(std::string("f36003f22b462ffa184390533c500d8989e9f681"),
                       MessageDigestHelper::digest
                       (MessageDigestContext::SHA1, data.c_str(), data.size()));
}

} // namespace aria2
