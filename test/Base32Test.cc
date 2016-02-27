#include "base32.h"

#include <cppunit/extensions/HelperMacros.h>

#include "util.h"

namespace aria2 {

class Base32Test : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Base32Test);
  CPPUNIT_TEST(testEncode);
  CPPUNIT_TEST(testDecode);
  CPPUNIT_TEST_SUITE_END();

public:
  void testEncode();
  void testDecode();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Base32Test);

void Base32Test::testEncode()
{
  CPPUNIT_ASSERT_EQUAL(std::string(), base32::encode(""));
  CPPUNIT_ASSERT_EQUAL(std::string("GE======"), base32::encode("1"));
  CPPUNIT_ASSERT_EQUAL(std::string("GEZA===="), base32::encode("12"));
  CPPUNIT_ASSERT_EQUAL(std::string("GEZDG==="), base32::encode("123"));
  CPPUNIT_ASSERT_EQUAL(std::string("GEZDGNA="), base32::encode("1234"));
  CPPUNIT_ASSERT_EQUAL(std::string("GEZDGNBV"), base32::encode("12345"));
  std::string s = "248d0a1cd08284";
  CPPUNIT_ASSERT_EQUAL(std::string("ESGQUHGQQKCA===="),
                       base32::encode(util::fromHex(s.begin(), s.end())));
}

void Base32Test::testDecode()
{
  std::string s = "";
  CPPUNIT_ASSERT_EQUAL(std::string(), base32::decode(s.begin(), s.end()));
  s = "GE======";
  CPPUNIT_ASSERT_EQUAL(std::string("1"), base32::decode(s.begin(), s.end()));
  s = "GEZA====";
  CPPUNIT_ASSERT_EQUAL(std::string("12"), base32::decode(s.begin(), s.end()));
  s = "GEZDG===";
  CPPUNIT_ASSERT_EQUAL(std::string("123"), base32::decode(s.begin(), s.end()));
  s = "GEZDGNA=";
  CPPUNIT_ASSERT_EQUAL(std::string("1234"), base32::decode(s.begin(), s.end()));
  s = "GEZDGNBV";
  CPPUNIT_ASSERT_EQUAL(std::string("12345"),
                       base32::decode(s.begin(), s.end()));
  s = "gezdgnbv";
  CPPUNIT_ASSERT_EQUAL(std::string("12345"),
                       base32::decode(s.begin(), s.end()));
}

} // namespace aria2
