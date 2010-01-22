#include "GZipEncoder.h"

#include <cppunit/extensions/HelperMacros.h>

#include "GZipDecoder.h"
#include "util.h"

namespace aria2 {

class GZipEncoderTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(GZipEncoderTest);
  CPPUNIT_TEST(testEncode);
  CPPUNIT_TEST_SUITE_END();
public:
  void testEncode();
};


CPPUNIT_TEST_SUITE_REGISTRATION(GZipEncoderTest);

void GZipEncoderTest::testEncode()
{
  GZipEncoder encoder;
  encoder.init();

  std::vector<std::string> inputs;
  inputs.push_back("Hello World");
  inputs.push_back("9223372036854775807");
  inputs.push_back("Fox");
  
  encoder << inputs[0];
  encoder << util::parseLLInt(inputs[1]);
  encoder << inputs[2].c_str();

  std::string gzippedData = encoder.str();

  GZipDecoder decoder;
  decoder.init();
  std::string gunzippedData =
    decoder.decode(reinterpret_cast<const unsigned char*>(gzippedData.data()),
                   gzippedData.size());
  CPPUNIT_ASSERT(decoder.finished());
  CPPUNIT_ASSERT_EQUAL(strjoin(inputs.begin(), inputs.end(), ""),
                       gunzippedData);
}

} // namespace aria2
