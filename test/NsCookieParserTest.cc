#include "NsCookieParser.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "RecoverableException.h"
#include "Util.h"

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
  CPPUNIT_ASSERT_EQUAL((size_t)5, cookies.size());

  Cookie c = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string(".localhost.local"), c.getDomain());

  c = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string("user"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("me"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)1181473200, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string(".expired.local"), c.getDomain());

  c = cookies[2];
  CPPUNIT_ASSERT_EQUAL(std::string("passwd"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("secret"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("/cgi-bin"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string(".localhost.local"), c.getDomain());

  c = cookies[3];
  CPPUNIT_ASSERT_EQUAL(std::string("TAX"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("1000"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string(".overflow.local"), c.getDomain());

  c = cookies[4];
  CPPUNIT_ASSERT_EQUAL(std::string("novalue"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string(""), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string(".localhost.local"), c.getDomain());
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
