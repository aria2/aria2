#include "NsCookieParser.h"
#include "RecoverableException.h"
#include "Util.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class NsCookieParserTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(NsCookieParserTest);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST(testParse_fileNotFound);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testParse();
  void testParse_fileNotFound();
};


CPPUNIT_TEST_SUITE_REGISTRATION(NsCookieParserTest);

void NsCookieParserTest::testParse()
{
  NsCookieParser parser;
  std::deque<Cookie> cookies = parser.parse("nscookietest.txt");
  CPPUNIT_ASSERT_EQUAL((size_t)4, cookies.size());

  Cookie c = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)1181473200, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.domain);

  c = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string("user"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string("me"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)1181473200, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.domain);

  c = cookies[2];
  CPPUNIT_ASSERT_EQUAL(std::string("passwd"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string("secret"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/cgi-bin"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.domain);

  c = cookies[3];
  CPPUNIT_ASSERT_EQUAL(std::string("novalue"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string(""), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.domain);
}

void NsCookieParserTest::testParse_fileNotFound()
{
  NsCookieParser parser;
  try {
    parser.parse("fileNotFound");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // SUCCESS
  }
}

} // namespace aria2
