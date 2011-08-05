#include "CookieStorage.h"

#include <iostream>
#include <algorithm>
#include <limits>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "TimeA2.h"
#include "RecoverableException.h"
#include "File.h"
#include "TestUtil.h"

namespace aria2 {

class CookieStorageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(CookieStorageTest);
  CPPUNIT_TEST(testStore);
  CPPUNIT_TEST(testParseAndStore);
  CPPUNIT_TEST(testCriteriaFind);
  CPPUNIT_TEST(testCriteriaFind_cookieOrder);
  CPPUNIT_TEST(testLoad);
  CPPUNIT_TEST(testLoad_sqlite3);
  CPPUNIT_TEST(testLoad_fileNotfound);
  CPPUNIT_TEST(testSaveNsFormat);
  CPPUNIT_TEST(testSaveNsFormat_fail);
  CPPUNIT_TEST(testCookieIsFull);
  CPPUNIT_TEST(testDomainIsFull);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void dumpCookie(std::vector<Cookie>& cookies, const CookieStorage& cs);

  void testStore();
  void testParseAndStore();
  void testCriteriaFind();
  void testCriteriaFind_cookieOrder();
  void testLoad();
  void testLoad_sqlite3();
  void testLoad_fileNotfound();
  void testSaveNsFormat();
  void testSaveNsFormat_fail();
  void testCookieIsFull();
  void testDomainIsFull();
};


CPPUNIT_TEST_SUITE_REGISTRATION(CookieStorageTest);

void CookieStorageTest::dumpCookie
(std::vector<Cookie>& cookies, const CookieStorage& st)
{
  st.dumpCookie(std::back_inserter(cookies));
  std::sort(cookies.begin(), cookies.end(), CookieSorter());
}

void CookieStorageTest::testStore()
{
  time_t now = 1000;
  CookieStorage st;
  Cookie goodCookie(createCookie("k", "v", "localhost", true, "/", false));
  CPPUNIT_ASSERT(st.store(goodCookie, now));
  CPPUNIT_ASSERT_EQUAL((size_t)1, st.size());
  CPPUNIT_ASSERT(st.contains(goodCookie));

  Cookie anotherCookie(createCookie("k", "v", "mirror",  true, "/", true));
  CPPUNIT_ASSERT(st.store(anotherCookie, now));
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  CPPUNIT_ASSERT(st.contains(anotherCookie));
  CPPUNIT_ASSERT(st.contains(goodCookie));

  Cookie updateGoodCookie(createCookie("k", "v2", "localhost",  true,
                                       "/", false));
  CPPUNIT_ASSERT(st.store(updateGoodCookie, now));
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  CPPUNIT_ASSERT(st.contains(updateGoodCookie));
  CPPUNIT_ASSERT(st.contains(anotherCookie));

  Cookie expireGoodCookie(createCookie("k", "v3", 0, "localhost", true,
                                       "/", false));
  CPPUNIT_ASSERT(!st.store(expireGoodCookie, now));
  CPPUNIT_ASSERT_EQUAL((size_t)1, st.size());
  CPPUNIT_ASSERT(st.contains(anotherCookie));

  Cookie fromNumericHost(createCookie("k", "v", "192.168.1.1", true,
                                      "/", false));
  CPPUNIT_ASSERT(st.store(fromNumericHost, now));
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  CPPUNIT_ASSERT(st.contains(fromNumericHost));
}

void CookieStorageTest::testParseAndStore()
{
  CookieStorage st;
  time_t now = 1000;
  std::string localhostCookieStr = "k=v;"
    " expires=Fri, 01 Jan 2038 00:00:00 GMT; path=/; domain=localhost;";
  CPPUNIT_ASSERT
    (st.parseAndStore(localhostCookieStr, "localhost", "/downloads", now));
  CPPUNIT_ASSERT
    (!st.parseAndStore(localhostCookieStr, "mirror", "/downloads", now));
  CPPUNIT_ASSERT
    (!st.parseAndStore(localhostCookieStr, "127.0.0.1", "/downloads", now));

  std::string numericHostCookieStr = "k=v;"
    " expires=Fri, 01 Jan 2038 00:00:00 GMT; path=/; domain=192.168.1.1;";
  CPPUNIT_ASSERT
    (st.parseAndStore(numericHostCookieStr, "192.168.1.1", "/", now));

  // No domain and no path are specified.
  std::string noDomainPathCookieStr = "k=v";
  CPPUNIT_ASSERT
    (st.parseAndStore(noDomainPathCookieStr,
                      "aria2.sf.net", "/downloads", now));
}

