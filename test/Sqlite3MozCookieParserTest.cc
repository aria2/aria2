#include "Sqlite3MozCookieParser.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "RecoverableException.h"
#include "Util.h"

namespace aria2 {

class Sqlite3MozCookieParserTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Sqlite3MozCookieParserTest);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST(testParse_fileNotFound);
  CPPUNIT_TEST(testParse_badfile);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testParse();
  void testParse_fileNotFound();
  void testParse_badfile();
};


CPPUNIT_TEST_SUITE_REGISTRATION(Sqlite3MozCookieParserTest);

void Sqlite3MozCookieParserTest::testParse()
{
  Sqlite3MozCookieParser parser;
  std::deque<Cookie> cookies = parser.parse("cookies.sqlite");
  CPPUNIT_ASSERT_EQUAL((size_t)3, cookies.size());

  const Cookie& localhost = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string(".localhost.local"), localhost.getDomain());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), localhost.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), localhost.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), localhost.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)INT32_MAX, localhost.getExpiry());
  CPPUNIT_ASSERT_EQUAL(true, localhost.isSecureCookie());

  const Cookie& nullValue = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string(".null_value.local"), nullValue.getDomain());
  CPPUNIT_ASSERT_EQUAL(std::string("/path/to"), nullValue.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("uid"), nullValue.getName());
  CPPUNIT_ASSERT_EQUAL(std::string(""), nullValue.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)0, nullValue.getExpiry());
  CPPUNIT_ASSERT_EQUAL(false, nullValue.isSecureCookie());

  // See row id=3 has no name, so it is skipped.

  const Cookie& overflowTime = cookies[2];
  CPPUNIT_ASSERT_EQUAL(std::string(".overflow_time_t.local"),
		       overflowTime.getDomain());
  CPPUNIT_ASSERT_EQUAL(std::string("/path/to"), overflowTime.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), overflowTime.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), overflowTime.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)INT32_MAX, overflowTime.getExpiry());
  CPPUNIT_ASSERT_EQUAL(false, overflowTime.isSecureCookie());
}

void Sqlite3MozCookieParserTest::testParse_fileNotFound()
{
  Sqlite3MozCookieParser parser;
  try {
    parser.parse("fileNotFound");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // SUCCESS
    CPPUNIT_ASSERT(Util::startsWith(e.what(),
				    "Failed to open SQLite3 database:"));
  }
}

void Sqlite3MozCookieParserTest::testParse_badfile()
{
  Sqlite3MozCookieParser parser;
  try {
    parser.parse("badcookies.sqlite");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // SUCCESS
  }
}

} // namespace aria2
