#include "Cookie.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "TimeA2.h"

namespace aria2 {

class CookieTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(CookieTest);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testOperatorEqual);
  CPPUNIT_TEST(testMatch);
  CPPUNIT_TEST(testIsExpired);
  CPPUNIT_TEST(testNormalizeDomain);
  CPPUNIT_TEST(testToNsCookieFormat);
  CPPUNIT_TEST(testMarkOriginServerOnly);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testValidate();
  void testOperatorEqual();
  void testMatch();
  void testIsExpired();
  void testNormalizeDomain();
  void testToNsCookieFormat();
  void testMarkOriginServerOnly();
};


CPPUNIT_TEST_SUITE_REGISTRATION(CookieTest);

void CookieTest::testValidate()
{
  {
    Cookie defaultDomainPath("k", "v", "/", "localhost", false);
    CPPUNIT_ASSERT(defaultDomainPath.validate("localhost", "/"));
  }
  {
    Cookie domainStartsWithoutDot("k", "v", "/", "aria2.org", false);
    CPPUNIT_ASSERT(domainStartsWithoutDot.validate("www.aria2.org", "/"));
    Cookie domainStartsWithDot("k", "v", "/", ".aria2.org", false);
    CPPUNIT_ASSERT(domainStartsWithDot.validate("www.aria2.org", "/"));
  }
  {
    Cookie domainWithoutEmbeddedDot("k", "v", "/", ".org", false);
    CPPUNIT_ASSERT(!domainWithoutEmbeddedDot.validate("aria2.org", "/"));
  }
  {
    Cookie domainEndsWithDot("k", "v", "/", ".aria2.org.", false);
    CPPUNIT_ASSERT(!domainEndsWithDot.validate("www.aria2.org", "/"));
  }
  {
    Cookie domainHD("k", "v", "/", ".aria2.org", false);
    CPPUNIT_ASSERT(!domainHD.validate("aria2.www.aria2.org", "/"));
  }
  {
    Cookie pathNotStartsWith("k", "v", "/downloads", "localhost", false);
    CPPUNIT_ASSERT(!pathNotStartsWith.validate("localhost", "/examples"));
  }
  {
    Cookie pathStartsWith("k", "v", "/downloads", "localhost", false);
    CPPUNIT_ASSERT(pathStartsWith.validate("localhost", "/downloads/latest/"));
  }
  {
    Cookie pathSlash("k", "v", "/downloads", "localhost", false);
    CPPUNIT_ASSERT(pathSlash.validate("localhost", "/downloads/"));
  }
  {
    Cookie pathSlash("k", "v", "/downloads/", "localhost", false);
    CPPUNIT_ASSERT(pathSlash.validate("localhost", "/downloads"));
  }
  {
    Cookie pathNotMatch("k", "v", "/downloads", "localhost", false);
    CPPUNIT_ASSERT(!pathNotMatch.validate("localhost", "/downloadss"));
  }
  {
    Cookie pathNotMatch("k", "v", "/downloads/", "localhost", false);
    CPPUNIT_ASSERT(!pathNotMatch.validate("localhost", "/downloadss"));
  }
  {
    Cookie pathNotMatch("k", "v", "/downloads", "localhost", false);
    CPPUNIT_ASSERT(!pathNotMatch.validate("localhost", "/downloadss/"));
  }
  {
    Cookie nameEmpty("", "v", "/", "localhost", false);
    CPPUNIT_ASSERT(!nameEmpty.validate("localhost", "/"));
  }
  {
    Cookie fromNumericHost("k", "v", "/", "192.168.1.1", false);
    CPPUNIT_ASSERT(fromNumericHost.validate("192.168.1.1", "/"));
    CPPUNIT_ASSERT(!fromNumericHost.validate("www.aria2.org", "/"));
  }
}

void CookieTest::testOperatorEqual()
{
  Cookie a("k", "v", "/", "localhost", false);
  Cookie b("k", "v", "/", "LOCALHOST", true);
  Cookie wrongPath("k", "v", "/a", "localhost", false);
  Cookie wrongDomain("k", "v", "/", "mydomain", false);
  Cookie wrongName("h", "v", "/a", "localhost", false);
  Cookie caseSensitiveName("K", "v", "/a", "localhost", false);
  CPPUNIT_ASSERT(a == b);
  CPPUNIT_ASSERT(!(a == wrongPath));
  CPPUNIT_ASSERT(!(a == wrongDomain));
  CPPUNIT_ASSERT(!(a == wrongName));
  CPPUNIT_ASSERT(!(a == caseSensitiveName));
  // normalize
  Cookie n1("k", "v", "/", "localhost", false);
  Cookie n2("k", "v", "/", ".localhost", false);
  Cookie n3("k", "v", "/", "localhost.local", false);
  Cookie n4("k", "v", "/", ".localhost.local", false);
  CPPUNIT_ASSERT(n1 == n2);
  CPPUNIT_ASSERT(n1 == n3);
  CPPUNIT_ASSERT(n1 == n4);
}