void CookieStorageTest::testCriteriaFind()
{
  CookieStorage st;
  time_t now = 1000;

  Cookie alpha(createCookie("alpha", "ALPHA", "aria2.org", false,  "/", false));
  Cookie bravo(createCookie("bravo", "BRAVO", 1060, "aria2.org", false,
                            "/foo", false));
  Cookie charlie(createCookie("charlie", "CHARLIE", "aria2.org", false,
                              "/", true));
  Cookie delta(createCookie("delta", "DELTA", "aria2.org", false,
                            "/foo/bar", false));
  Cookie echo(createCookie("echo", "ECHO", "www.dl.aria2.org", false,
                           "/", false));
  Cookie foxtrot(createCookie("foxtrot", "FOXTROT", "sf.net", false,
                              "/", false));
  Cookie golf(createCookie("golf", "GOLF", "192.168.1.1",  true,
                           "/", false));
  Cookie hotel1(createCookie("hotel", "HOTEL1", "samename.x", false,
                             "/", false));
  Cookie hotel2(createCookie("hotel", "HOTEL2", "samename.x", false,
                             "/hotel", false));
  Cookie hotel3(createCookie("hotel", "HOTEL3", "samename.x", false,
                             "/bar/wine", false));
  Cookie hotel4(createCookie("hotel", "HOTEL4", "samename.x", false,
                             "/bar/", false));
  Cookie india1(createCookie("india", "INDIA1", "default.domain",  true,
                             "/foo", false));
  Cookie india2(createCookie("india", "INDIA2", "default.domain", false,
                             "/",  false));
  Cookie juliet1(createCookie("juliet", "JULIET1", "localhost", true,
                              "/foo", false));

  CPPUNIT_ASSERT(st.store(alpha, now));
  CPPUNIT_ASSERT(st.store(bravo, now));
  CPPUNIT_ASSERT(st.store(charlie, now));
  CPPUNIT_ASSERT(st.store(delta, now));
  CPPUNIT_ASSERT(st.store(echo, now));
  CPPUNIT_ASSERT(st.store(foxtrot, now));
  CPPUNIT_ASSERT(st.store(golf, now));
  CPPUNIT_ASSERT(st.store(hotel1, now));
  CPPUNIT_ASSERT(st.store(hotel2, now));
  CPPUNIT_ASSERT(st.store(hotel3, now));
  CPPUNIT_ASSERT(st.store(hotel4, now));
  CPPUNIT_ASSERT(st.store(india1, now));
  CPPUNIT_ASSERT(st.store(india2, now));
  CPPUNIT_ASSERT(st.store(juliet1, now));

  std::vector<Cookie> aria2Slash = st.criteriaFind("www.dl.aria2.org", "/",
                                                   0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)2, aria2Slash.size());
  CPPUNIT_ASSERT(std::find(aria2Slash.begin(), aria2Slash.end(), alpha)
                 != aria2Slash.end());
  CPPUNIT_ASSERT(std::find(aria2Slash.begin(), aria2Slash.end(), echo)
                 != aria2Slash.end());

  std::vector<Cookie> aria2SlashFoo = st.criteriaFind("www.dl.aria2.org","/foo",
                                                      0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)3, aria2SlashFoo.size());
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), aria2SlashFoo[0].getName());
  CPPUNIT_ASSERT(std::find(aria2SlashFoo.begin(), aria2SlashFoo.end(), alpha)
                 != aria2SlashFoo.end());
  CPPUNIT_ASSERT(std::find(aria2SlashFoo.begin(), aria2SlashFoo.end(), echo)
                 != aria2SlashFoo.end());

  std::vector<Cookie> aria2Expires = st.criteriaFind("www.dl.aria2.org", "/foo",
                                                     1120,
                                                     false);
  CPPUNIT_ASSERT_EQUAL((size_t)2, aria2Expires.size());
  CPPUNIT_ASSERT(std::find(aria2Expires.begin(), aria2Expires.end(), alpha)
                 != aria2Expires.end());
  CPPUNIT_ASSERT(std::find(aria2Expires.begin(), aria2Expires.end(), echo)
                 != aria2Expires.end());

  std::vector<Cookie> dlAria2 = st.criteriaFind("dl.aria2.org", "/", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)1, dlAria2.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), dlAria2[0].getName());

  std::vector<Cookie> numericHostCookies = st.criteriaFind("192.168.1.1", "/",0,
                                                           false);
  CPPUNIT_ASSERT_EQUAL((size_t)1, numericHostCookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("golf"), numericHostCookies[0].getName());

  std::vector<Cookie> sameNameCookies =
    st.criteriaFind("samename.x", "/bar/wine", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)3, sameNameCookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("HOTEL3"), sameNameCookies[0].getValue());
  CPPUNIT_ASSERT_EQUAL(std::string("HOTEL4"), sameNameCookies[1].getValue());
  CPPUNIT_ASSERT_EQUAL(std::string("HOTEL1"), sameNameCookies[2].getValue());

  std::vector<Cookie> defaultDomainCookies =
    st.criteriaFind("default.domain", "/foo", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)2, defaultDomainCookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("INDIA1"),
                       defaultDomainCookies[0].getValue());
  CPPUNIT_ASSERT_EQUAL(std::string("INDIA2"),
                       defaultDomainCookies[1].getValue());
  defaultDomainCookies =
    st.criteriaFind("sub.default.domain", "/foo", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)1, defaultDomainCookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("INDIA2"),
                       defaultDomainCookies[0].getValue());

  // localhost.local case
  std::vector<Cookie> localDomainCookies =
    st.criteriaFind("localhost", "/foo", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)1, localDomainCookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("JULIET1"),
                       localDomainCookies[0].getValue());
}

