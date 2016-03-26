#include "cookie_helper.h"

#include <limits>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "Cookie.h"

namespace aria2 {

class CookieHelperTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(CookieHelperTest);
  CPPUNIT_TEST(testParseDate);
  CPPUNIT_TEST(testDomainMatch);
  CPPUNIT_TEST(testPathMatch);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST(testReverseDomainLevel);
  CPPUNIT_TEST_SUITE_END();

public:
  void testParseDate();
  void testDomainMatch();
  void testPathMatch();
  void testParse();
  void testReverseDomainLevel();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CookieHelperTest);

void CookieHelperTest::testParseDate()
{
  // RFC1123
  time_t time = 0;
  std::string s = "Sat, 06 Sep 2008 15:26:33 GMT";
  CPPUNIT_ASSERT(cookie::parseDate(time, s.begin(), s.end()));
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793, time);
  // RFC850
  s = "Saturday, 06-Sep-08 15:26:33 GMT";
  CPPUNIT_ASSERT(cookie::parseDate(time, s.begin(), s.end()));
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793, time);
  // ANSI C's asctime()
  s = "Sun Sep  6 15:26:33 2008";
  CPPUNIT_ASSERT(cookie::parseDate(time, s.begin(), s.end()));
  CPPUNIT_ASSERT_EQUAL((time_t)1220714793, time);
  s = "Thu Jan 1 0:0:0 1970";
  CPPUNIT_ASSERT(cookie::parseDate(time, s.begin(), s.end()));
  CPPUNIT_ASSERT_EQUAL((time_t)0, time);

  s = "Thu Jan 1 1970 0:";
  CPPUNIT_ASSERT(!cookie::parseDate(time, s.begin(), s.end()));
  s = "Thu Jan 1 1970 0:0";
  CPPUNIT_ASSERT(!cookie::parseDate(time, s.begin(), s.end()));
  s = "Thu Jan 1 1970 0:0:";
  CPPUNIT_ASSERT(!cookie::parseDate(time, s.begin(), s.end()));

  // Leap year
  s = "Tue, 29 Feb 2000 00:00:00 GMT";
  CPPUNIT_ASSERT(cookie::parseDate(time, s.begin(), s.end()));
  s = "Thu, 29 Feb 2001 00:00:00 GMT";
  CPPUNIT_ASSERT(!cookie::parseDate(time, s.begin(), s.end()));
}

void CookieHelperTest::testDomainMatch()
{
  CPPUNIT_ASSERT(cookie::domainMatch("localhost", "localhost"));
  CPPUNIT_ASSERT(cookie::domainMatch("192.168.0.1", "192.168.0.1"));
  CPPUNIT_ASSERT(cookie::domainMatch("www.example.org", "example.org"));
  CPPUNIT_ASSERT(!cookie::domainMatch("192.168.0.1", "0.1"));
  CPPUNIT_ASSERT(!cookie::domainMatch("example.org", "example.com"));
  CPPUNIT_ASSERT(!cookie::domainMatch("example.org", "www.example.org"));
}

void CookieHelperTest::testPathMatch()
{
  CPPUNIT_ASSERT(cookie::pathMatch("/", "/"));
  CPPUNIT_ASSERT(cookie::pathMatch("/foo/", "/foo"));
  CPPUNIT_ASSERT(!cookie::pathMatch("/bar/", "/foo"));
  CPPUNIT_ASSERT(!cookie::pathMatch("/foo", "/bar/foo"));
  CPPUNIT_ASSERT(cookie::pathMatch("/foo/bar", "/foo/"));
}

