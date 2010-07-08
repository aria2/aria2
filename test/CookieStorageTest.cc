#include "CookieStorage.h"

#include <iostream>
#include <algorithm>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "TimeA2.h"
#include "CookieParser.h"
#include "RecoverableException.h"
#include "File.h"
#include "TestUtil.h"

namespace aria2 {

class CookieStorageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(CookieStorageTest);
  CPPUNIT_TEST(testStore);
  CPPUNIT_TEST(testParseAndStore);
  CPPUNIT_TEST(testCriteriaFind);
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
  CookieStorage st;
  Cookie goodCookie("k", "v", "/", "localhost", false);
  CPPUNIT_ASSERT(st.store(goodCookie));
  CPPUNIT_ASSERT_EQUAL((size_t)1, st.size());
  CPPUNIT_ASSERT(st.contains(goodCookie));

  Cookie anotherCookie("k", "v", "/", "mirror", true);
  CPPUNIT_ASSERT(st.store(anotherCookie));
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  CPPUNIT_ASSERT(st.contains(anotherCookie));
  CPPUNIT_ASSERT(st.contains(goodCookie));

  Cookie updateGoodCookie("k", "v2", "/", "localhost", false);
  CPPUNIT_ASSERT(st.store(goodCookie));
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  CPPUNIT_ASSERT(st.contains(updateGoodCookie));
  CPPUNIT_ASSERT(st.contains(anotherCookie));

  Cookie expireGoodCookie("k", "v3", 1, "/", "localhost", false);
  CPPUNIT_ASSERT(!st.store(expireGoodCookie));
  CPPUNIT_ASSERT_EQUAL((size_t)1, st.size());
  CPPUNIT_ASSERT(st.contains(anotherCookie));

  Cookie badCookie("", "", "/", "localhost", false);
  CPPUNIT_ASSERT(!st.store(badCookie));
  CPPUNIT_ASSERT_EQUAL((size_t)1, st.size());
  CPPUNIT_ASSERT(st.contains(anotherCookie));

  Cookie fromNumericHost("k", "v", "/", "192.168.1.1", false);
  CPPUNIT_ASSERT(st.store(fromNumericHost));
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  CPPUNIT_ASSERT(st.contains(fromNumericHost));

  Cookie sessionScopedGoodCookie("k", "v3", 0, "/", "localhost", false);
  CPPUNIT_ASSERT(st.store(sessionScopedGoodCookie));
  CPPUNIT_ASSERT_EQUAL((size_t)3, st.size());
  CPPUNIT_ASSERT(st.contains(sessionScopedGoodCookie));

  Cookie sessionScopedGoodCookie2("k2", "v3", "/", "localhost", false);
  CPPUNIT_ASSERT(st.store(sessionScopedGoodCookie2));
  CPPUNIT_ASSERT_EQUAL((size_t)4, st.size());
  CPPUNIT_ASSERT(st.contains(sessionScopedGoodCookie2));
}

void CookieStorageTest::testParseAndStore()
{
  CookieStorage st;

  std::string localhostCookieStr = "k=v;"
    " expires=Fri, 2038-01-01 00:00:00 GMT; path=/; domain=localhost;";
  
  CPPUNIT_ASSERT(st.parseAndStore(localhostCookieStr,
                                  "localhost", "/downloads"));
  CPPUNIT_ASSERT(!st.parseAndStore(localhostCookieStr,
                                   "mirror", "/downloads"));

  CPPUNIT_ASSERT(!st.parseAndStore(localhostCookieStr,
                                   "127.0.0.1", "/downloads"));

  std::string numericHostCookieStr = "k=v;"
    " expires=Fri, 2038-01-01 00:00:00 GMT; path=/; domain=192.168.1.1;";
  CPPUNIT_ASSERT(st.parseAndStore(numericHostCookieStr, "192.168.1.1", "/"));

  // No domain and no path are specified.
  std::string noDomainPathCookieStr = "k=v";
  CPPUNIT_ASSERT
    (st.parseAndStore(noDomainPathCookieStr, "aria2.sf.net", "/downloads"));
}

void CookieStorageTest::testCriteriaFind()
{
  CookieStorage st;

  Cookie alpha("alpha", "ALPHA", "/", ".aria2.org", false);
  Cookie bravo("bravo", "BRAVO", Time().getTime()+60, "/foo", ".aria2.org",
               false);
  Cookie charlie("charlie", "CHARLIE", "/", ".aria2.org", true);
  Cookie delta("delta", "DELTA", "/foo/bar", ".aria2.org", false);
  Cookie echo("echo", "ECHO", "/", "www.dl.aria2.org", false);
  Cookie foxtrot("foxtrot", "FOXTROT", "/", ".sf.net", false);
  Cookie golf("golf", "GOLF", "/", "192.168.1.1", false);
  Cookie hotel1("hotel", "HOTEL1", "/", "samename.x", false);
  Cookie hotel2("hotel", "HOTEL2", "/hotel", "samename.x", false);
  Cookie hotel3("hotel", "HOTEL3", "/bar/wine", "samename.x", false);
  Cookie hotel4("hotel", "HOTEL4", "/bar/", "samename.x", false);
  Cookie india1("india", "INDIA1", "/foo", "default.domain", false);
  india1.markOriginServerOnly();
  Cookie india2("india", "INDIA2", "/", "default.domain", false);
  Cookie juliet1("juliet", "JULIET1", "/foo", "localhost", false);
  juliet1.markOriginServerOnly();
  Cookie juliet2("juliet", "JULIET2", "/", "localhost", false);

  CPPUNIT_ASSERT(st.store(alpha));
  CPPUNIT_ASSERT(st.store(bravo));
  CPPUNIT_ASSERT(st.store(charlie));
  CPPUNIT_ASSERT(st.store(delta));
  CPPUNIT_ASSERT(st.store(echo));
  CPPUNIT_ASSERT(st.store(foxtrot));
  CPPUNIT_ASSERT(st.store(golf));
  CPPUNIT_ASSERT(st.store(hotel1));
  CPPUNIT_ASSERT(st.store(hotel2));
  CPPUNIT_ASSERT(st.store(hotel3));
  CPPUNIT_ASSERT(st.store(hotel4));
  CPPUNIT_ASSERT(st.store(india1));
  CPPUNIT_ASSERT(st.store(india2));
  CPPUNIT_ASSERT(st.store(juliet1));
  CPPUNIT_ASSERT(st.store(juliet2));

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
                                                     Time().getTime()+120,
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


  // localhost.local case
  std::vector<Cookie> localDomainCookies =
    st.criteriaFind("localhost", "/foo", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)2, localDomainCookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("JULIET1"),
                       localDomainCookies[0].getValue());
  CPPUNIT_ASSERT_EQUAL(std::string("JULIET2"),
                       localDomainCookies[1].getValue());
}

