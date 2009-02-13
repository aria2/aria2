#include "GZipDecoder.h"
#include "Exception.h"
#include "Util.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST
#include <iostream>
#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class GZipDecoderTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(GZipDecoderTest);
  CPPUNIT_TEST(testDecode);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testDecode();
};


CPPUNIT_TEST_SUITE_REGISTRATION(GZipDecoderTest);

void GZipDecoderTest::testDecode()
{
  GZipDecoder decoder;
  decoder.init();

  std::string outfile("/tmp/aria2_GZipDecoderTest_testDecode");

  char buf[4096];
  std::ifstream in("gzip_decode_test.gz", std::ios::binary);
  std::ofstream out(outfile.c_str(), std::ios::binary);
  while(in) {
    in.read(buf, sizeof(buf));

    std::string r = decoder.decode
      (reinterpret_cast<const unsigned char*>(buf), in.gcount());

    out.write(r.data(), r.size());
  }
  CPPUNIT_ASSERT(decoder.finished());
  decoder.release();

  out.close();

#ifdef ENABLE_MESSAGE_DIGEST
  CPPUNIT_ASSERT_EQUAL(std::string("8b577b33c0411b2be9d4fa74c7402d54a8d21f96"),
		       MessageDigestHelper::digest(MessageDigestContext::SHA1,
						   outfile));
#endif // ENABLE_MESSAGE_DIGEST
}

} // namespace aria2
