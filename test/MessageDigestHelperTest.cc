#include "message_digest_helper.h"

#include <cppunit/extensions/HelperMacros.h>

#include "util.h"
#include "DefaultDiskWriter.h"
#include "MessageDigest.h"

namespace aria2 {

class MessageDigestHelperTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MessageDigestHelperTest);
  CPPUNIT_TEST(testDigestDiskWriter);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testDigestDiskWriter();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MessageDigestHelperTest);

void MessageDigestHelperTest::testDigestDiskWriter()
{
  auto diskio =
      std::make_shared<DefaultDiskWriter>(A2_TEST_DIR "/4096chunk.txt");
  diskio->enableReadOnly();
  diskio->openExistingFile();
  CPPUNIT_ASSERT_EQUAL(std::string("608cabc0f2fa18c260cafd974516865c772363d5"),
                       util::toHex(message_digest::digest(
                           MessageDigest::sha1().get(), diskio, 0, 4_k)));

  CPPUNIT_ASSERT_EQUAL(std::string("7a4a9ae537ebbbb826b1060e704490ad0f365ead"),
                       util::toHex(message_digest::digest(
                           MessageDigest::sha1().get(), diskio, 5, 100)));
}

} // namespace aria2
