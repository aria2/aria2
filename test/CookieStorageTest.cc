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
#include "TimerA2.h"

namespace aria2 {

class CookieStorageTest : public CppUnit::TestFixture {

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
  CPPUNIT_TEST(testEviction);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

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
  void testEviction();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CookieStorageTest);

namespace {
std::vector<const Cookie*> dumpCookie(const CookieStorage& st)
{
  auto res = std::vector<const Cookie*>{};
  st.dumpCookie(std::back_inserter(res));
  std::sort(res.begin(), res.end(), CookieSorter());
  return res;
}
} // namespace

void CookieStorageTest::testStore()
{
  time_t now = 999;
  auto st = CookieStorage{};
  auto goodCookie = [&]() {
    auto c = createCookie("k", "v", "localhost", true, "/", false);
    c->setCreationTime(now);
    return c;
  };
  CPPUNIT_ASSERT(st.store(goodCookie(), now));
  CPPUNIT_ASSERT_EQUAL((size_t)1, st.size());
  CPPUNIT_ASSERT(st.contains(*goodCookie()));

  auto anotherCookie = []() {
    return createCookie("k", "v", "mirror", true, "/", true);
  };
  CPPUNIT_ASSERT(st.store(anotherCookie(), now));
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  CPPUNIT_ASSERT(st.contains(*anotherCookie()));
  CPPUNIT_ASSERT(st.contains(*goodCookie()));

  ++now;
  auto updateGoodCookie = [&]() {
    auto c = createCookie("k", "v2", "localhost", true, "/", false);
    c->setCreationTime(now);
    return c;
  };
  CPPUNIT_ASSERT(st.store(updateGoodCookie(), now));
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  CPPUNIT_ASSERT(st.contains(*updateGoodCookie()));
  CPPUNIT_ASSERT(st.contains(*anotherCookie()));
  auto cookies = st.criteriaFind("localhost", "/", now, false);
  CPPUNIT_ASSERT_EQUAL((size_t)1, cookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("v2"), cookies[0]->getValue());
  // New cookie's creation time must match old cookie's creation time.
  CPPUNIT_ASSERT_EQUAL((time_t)999, cookies[0]->getCreationTime());

  auto expireGoodCookie = []() {
    return createCookie("k", "v3", 0, "localhost", true, "/", false);
  };
  CPPUNIT_ASSERT(!st.store(expireGoodCookie(), now));
  CPPUNIT_ASSERT_EQUAL((size_t)1, st.size());
  CPPUNIT_ASSERT(st.contains(*anotherCookie()));

  auto fromNumericHost = []() {
    return createCookie("k", "v", "192.168.1.1", true, "/", false);
  };
  CPPUNIT_ASSERT(st.store(fromNumericHost(), now));
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  CPPUNIT_ASSERT(st.contains(*fromNumericHost()));
}

void CookieStorageTest::testParseAndStore()
{
  auto st = CookieStorage{};
  time_t now = 1000;
  std::string localhostCookieStr =
      "k=v;"
      " expires=Fri, 01 Jan 2038 00:00:00 GMT; path=/; domain=localhost;";
  CPPUNIT_ASSERT(
      st.parseAndStore(localhostCookieStr, "localhost", "/downloads", now));
  CPPUNIT_ASSERT(
      !st.parseAndStore(localhostCookieStr, "mirror", "/downloads", now));
  CPPUNIT_ASSERT(
      !st.parseAndStore(localhostCookieStr, "127.0.0.1", "/downloads", now));

  std::string numericHostCookieStr =
      "k=v;"
      " expires=Fri, 01 Jan 2038 00:00:00 GMT; path=/; domain=192.168.1.1;";
  CPPUNIT_ASSERT(
      st.parseAndStore(numericHostCookieStr, "192.168.1.1", "/", now));

  // No domain and no path are specified.
  std::string noDomainPathCookieStr = "k=v";
  CPPUNIT_ASSERT(st.parseAndStore(noDomainPathCookieStr, "aria2.sf.net",
                                  "/downloads", now));
}

void CookieStorageTest::testCriteriaFind()
{
  auto st = CookieStorage{};
  time_t now = 1000;

  auto alpha = []() {
    return createCookie("alpha", "ALPHA", "aria2.org", false, "/", false);
  };
  auto bravo = []() {
    return createCookie("bravo", "BRAVO", 1060, "aria2.org", false, "/foo",
                        false);
  };
  auto charlie = []() {
    return createCookie("charlie", "CHARLIE", "aria2.org", false, "/", true);
  };
  auto delta = []() {
    return createCookie("delta", "DELTA", "aria2.org", false, "/foo/bar",
                        false);
  };
  auto echo = []() {
    return createCookie("echo", "ECHO", "www.dl.aria2.org", false, "/", false);
  };
  auto foxtrot = []() {
    return createCookie("foxtrot", "FOXTROT", "sf.net", false, "/", false);
  };
  auto golf = []() {
    return createCookie("golf", "GOLF", "192.168.1.1", true, "/", false);
  };
  auto hotel1 = []() {
    return createCookie("hotel", "HOTEL1", "samename.x", false, "/", false);
  };
  auto hotel2 = []() {
    return createCookie("hotel", "HOTEL2", "samename.x", false, "/hotel",
                        false);
  };
  auto hotel3 = []() {
    return createCookie("hotel", "HOTEL3", "samename.x", false, "/bar/wine",
                        false);
  };
  auto hotel4 = []() {
    return createCookie("hotel", "HOTEL4", "samename.x", false, "/bar/", false);
  };
  auto india1 = []() {
    return createCookie("india", "INDIA1", "default.domain", true, "/foo",
                        false);
  };
  auto india2 = []() {
    return createCookie("india", "INDIA2", "default.domain", false, "/", false);
  };
  auto juliet1 = []() {
    return createCookie("juliet", "JULIET1", "localhost", true, "/foo", false);
  };

  CPPUNIT_ASSERT(st.store(alpha(), now));
  CPPUNIT_ASSERT(st.store(bravo(), now));
  CPPUNIT_ASSERT(st.store(charlie(), now));
  CPPUNIT_ASSERT(st.store(delta(), now));
  CPPUNIT_ASSERT(st.store(echo(), now));
  CPPUNIT_ASSERT(st.store(foxtrot(), now));
  CPPUNIT_ASSERT(st.store(golf(), now));
  CPPUNIT_ASSERT(st.store(hotel1(), now));
  CPPUNIT_ASSERT(st.store(hotel2(), now));
  CPPUNIT_ASSERT(st.store(hotel3(), now));
  CPPUNIT_ASSERT(st.store(hotel4(), now));
  CPPUNIT_ASSERT(st.store(india1(), now));
  CPPUNIT_ASSERT(st.store(india2(), now));
  CPPUNIT_ASSERT(st.store(juliet1(), now));

  auto aria2Slash = st.criteriaFind("www.dl.aria2.org", "/", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)2, aria2Slash.size());
  CPPUNIT_ASSERT(derefFind(aria2Slash, alpha()));
  CPPUNIT_ASSERT(derefFind(aria2Slash, echo()));

