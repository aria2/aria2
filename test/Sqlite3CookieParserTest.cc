#include "Sqlite3CookieParserImpl.h"

#include <cstring>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "RecoverableException.h"
#include "util.h"
#include "array_fun.h"

namespace aria2 {

class Sqlite3CookieParserTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Sqlite3CookieParserTest);
  CPPUNIT_TEST(testMozParse);
  CPPUNIT_TEST(testMozParse_fileNotFound);
  CPPUNIT_TEST(testMozParse_badfile);
  CPPUNIT_TEST(testChromumParse);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testMozParse();
  void testMozParse_fileNotFound();
  void testMozParse_badfile();
  void testChromumParse();
};


CPPUNIT_TEST_SUITE_REGISTRATION(Sqlite3CookieParserTest);

void Sqlite3CookieParserTest::testMozParse()
{
  Sqlite3MozCookieParser parser(A2_TEST_DIR"/cookies.sqlite");
  std::vector<Cookie> cookies;
  parser.parse(cookies);
  CPPUNIT_ASSERT_EQUAL((size_t)3, cookies.size());

  const Cookie& localhost = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), localhost.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), localhost.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)INT32_MAX, localhost.getExpiryTime());
  CPPUNIT_ASSERT(localhost.getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), localhost.getDomain());
  CPPUNIT_ASSERT(localhost.getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), localhost.getPath());
  CPPUNIT_ASSERT(localhost.getSecure());
  CPPUNIT_ASSERT_EQUAL((time_t)3000, localhost.getLastAccessTime());
  CPPUNIT_ASSERT_EQUAL((time_t)3000, localhost.getCreationTime());

  const Cookie& nullValue = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string("uid"), nullValue.getName());
  CPPUNIT_ASSERT_EQUAL(std::string(""), nullValue.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)0, nullValue.getExpiryTime());
  CPPUNIT_ASSERT(nullValue.getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("null_value.com"), nullValue.getDomain());
  CPPUNIT_ASSERT(!nullValue.getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/path/to"), nullValue.getPath());
  CPPUNIT_ASSERT(!nullValue.getSecure());

  // See row id=3 has no name, so it is skipped.

  const Cookie& overflowTime = cookies[2];
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), overflowTime.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), overflowTime.getValue());
  CPPUNIT_ASSERT((time_t)INT32_MAX <= overflowTime.getExpiryTime());
  CPPUNIT_ASSERT(overflowTime.getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("overflow.time_t.org"),
                       overflowTime.getDomain());
  CPPUNIT_ASSERT(!overflowTime.getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/path/to"), overflowTime.getPath());
  CPPUNIT_ASSERT(!overflowTime.getSecure());

  // See row id=5 has bad path, so it is skipped.
}

void Sqlite3CookieParserTest::testMozParse_fileNotFound()
{
  Sqlite3MozCookieParser parser("fileNotFound");
  try {
    std::vector<Cookie> cookies;
    parser.parse(cookies);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // SUCCESS
    const char A2_SQLITE_ERR[] = "SQLite3 database is not opened";
    CPPUNIT_ASSERT(util::startsWith(e.what(), e.what()+strlen(e.what()),
                                    A2_SQLITE_ERR, vend(A2_SQLITE_ERR)-1));
  }
}

void Sqlite3CookieParserTest::testMozParse_badfile()
{
  Sqlite3MozCookieParser parser(A2_TEST_DIR"/badcookies.sqlite");
  try {
    std::vector<Cookie> cookies;
    parser.parse(cookies);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // SUCCESS
  }
}

void Sqlite3CookieParserTest::testChromumParse()
{
  Sqlite3ChromiumCookieParser parser(A2_TEST_DIR"/chromium_cookies.sqlite");
  std::vector<Cookie> cookies;
  parser.parse(cookies);
  CPPUNIT_ASSERT_EQUAL((size_t)3, cookies.size());

  const Cookie& sfnet = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string("mykey"), sfnet.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("pass"), sfnet.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)12345679, sfnet.getExpiryTime());
  CPPUNIT_ASSERT(sfnet.getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sourceforge.net"),
                       sfnet.getDomain());
  CPPUNIT_ASSERT(!sfnet.getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), sfnet.getPath());
  CPPUNIT_ASSERT(!sfnet.getSecure());

  const Cookie& sfjp = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string("myseckey"), sfjp.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("pass2"), sfjp.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)0, sfjp.getExpiryTime());
  CPPUNIT_ASSERT(sfjp.getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sourceforge.jp"), sfjp.getDomain());
  CPPUNIT_ASSERT(sfjp.getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/profile"), sfjp.getPath());
  CPPUNIT_ASSERT(sfjp.getSecure());

  const Cookie& localnet = cookies[2];
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), localnet.getDomain());
  CPPUNIT_ASSERT(sfjp.getHostOnly());
  CPPUNIT_ASSERT_EQUAL((time_t)3000, localnet.getLastAccessTime());
  CPPUNIT_ASSERT_EQUAL((time_t)3000, localnet.getCreationTime());
}

} // namespace aria2
