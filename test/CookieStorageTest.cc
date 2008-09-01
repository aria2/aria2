#include "CookieStorage.h"
#include "Exception.h"
#include "Util.h"
#include "TimeA2.h"
#include "CookieParser.h"
#include "RecoverableException.h"
#include <iostream>
#include <algorithm>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class CookieStorageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(CookieStorageTest);
  CPPUNIT_TEST(testStore);
  CPPUNIT_TEST(testParseAndStore);
  CPPUNIT_TEST(testCriteriaFind);
  CPPUNIT_TEST(testLoad);
  CPPUNIT_TEST(testLoad_sqlite3);
  CPPUNIT_TEST(testLoad_fileNotfound);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testStore();
  void testParseAndStore();
  void testCriteriaFind();
  void testLoad();
  void testLoad_sqlite3();
  void testLoad_fileNotfound();
};


CPPUNIT_TEST_SUITE_REGISTRATION(CookieStorageTest);

void CookieStorageTest::testStore()
{
  CookieStorage st;
  Cookie goodCookie("k", "v", "/", "localhost", false);
  CPPUNIT_ASSERT(st.store(goodCookie));
  CPPUNIT_ASSERT_EQUAL((size_t)1, st.size());
  CPPUNIT_ASSERT(std::find(st.begin(), st.end(), goodCookie) != st.end());

  Cookie anotherCookie("k", "v", "/", "mirror", true);
  CPPUNIT_ASSERT(st.store(anotherCookie));
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  CPPUNIT_ASSERT(std::find(st.begin(), st.end(), anotherCookie) != st.end());
  CPPUNIT_ASSERT(std::find(st.begin(), st.end(), goodCookie) != st.end());

  Cookie updateGoodCookie("k", "v2", "/", "localhost", false);
  CPPUNIT_ASSERT(st.store(goodCookie));
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  CPPUNIT_ASSERT(std::find(st.begin(), st.end(), updateGoodCookie) != st.end());
  CPPUNIT_ASSERT(std::find(st.begin(), st.end(), anotherCookie) != st.end());  

  Cookie expireGoodCookie("k", "v3", 0, "/", "localhost", false);
  CPPUNIT_ASSERT(!st.store(expireGoodCookie));
  CPPUNIT_ASSERT_EQUAL((size_t)1, st.size());
  CPPUNIT_ASSERT(std::find(st.begin(), st.end(), anotherCookie) != st.end());

  Cookie badCookie("", "", "/", "localhost", false);
  CPPUNIT_ASSERT(!st.store(badCookie));
  CPPUNIT_ASSERT_EQUAL((size_t)1, st.size());
  CPPUNIT_ASSERT(std::find(st.begin(), st.end(), anotherCookie) != st.end());

  Cookie fromNumericHost("k", "v", "/", "192.168.1.1", false);
  CPPUNIT_ASSERT(st.store(fromNumericHost));
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  CPPUNIT_ASSERT(std::find(st.begin(), st.end(), fromNumericHost) != st.end());
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

}