  auto aria2SlashFoo = st.criteriaFind("www.dl.aria2.org", "/foo", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)3, aria2SlashFoo.size());
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), aria2SlashFoo[0]->getName());
  CPPUNIT_ASSERT(derefFind(aria2SlashFoo, alpha()));
  CPPUNIT_ASSERT(derefFind(aria2SlashFoo, echo()));

  auto aria2Expires = st.criteriaFind("www.dl.aria2.org", "/foo", 1120, false);
  CPPUNIT_ASSERT_EQUAL((size_t)2, aria2Expires.size());
  CPPUNIT_ASSERT(derefFind(aria2Expires, alpha()));
  CPPUNIT_ASSERT(derefFind(aria2Expires, echo()));

  auto dlAria2 = st.criteriaFind("dl.aria2.org", "/", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)1, dlAria2.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), dlAria2[0]->getName());

  auto tailmatchAria2 = st.criteriaFind("myaria2.org", "/", 0, false);
  CPPUNIT_ASSERT(tailmatchAria2.empty());

  auto numericHostCookies = st.criteriaFind("192.168.1.1", "/", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)1, numericHostCookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("golf"), numericHostCookies[0]->getName());

  auto sameNameCookies = st.criteriaFind("samename.x", "/bar/wine", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)3, sameNameCookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("HOTEL3"), sameNameCookies[0]->getValue());
  CPPUNIT_ASSERT_EQUAL(std::string("HOTEL4"), sameNameCookies[1]->getValue());
  CPPUNIT_ASSERT_EQUAL(std::string("HOTEL1"), sameNameCookies[2]->getValue());

  auto defaultDomainCookies =
      st.criteriaFind("default.domain", "/foo", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)2, defaultDomainCookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("INDIA1"),
                       defaultDomainCookies[0]->getValue());
  CPPUNIT_ASSERT_EQUAL(std::string("INDIA2"),
                       defaultDomainCookies[1]->getValue());
  defaultDomainCookies =
      st.criteriaFind("sub.default.domain", "/foo", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)1, defaultDomainCookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("INDIA2"),
                       defaultDomainCookies[0]->getValue());

  // localhost.local case
  auto localDomainCookies = st.criteriaFind("localhost", "/foo", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)1, localDomainCookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("JULIET1"),
                       localDomainCookies[0]->getValue());
}