void CookieStorageTest::testLoad()
{
  CookieStorage st;

  st.load("nscookietest.txt");

  CPPUNIT_ASSERT_EQUAL((size_t)4, st.size());

  std::vector<Cookie> cookies;
  dumpCookie(cookies, st);

  Cookie c = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string("novalue"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string(""), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string(".example.org"), c.getDomain());
  CPPUNIT_ASSERT(!c.isSecureCookie());

  c = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost.local"), c.getDomain());
  CPPUNIT_ASSERT(c.isSecureCookie());

  c = cookies[2];
  CPPUNIT_ASSERT_EQUAL(std::string("passwd"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("secret"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("/cgi-bin"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost.local"), c.getDomain());
  CPPUNIT_ASSERT(!c.isSecureCookie());

  c = cookies[3];
  CPPUNIT_ASSERT_EQUAL(std::string("TAX"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("1000"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("overflow.local"), c.getDomain());
  CPPUNIT_ASSERT(!c.isSecureCookie());
}

void CookieStorageTest::testLoad_sqlite3()
{
  CookieStorage st;
#ifdef HAVE_SQLITE3
  st.load("cookies.sqlite");
  CPPUNIT_ASSERT_EQUAL((size_t)3, st.size());
  std::vector<Cookie> cookies;
  dumpCookie(cookies, st);
  Cookie c = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string("uid"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string(""), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)0, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("/path/to"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string(".null_value.com"), c.getDomain());
  CPPUNIT_ASSERT(!c.isSecureCookie());
  CPPUNIT_ASSERT(c.isSessionCookie());

  c = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("/path/to"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string(".overflow.time_t.org"), c.getDomain());
  CPPUNIT_ASSERT(!c.isSecureCookie());
  CPPUNIT_ASSERT(!c.isSessionCookie());

  c = cookies[2];
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost.local"), c.getDomain());
  CPPUNIT_ASSERT(c.isSecureCookie());
  CPPUNIT_ASSERT(!c.isSessionCookie());
    
#else // !HAVE_SQLITE3
  CPPUNIT_ASSERT(!st.load("cookies.sqlite"));
#endif // !HAVE_SQLITE3
}

void CookieStorageTest::testLoad_fileNotfound()
{
  CookieStorage st;
  CPPUNIT_ASSERT(!st.load("./aria2_CookieStorageTest_testLoad_fileNotfound"));
}

void CookieStorageTest::testSaveNsFormat()
{
  // TODO add cookie with default domain
  std::string filename = "./aria2_CookieStorageTest_testSaveNsFormat";
  File(filename).remove();
  CookieStorage st;
  st.store(Cookie("favorite","classic","/config",".domain.org",true));
  st.store(Cookie("uid","tujikawa","/",".domain.org",false));
  st.saveNsFormat(filename);
  CookieStorage loadst;
  loadst.load(filename);
  CPPUNIT_ASSERT_EQUAL((size_t)2, loadst.size());

  std::vector<Cookie> cookies;
  dumpCookie(cookies, loadst);

  CPPUNIT_ASSERT_EQUAL(std::string("favorite"), cookies[0].getName());
  CPPUNIT_ASSERT_EQUAL((time_t)0, cookies[0].getExpiry());
  CPPUNIT_ASSERT(cookies[0].isSessionCookie());
  CPPUNIT_ASSERT_EQUAL(std::string("uid"), cookies[1].getName());
}

void CookieStorageTest::testSaveNsFormat_fail()
{
  std::string filename = "./aria2_CookieStorageTest_testSaveNsFormat_fail";
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
    Cookie c("k"+util::itos(i), "v", "/", ".aria2.org", false);
    st.store(c);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)CookieStorage::MAX_COOKIE_PER_DOMAIN, st.size());
}

void CookieStorageTest::testDomainIsFull()
{
  // See DOMAIN_EVICTION_TRIGGER and DOMAIN_EVICTION_RATE in
  // CookieStorage.cc
  CookieStorage st;
  for(size_t i = 0; i < 2001; ++i) {
    Cookie c("k", "v", "/", "domain"+util::itos(i), false);
    st.store(c);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)1801, st.size());
}

} // namespace aria2