void CookieStorageTest::testCriteriaFind_cookieOrder()
{
  CookieStorage st;
  Cookie a(createCookie("a", "0", "host", true, "/", false));
  a.setCreationTime(1000);
  Cookie b(createCookie("b", "0", "host", true, "/foo", false));
  b.setCreationTime(5000);
  Cookie c(createCookie("c", "0", "host", true, "/foo", false));
  c.setCreationTime(4000);
  Cookie d(createCookie("d", "0", "host", true, "/foo/bar", false));
  d.setCreationTime(6000);

  st.store(a, 0);
  st.store(b, 0);
  st.store(c, 0);
  st.store(d, 0);

  std::vector<Cookie> cookies = st.criteriaFind("host", "/foo/bar", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)4, cookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("d"), cookies[0].getName());
  CPPUNIT_ASSERT_EQUAL(std::string("c"), cookies[1].getName());
  CPPUNIT_ASSERT_EQUAL(std::string("b"), cookies[2].getName());
  CPPUNIT_ASSERT_EQUAL(std::string("a"), cookies[3].getName());
}

void CookieStorageTest::testLoad()
{
  CookieStorage st;

  st.load(A2_TEST_DIR"/nscookietest.txt", 1001);

  CPPUNIT_ASSERT_EQUAL((size_t)4, st.size());

  std::vector<Cookie> cookies;
  dumpCookie(cookies, st);

  Cookie c = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string("passwd"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("secret"), c.getValue());
  CPPUNIT_ASSERT_EQUAL(std::numeric_limits<time_t>::max(), c.getExpiryTime());
  CPPUNIT_ASSERT(!c.getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("/cgi-bin"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), c.getDomain());
  CPPUNIT_ASSERT(c.getHostOnly());
  CPPUNIT_ASSERT(!c.getSecure());

  c = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string("novalue"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string(""), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.getExpiryTime());
  CPPUNIT_ASSERT(c.getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("example.org"), c.getDomain());
  CPPUNIT_ASSERT(!c.getHostOnly());
  CPPUNIT_ASSERT(!c.getSecure());

  c = cookies[2];
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.getExpiryTime());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.getDomain());
  CPPUNIT_ASSERT(c.getHostOnly());
  CPPUNIT_ASSERT(c.getSecure());

  c = cookies[3];
  CPPUNIT_ASSERT_EQUAL(std::string("TAX"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("1000"), c.getValue());
  CPPUNIT_ASSERT((time_t)INT32_MAX <= c.getExpiryTime());
  CPPUNIT_ASSERT(c.getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("overflow"), c.getDomain());
  CPPUNIT_ASSERT(!c.getSecure());
}