void CookieStorageTest::testCriteriaFind_cookieOrder()
{
  auto st = CookieStorage{};
  auto a = createCookie("a", "0", "host", true, "/", false);
  a->setCreationTime(1000);
  auto b = createCookie("b", "0", "host", true, "/foo", false);
  b->setCreationTime(5000);
  auto c = createCookie("c", "0", "host", true, "/foo", false);
  c->setCreationTime(4000);
  auto d = createCookie("d", "0", "host", true, "/foo/bar", false);
  d->setCreationTime(6000);

  st.store(std::move(a), 0);
  st.store(std::move(b), 0);
  st.store(std::move(c), 0);
  st.store(std::move(d), 0);

  auto cookies = st.criteriaFind("host", "/foo/bar", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)4, cookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("d"), cookies[0]->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("c"), cookies[1]->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("b"), cookies[2]->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("a"), cookies[3]->getName());
}

void CookieStorageTest::testLoad()
{
  auto st = CookieStorage{};

  st.load(A2_TEST_DIR "/nscookietest.txt", 1001);

  CPPUNIT_ASSERT_EQUAL((size_t)4, st.size());

  auto cookies = dumpCookie(st);

  auto c = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string("passwd"), c->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("secret"), c->getValue());
  CPPUNIT_ASSERT_EQUAL(std::numeric_limits<time_t>::max(), c->getExpiryTime());
  CPPUNIT_ASSERT(!c->getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("/cgi-bin"), c->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), c->getDomain());
  CPPUNIT_ASSERT(c->getHostOnly());
  CPPUNIT_ASSERT(!c->getSecure());

  c = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string("novalue"), c->getName());
  CPPUNIT_ASSERT_EQUAL(std::string(""), c->getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c->getExpiryTime());
  CPPUNIT_ASSERT(c->getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("example.org"), c->getDomain());
  CPPUNIT_ASSERT(!c->getHostOnly());
  CPPUNIT_ASSERT(!c->getSecure());

  c = cookies[2];
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c->getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c->getExpiryTime());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c->getDomain());
  CPPUNIT_ASSERT(c->getHostOnly());
  CPPUNIT_ASSERT(c->getSecure());

  c = cookies[3];
  CPPUNIT_ASSERT_EQUAL(std::string("TAX"), c->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("1000"), c->getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)1463304912, c->getExpiryTime());
  CPPUNIT_ASSERT(c->getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("something"), c->getDomain());
  CPPUNIT_ASSERT(!c->getSecure());
}

void CookieStorageTest::testLoad_sqlite3()
{
  auto st = CookieStorage{};
#ifdef HAVE_SQLITE3
  st.load(A2_TEST_DIR "/cookies.sqlite", 1000);
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  auto cookies = dumpCookie(st);
  auto c = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c->getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)INT32_MAX, c->getExpiryTime());
  CPPUNIT_ASSERT(c->getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c->getDomain());
  CPPUNIT_ASSERT(c->getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c->getPath());
  CPPUNIT_ASSERT(c->getSecure());

  c = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), c->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), c->getValue());
  CPPUNIT_ASSERT((time_t)INT32_MAX <= c->getExpiryTime());
  CPPUNIT_ASSERT(c->getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("overflow.time_t.org"), c->getDomain());
  CPPUNIT_ASSERT(!c->getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/path/to"), c->getPath());
  CPPUNIT_ASSERT(!c->getSecure());

#else  // !HAVE_SQLITE3
  CPPUNIT_ASSERT(!st.load(A2_TEST_DIR "/cookies.sqlite", 1000));
#endif // !HAVE_SQLITE3
}

void CookieStorageTest::testLoad_fileNotfound()
{
  auto st = CookieStorage{};
  CPPUNIT_ASSERT(
      !st.load("./aria2_CookieStorageTest_testLoad_fileNotfound", 0));
}