void CookieTest::testMatch()
{
  Cookie c("k", "v", "/downloads", ".aria2.org", false);
  Cookie c2("k", "v", "/downloads/", ".aria2.org", false);
  Cookie c3("k", "v", "/downloads/", "aria2.org", false);
  Cookie c4("k", "v", "/downloads/", "localhost", false);
  CPPUNIT_ASSERT(c.match("www.aria2.org", "/downloads", 0, false));
  CPPUNIT_ASSERT(c.match("www.aria2.org", "/downloads/", 0, false));
  CPPUNIT_ASSERT(c2.match("www.aria2.org", "/downloads", 0, false));
  CPPUNIT_ASSERT(c.match("WWW.ARIA2.ORG", "/downloads", 0, false));
  CPPUNIT_ASSERT(!c.match("www.aria.org", "/downloads", 0, false));
  CPPUNIT_ASSERT(!c.match("www.aria2.org", "/examples", 0, false));
  CPPUNIT_ASSERT(c.match("www.aria2.org", "/downloads", 0, true));
  CPPUNIT_ASSERT(c.match("www.aria2.org", "/downloads/latest", 0, false));
  CPPUNIT_ASSERT(!c.match("www.aria2.org", "/downloadss/latest", 0, false));
  CPPUNIT_ASSERT(!c.match("www.aria2.org", "/DOWNLOADS", 0, false));
  CPPUNIT_ASSERT(c3.match("www.aria2.org", "/downloads", 0, false));
  CPPUNIT_ASSERT(c4.match("localhost", "/downloads", 0, false));

  Cookie secureCookie("k", "v", "/", "secure.aria2.org", true);
  CPPUNIT_ASSERT(secureCookie.match("secure.aria2.org", "/", 0, true));
  CPPUNIT_ASSERT(!secureCookie.match("secure.aria2.org", "/", 0, false));
  CPPUNIT_ASSERT(!secureCookie.match("ssecure.aria2.org", "/", 0, true));
  CPPUNIT_ASSERT(secureCookie.match("www.secure.aria2.org", "/", 0, true));

  Cookie expireTest("k", "v", 1000, "/", ".aria2.org", false);
  CPPUNIT_ASSERT(expireTest.match("www.aria2.org", "/", 999, false));
  CPPUNIT_ASSERT(!expireTest.match("www.aria2.org", "/", 1000, false));
  CPPUNIT_ASSERT(!expireTest.match("www.aria2.org", "/", 1001, false));
  
  Cookie fromNumericHost("k", "v", "/", "192.168.1.1", false);
  CPPUNIT_ASSERT(fromNumericHost.match("192.168.1.1", "/", 0, false));
  CPPUNIT_ASSERT(!fromNumericHost.match("www.aria2.org", "/", 0, false));
}

void CookieTest::testIsExpired()
{
  Cookie expiredCookie("k", "v", 1000, "/", "localhost", false);
  CPPUNIT_ASSERT(expiredCookie.isExpired());
  Cookie validCookie("k", "v", Time().getTime()+60, "/", "localhost", false);
  CPPUNIT_ASSERT(!validCookie.isExpired());
  Cookie sessionCookie("k", "v", "/", "localhost", false);
  CPPUNIT_ASSERT(!sessionCookie.isExpired());
  Cookie sessionCookie2("k", "v", 0, "/", "localhost", false);
  CPPUNIT_ASSERT(!sessionCookie2.isExpired());

}

void CookieTest::testNormalizeDomain()
{
  Cookie dot("k", "v", "/", ".", false);
  CPPUNIT_ASSERT_EQUAL(std::string("..local"), dot.getDomain());
  Cookie ip("k", "v", "/", "192.168.1.1", false);
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.1.1"), ip.getDomain());
}

void CookieTest::testToNsCookieFormat()
{
  CPPUNIT_ASSERT_EQUAL
    (std::string(".domain.org\tTRUE\t/\tFALSE\t12345678\thello\tworld"),
     Cookie("hello","world",12345678,"/",".domain.org",false).toNsCookieFormat());
  // Session cookie's expiry is 0
  CPPUNIT_ASSERT_EQUAL
    (std::string(".domain.org\tTRUE\t/\tTRUE\t0\thello\tworld"),
     Cookie("hello","world","/","domain.org",true).toNsCookieFormat());
}

void CookieTest::testMarkOriginServerOnly()
{
  Cookie c("k", "v", "/", "aria2.sf.net", false);
  c.markOriginServerOnly();
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sf.net"), c.getDomain());
  Cookie ip("k", "v", "/", "192.168.0.1", false);
  ip.markOriginServerOnly();
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), ip.getDomain());
}

} // namespace aria2