void CookieStorageTest::testCriteriaFind()
{
  CookieStorage st;

  Cookie alpha("alpha", "ALPHA", "/", ".aria2.org", false);
  Cookie bravo("bravo", "BRAVO", Time().getTime()+60, "/foo", ".aria2.org",
	       false);
  Cookie charlie("charlie", "CHARLIE", "/", ".aria2.org", true);
  Cookie delta("delta", "DELTA", "/foo/bar", ".aria2.org", false);
  Cookie echo("echo", "ECHO", "/", "www.aria2.org", false);
  Cookie foxtrot("foxtrot", "FOXTROT", "/", ".sf.net", false);
  Cookie golf("golf", "GOLF", "/", "192.168.1.1", false);

  CPPUNIT_ASSERT(st.store(alpha));
  CPPUNIT_ASSERT(st.store(bravo));
  CPPUNIT_ASSERT(st.store(charlie));
  CPPUNIT_ASSERT(st.store(delta));
  CPPUNIT_ASSERT(st.store(echo));
  CPPUNIT_ASSERT(st.store(foxtrot));
  CPPUNIT_ASSERT(st.store(golf));
  
  std::deque<Cookie> aria2Slash = st.criteriaFind("www.aria2.org", "/",
						  0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)2, aria2Slash.size());
  CPPUNIT_ASSERT(std::find(aria2Slash.begin(), aria2Slash.end(), alpha)
		 != aria2Slash.end());
  CPPUNIT_ASSERT(std::find(aria2Slash.begin(), aria2Slash.end(), echo)
		 != aria2Slash.end());

  std::deque<Cookie> aria2SlashFoo = st.criteriaFind("www.aria2.org", "/foo",
						     0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)3, aria2SlashFoo.size());
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), aria2SlashFoo[0].name);
  CPPUNIT_ASSERT(std::find(aria2SlashFoo.begin(), aria2SlashFoo.end(), alpha)
		 != aria2SlashFoo.end());
  CPPUNIT_ASSERT(std::find(aria2SlashFoo.begin(), aria2SlashFoo.end(), echo)
		 != aria2SlashFoo.end());

  std::deque<Cookie> aria2Expires = st.criteriaFind("www.aria2.org", "/foo",
						    Time().getTime()+120,
						    false);
  CPPUNIT_ASSERT_EQUAL((size_t)2, aria2Expires.size());
  CPPUNIT_ASSERT(std::find(aria2Expires.begin(), aria2Expires.end(), alpha)
		 != aria2Expires.end());
  CPPUNIT_ASSERT(std::find(aria2Expires.begin(), aria2Expires.end(), echo)
		 != aria2Expires.end());

  std::deque<Cookie> dlAria2 = st.criteriaFind("dl.aria2.org", "/", 0, false);
  CPPUNIT_ASSERT_EQUAL((size_t)1, dlAria2.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), dlAria2[0].name);

  std::deque<Cookie> numericHostCookies = st.criteriaFind("192.168.1.1", "/", 0,
							  false);
  CPPUNIT_ASSERT_EQUAL((size_t)1, numericHostCookies.size());
  CPPUNIT_ASSERT_EQUAL(std::string("golf"), numericHostCookies[0].name);
}

void CookieStorageTest::testLoad()
{
  CookieStorage st;

  st.load("nscookietest.txt");

  CPPUNIT_ASSERT_EQUAL((size_t)4, st.size());

  Cookie c = *st.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.domain);
  CPPUNIT_ASSERT(c.secure);

  c = *(st.begin()+1);
  CPPUNIT_ASSERT_EQUAL(std::string("passwd"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string("secret"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/cgi-bin"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.domain);
  CPPUNIT_ASSERT(!c.secure);

  c = *(st.begin()+2);
  CPPUNIT_ASSERT_EQUAL(std::string("TAX"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string("1000"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("overflow"), c.domain);
  CPPUNIT_ASSERT(!c.secure);

  c = *(st.begin()+3);
  CPPUNIT_ASSERT_EQUAL(std::string("novalue"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string(""), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.domain);
  CPPUNIT_ASSERT(!c.secure);
}

void CookieStorageTest::testLoad_sqlite3()
{
  CookieStorage st;
#ifdef HAVE_SQLITE3
  st.load("cookies.sqlite");
  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());
  Cookie c = *st.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.domain);
  CPPUNIT_ASSERT(c.secure);

  c = *(st.begin()+1);
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/path/to"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("overflow_time_t"), c.domain);
  CPPUNIT_ASSERT(!c.secure);
    
#else // !HAVE_SQLITE3
  try {
    st.load("cookies.sqlite");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // success
  }
#endif // !HAVE_SQLITE3
}

void CookieStorageTest::testLoad_fileNotfound()
{
  CookieStorage st;
  try {
    st.load("/tmp/aria2_CookieStorageTest_testLoad_fileNotfound");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // success
  }
}

} // namespace aria2