void CookieStorageTest::testSaveNsFormat()
{
  // TODO add cookie with default domain
  std::string filename =
      A2_TEST_OUT_DIR "/aria2_CookieStorageTest_testSaveNsFormat";
  File(filename).remove();
  auto st = CookieStorage{};
  time_t now = 1000;
  st.store(
      createCookie("favorite", "classic", "domain.org", false, "/config", true),
      now);
  st.store(createCookie("uid", "tujikawa", now, "domain.org", true, "/", false),
           now);
  CPPUNIT_ASSERT(st.saveNsFormat(filename));
  auto loadst = CookieStorage{};
  loadst.load(filename, now);
  CPPUNIT_ASSERT_EQUAL((size_t)2, loadst.size());

  auto cookies = dumpCookie(loadst);

  CPPUNIT_ASSERT_EQUAL((size_t)2, cookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("favorite"), cookies[0]->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("uid"), cookies[1]->getName());
}

void CookieStorageTest::testSaveNsFormat_fail()
{
  std::string filename =
      A2_TEST_OUT_DIR "/aria2_CookieStorageTest_testSaveNsFormat_fail";
  File f(filename);
  f.remove();
  f.mkdirs();
  CPPUNIT_ASSERT(f.isDir());
  auto st = CookieStorage{};
  CPPUNIT_ASSERT(!st.saveNsFormat(filename));
}

void CookieStorageTest::testCookieIsFull()
{
  auto st = CookieStorage{};
  for (size_t i = 0; i < CookieStorage::MAX_COOKIE_PER_DOMAIN + 1; ++i) {
    st.store(
        createCookie("k" + util::itos(i), "v", "aria2.org", false, "/", false),
        0);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)CookieStorage::MAX_COOKIE_PER_DOMAIN, st.size());
}

void CookieStorageTest::testDomainIsFull()
{
  // See DOMAIN_EVICTION_TRIGGER and DOMAIN_EVICTION_RATE in
  // CookieStorage.cc
  auto st = CookieStorage{};
  for (size_t i = 0; i < 2001; ++i) {
    st.store(createCookie("k", "v", "domain" + util::itos(i), true, "/", false),
             0);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)1801, st.size());
}

void CookieStorageTest::testEviction()
{
  auto st = CookieStorage{};
  auto alpha = []() {
    return createCookie("a", "alpha", "aria2.sf.net", false, "/", false);
  };
  auto bravo = []() {
    return createCookie("b", "bravo", "d.aria2.sf.net", false, "/", false);
  };
  auto charlie = []() {
    return createCookie("c", "charlie", "a2.github.com", false, "/", false);
  };
  auto delta = []() {
    return createCookie("d", "delta", "aria2.sf.net", false, "/", false);
  };
  st.store(alpha(), 0);
  CPPUNIT_ASSERT_EQUAL((size_t)1, st.getLruTrackerSize());
  st.store(bravo(), 1);
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.getLruTrackerSize());
  st.store(charlie(), 2);
  CPPUNIT_ASSERT_EQUAL((size_t)3, st.getLruTrackerSize());
  st.store(delta(), 0);
  CPPUNIT_ASSERT_EQUAL((size_t)3, st.getLruTrackerSize());

  // aria2.sf.net will be evicted
  st.evictNode(1);
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.getLruTrackerSize());
  CPPUNIT_ASSERT(!st.contains(*alpha()));
  CPPUNIT_ASSERT(st.contains(*bravo()));
  CPPUNIT_ASSERT(st.contains(*charlie()));
  CPPUNIT_ASSERT(!st.contains(*delta()));

  // d.aria2.sf.net will be evicted
  st.evictNode(1);
  CPPUNIT_ASSERT_EQUAL((size_t)1, st.getLruTrackerSize());
  CPPUNIT_ASSERT(!st.contains(*bravo()));
  CPPUNIT_ASSERT(st.contains(*charlie()));

  // a2.github.com will be evicted
  st.evictNode(1);
  CPPUNIT_ASSERT_EQUAL((size_t)0, st.getLruTrackerSize());
  CPPUNIT_ASSERT(!st.contains(*charlie()));
  CPPUNIT_ASSERT_EQUAL((size_t)0, st.size());
  CPPUNIT_ASSERT(!st.getRootNode()->hasNext());
}

} // namespace aria2
