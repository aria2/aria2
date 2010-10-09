#include "Cookie.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "TimeA2.h"
#include "TestUtil.h"

namespace aria2 {

class CookieTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(CookieTest);
  CPPUNIT_TEST(testOperatorEqual);
  CPPUNIT_TEST(testMatch);
  CPPUNIT_TEST(testIsExpired);
  CPPUNIT_TEST(testToNsCookieFormat);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testOperatorEqual();
  void testMatch();
  void testIsExpired();
  void testToNsCookieFormat();
};


CPPUNIT_TEST_SUITE_REGISTRATION(CookieTest);

void CookieTest::testOperatorEqual()
{
  Cookie a(createCookie("k", "v", "localhost", true, "/", false));
  Cookie b(createCookie("k", "v", "localhost", true, "/", true));
  Cookie wrongPath(createCookie("k", "v", "localhost", true, "/a", false));
  Cookie wrongDomain(createCookie("k", "v", "mydomain", true, "/", false));
  Cookie wrongName(createCookie("h", "v", "localhost", true, "/a", false));
  Cookie caseSensitiveName(createCookie("K", "v", "localhost", true,
                                        "/a", false));
  CPPUNIT_ASSERT(a == b);
  CPPUNIT_ASSERT(!(a == wrongPath));
  CPPUNIT_ASSERT(!(a == wrongDomain));
  CPPUNIT_ASSERT(!(a == wrongName));
  CPPUNIT_ASSERT(!(a == caseSensitiveName));
}

void CookieTest::testMatch()
{
  Cookie c(createCookie("k", "v", "aria2.org", false, "/downloads", false));
  Cookie c2(createCookie("k", "v", "aria2.org", false, "/downloads", false));
  Cookie c3(createCookie("k", "v", "aria2.org", true, "/downloads", false));
  Cookie c4(createCookie("k", "v", "localhost", true, "/downloads", false));
  CPPUNIT_ASSERT(c.match("www.aria2.org", "/downloads", 0, false));
  CPPUNIT_ASSERT(c2.match("www.aria2.org", "/downloads", 0, false));
  CPPUNIT_ASSERT(!c.match("www.aria.org", "/downloads", 0, false));
  CPPUNIT_ASSERT(!c.match("www.aria2.org", "/examples", 0, false));
  CPPUNIT_ASSERT(c.match("www.aria2.org", "/downloads", 0, true));
  CPPUNIT_ASSERT(c.match("www.aria2.org", "/downloads/latest", 0, false));
  CPPUNIT_ASSERT(!c.match("www.aria2.org", "/downloadss/latest", 0, false));
  CPPUNIT_ASSERT(!c.match("www.aria2.org", "/DOWNLOADS", 0, false));
  CPPUNIT_ASSERT(!c3.match("www.aria2.org", "/downloads", 0, false));
  CPPUNIT_ASSERT(c4.match("localhost", "/downloads", 0, false));

  Cookie secureCookie(createCookie("k", "v", "secure.aria2.org", false,
                                   "/", true));
  CPPUNIT_ASSERT(secureCookie.match("secure.aria2.org", "/", 0, true));
  CPPUNIT_ASSERT(!secureCookie.match("secure.aria2.org", "/", 0, false));
  CPPUNIT_ASSERT(!secureCookie.match("ssecure.aria2.org", "/", 0, true));
  CPPUNIT_ASSERT(secureCookie.match("www.secure.aria2.org", "/", 0, true));

  Cookie expireTest(createCookie("k", "v", 1000, "aria2.org", false,
                                 "/", false));
  CPPUNIT_ASSERT(expireTest.match("www.aria2.org", "/", 999, false));
  CPPUNIT_ASSERT(expireTest.match("www.aria2.org", "/", 1000, false));
  CPPUNIT_ASSERT(!expireTest.match("www.aria2.org", "/", 1001, false));
  
  Cookie fromNumericHost(createCookie("k", "v", "192.168.1.1", true,
                                      "/foo", false));
  CPPUNIT_ASSERT(fromNumericHost.match("192.168.1.1", "/foo", 0, false));
  CPPUNIT_ASSERT(!fromNumericHost.match("www.aria2.org", "/foo", 0, false));
  CPPUNIT_ASSERT(!fromNumericHost.match("1.192.168.1.1", "/foo", 0, false));
  CPPUNIT_ASSERT(!fromNumericHost.match("192.168.1.1", "/", 0, false));
}

void CookieTest::testIsExpired()
{
  Cookie cookie(createCookie("k", "v", 1000, "localhost", true,
                                    "/", false));
  CPPUNIT_ASSERT(cookie.isExpired(1001));
  CPPUNIT_ASSERT(!cookie.isExpired(1000));
  CPPUNIT_ASSERT(!cookie.isExpired(999));
  Cookie sessionCookie(createCookie("k", "v", "localhost", true, "/", false));
  CPPUNIT_ASSERT(!sessionCookie.isExpired(INT32_MAX));
}

void CookieTest::testToNsCookieFormat()
{
  CPPUNIT_ASSERT_EQUAL
    (std::string(".domain.org\tTRUE\t/\tFALSE\t12345678\thello\tworld"),
     createCookie("hello", "world", 12345678, "domain.org", false, "/",false)
     .toNsCookieFormat());
  // Session cookie
  CPPUNIT_ASSERT_EQUAL
    (std::string("domain.org\tFALSE\t/\tTRUE\t0\thello\tworld"),
     createCookie("hello", "world", "domain.org", true, "/", true)
     .toNsCookieFormat());
}

} // namespace aria2