void CookieHelperTest::testParse()
{
  time_t creationDate = 141;
  {
    std::string str = "ID=123456789; expires=Sun, 10-Jun-2007 11:00:00 GMT;"
                      "path=/foo; domain=localhost; secure;httpOnly   ";
    auto c = cookie::parse(str, "localhost", "/", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL(std::string("ID"), c->getName());
    CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c->getValue());
    CPPUNIT_ASSERT_EQUAL((time_t)1181473200, c->getExpiryTime());
    CPPUNIT_ASSERT(c->getPersistent());
    CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c->getDomain());
    CPPUNIT_ASSERT(!c->getHostOnly());
    CPPUNIT_ASSERT_EQUAL(std::string("/foo"), c->getPath());
    CPPUNIT_ASSERT(c->getSecure());
    CPPUNIT_ASSERT(c->getHttpOnly());
    CPPUNIT_ASSERT_EQUAL((time_t)141, c->getCreationTime());
    CPPUNIT_ASSERT_EQUAL((time_t)141, c->getLastAccessTime());
  }
  {
    std::string str = "id=; Max-Age=0;";
    auto c = cookie::parse(str, "localhost", "/", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL(std::string("id"), c->getName());
    CPPUNIT_ASSERT_EQUAL((time_t)0, c->getExpiryTime());
    CPPUNIT_ASSERT(c->getPersistent());
    CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c->getDomain());
    CPPUNIT_ASSERT(c->getHostOnly());
    CPPUNIT_ASSERT_EQUAL(std::string("/"), c->getPath());
    CPPUNIT_ASSERT(!c->getSecure());
    CPPUNIT_ASSERT(!c->getHttpOnly());
  }
  {
    std::string str = "id=; Max-Age=-100;";
    auto c = cookie::parse(str, "localhost", "/", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL((time_t)0, c->getExpiryTime());
    CPPUNIT_ASSERT(c->getPersistent());
  }
  {
    std::string str = "id=; Max-Age=100;";
    auto c = cookie::parse(str, "localhost", "/", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL((time_t)creationDate + 100, c->getExpiryTime());
    CPPUNIT_ASSERT(c->getPersistent());
  }
  {
    std::string str = "id=; Max-Age=9223372036854775807;";
    auto c = cookie::parse(str, "localhost", "/", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL(std::numeric_limits<time_t>::max(),
                         c->getExpiryTime());
    CPPUNIT_ASSERT(c->getPersistent());
  }
  {
    std::string str = "id=; Max-Age=X;";
    CPPUNIT_ASSERT(!cookie::parse(str, "localhost", "/", creationDate));
  }
  {
    std::string str = "id=; Max-Age=100garbage;";
    CPPUNIT_ASSERT(!cookie::parse(str, "localhost", "/", creationDate));
  }
  {
    std::string str = "id=; Max-Age=100;expires=Sun, 10-Jun-2007 11:00:00 GMT;";
    auto c = cookie::parse(str, "localhost", "/", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL((time_t)creationDate + 100, c->getExpiryTime());
    CPPUNIT_ASSERT(c->getPersistent());
  }
  {
    // Cookie data cannot be parsed.
    std::string str = "id=; expires=2007-10-01 11:00:00 GMT;";
    CPPUNIT_ASSERT(!cookie::parse(str, "localhost", "/", creationDate));
  }
  {
    std::string str = "id=;";
    auto c = cookie::parse(str, "localhost", "/", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT(!c->getPersistent());
  }
  {
    std::string str = "id=; path=abc";
    auto c = cookie::parse(str, "localhost", "/", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL(std::string("/"), c->getPath());
  }
  {
    std::string str = "id=; domain=.example.org";
    CPPUNIT_ASSERT(cookie::parse(str, "www.example.org", "/", creationDate));
  }
  {
    // Fails because request host does not domain-match with cookie
    // domain.
    std::string str = "id=; domain=www.example.org";
    CPPUNIT_ASSERT(!cookie::parse(str, "example.org", "/", creationDate));
  }
  {
    std::string str = "id=; domain=.";
    CPPUNIT_ASSERT(!cookie::parse(str, "localhost", "/", creationDate));
  }
  {
    std::string str = "";
    CPPUNIT_ASSERT(!cookie::parse(str, "localhost", "/", creationDate));
  }
  {
    std::string str = "=";
    CPPUNIT_ASSERT(!cookie::parse(str, "localhost", "/", creationDate));
  }
  {
    // Use domain last time seen.
    std::string str = "id=;domain=a.example.org;domain=.example.org";
    auto c = cookie::parse(str, "b.example.org", "/", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL(std::string("example.org"), c->getDomain());
  }
  {
    // numeric host
    std::string str = "id=;";
    auto c = cookie::parse(str, "192.168.0.1", "/", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), c->getDomain());
    CPPUNIT_ASSERT(c->getHostOnly());
  }
  {
    // numeric host
    std::string str = "id=; domain=192.168.0.1";
    auto c = cookie::parse(str, "192.168.0.1", "/", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), c->getDomain());
    CPPUNIT_ASSERT(c->getHostOnly());
  }
  {
    // DQUOTE around cookie-value
    std::string str = "id=\"foo\";";
    auto c = cookie::parse(str, "localhost", "/", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL(std::string("foo"), c->getValue());
  }
  {
    // Default path
    std::string str = "id=;";
    auto c = cookie::parse(str, "localhost", "/foo", creationDate);
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL(std::string("/foo"), c->getPath());
  }
}

void CookieHelperTest::testReverseDomainLevel()
{
  CPPUNIT_ASSERT_EQUAL(std::string("net.sourceforge.aria2"),
                       cookie::reverseDomainLevel("aria2.sourceforge.net"));
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"),
                       cookie::reverseDomainLevel("localhost"));
  // Behavior check
  CPPUNIT_ASSERT_EQUAL(std::string(""), cookie::reverseDomainLevel(""));
  CPPUNIT_ASSERT_EQUAL(std::string(""), cookie::reverseDomainLevel("."));
  CPPUNIT_ASSERT_EQUAL(std::string("foo."), cookie::reverseDomainLevel(".foo"));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), cookie::reverseDomainLevel("foo."));
}

} // namespace aria2
