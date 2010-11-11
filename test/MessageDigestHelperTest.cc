#include "MessageDigestHelper.h"

#include <cppunit/extensions/HelperMacros.h>

#include "util.h"
#include "DefaultDiskWriter.h"
#include "MessageDigest.h"

namespace aria2 {

class MessageDigestHelperTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MessageDigestHelperTest);
  CPPUNIT_TEST(testHexDigestDiskWriter);
  CPPUNIT_TEST(testDigest);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testHexDigestDiskWriter();
  void testDigest();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MessageDigestHelperTest );

void MessageDigestHelperTest::testHexDigestDiskWriter() {
  SharedHandle<DefaultDiskWriter> diskio
    (new DefaultDiskWriter("4096chunk.txt"));
  diskio->openExistingFile();
  CPPUNIT_ASSERT_EQUAL(std::string("608cabc0f2fa18c260cafd974516865c772363d5"),
                       MessageDigestHelper::hexDigest
                       (MessageDigest::sha1(), diskio, 0, 4096));

  CPPUNIT_ASSERT_EQUAL(std::string("7a4a9ae537ebbbb826b1060e704490ad0f365ead"),
                       MessageDigestHelper::hexDigest
                       (MessageDigest::sha1(), diskio, 5, 100));
}

void MessageDigestHelperTest::testDigest()
{
  std::string data = "aria2";
  SharedHandle<MessageDigest> sha1 = MessageDigest::sha1();
  sha1->update(data.data(), data.size());
  CPPUNIT_ASSERT_EQUAL(std::string("f36003f22b462ffa184390533c500d8989e9f681"),
                       sha1->hexDigest());
}

} // namespace aria2