void CookieStorageTest::testLoad_sqlite3()
{
  CookieStorage st;
#ifdef HAVE_SQLITE3
  st.load(A2_TEST_DIR"/cookies.sqlite", 1000);
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  std::vector<Cookie> cookies;
  dumpCookie(cookies, st);
  Cookie c = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)INT32_MAX, c.getExpiryTime());
  CPPUNIT_ASSERT(c.getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.getDomain());
  CPPUNIT_ASSERT(c.getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT(c.getSecure());

  c = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), c.getValue());
  CPPUNIT_ASSERT((time_t)INT32_MAX <= c.getExpiryTime());
  CPPUNIT_ASSERT(c.getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("overflow.time_t.org"), c.getDomain());
  CPPUNIT_ASSERT(!c.getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/path/to"), c.getPath());
  CPPUNIT_ASSERT(!c.getSecure());
    
#else // !HAVE_SQLITE3
  CPPUNIT_ASSERT(!st.load(A2_TEST_DIR"/cookies.sqlite", 1000));
#endif // !HAVE_SQLITE3
}

void CookieStorageTest::testLoad_fileNotfound()
{
  CookieStorage st;
  CPPUNIT_ASSERT(!st.load("./aria2_CookieStorageTest_testLoad_fileNotfound",0));
}

void CookieStorageTest::testSaveNsFormat()
{
  // TODO add cookie with default domain
  std::string filename = A2_TEST_OUT_DIR"/aria2_CookieStorageTest_testSaveNsFormat";
  File(filename).remove();
  CookieStorage st;
  time_t now = 1000;
  st.store(Cookie(createCookie("favorite", "classic", "domain.org", false,
                               "/config",true)), now);
  st.store(Cookie(createCookie("uid", "tujikawa", now, "domain.org", true,
                               "/",false)), now);
  CPPUNIT_ASSERT(st.saveNsFormat(filename));
  CookieStorage loadst;
  loadst.load(filename, now);
  CPPUNIT_ASSERT_EQUAL((size_t)2, loadst.size());

  std::vector<Cookie> cookies;
  dumpCookie(cookies, loadst);

  CPPUNIT_ASSERT_EQUAL(std::string("favorite"), cookies[0].getName());
  CPPUNIT_ASSERT_EQUAL(std::string("uid"), cookies[1].getName());
}

void CookieStorageTest::testSaveNsFormat_fail()
{
  std::string filename =
    A2_TEST_OUT_DIR"/aria2_CookieStorageTest_testSaveNsFormat_fail";
  File f(filename);
  f.remove();
  f.mkdirs();
  CPPUNIT_ASSERT(f.isDir());
  CookieStorage st;
  CPPUNIT_ASSERT(!st.saveNsFormat(filename));
}

void CookieStorageTest::testCookieIsFull()
{
  CookieStorage st;
  for(size_t i = 0; i < CookieStorage::MAX_COOKIE_PER_DOMAIN+1; ++i) {
    Cookie c(createCookie("k"+util::itos(i), "v", "aria2.org", false,
                          "/", false));
    st.store(c, 0);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)CookieStorage::MAX_COOKIE_PER_DOMAIN, st.size());
}

void CookieStorageTest::testDomainIsFull()
{
  // See DOMAIN_EVICTION_TRIGGER and DOMAIN_EVICTION_RATE in
  // CookieStorage.cc
  CookieStorage st;
  for(size_t i = 0; i < 2001; ++i) {
    Cookie c(createCookie("k", "v", "domain"+util::itos(i), true,
                          "/", false));
    st.store(c, 0);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)1801, st.size());
}

} // namespace aria2
