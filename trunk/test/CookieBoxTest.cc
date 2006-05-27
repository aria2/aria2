#include "CookieBox.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class CookieBoxTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(CookieBoxTest);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST(testCriteriaFind);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testParse();
  void testCriteriaFind();
};


CPPUNIT_TEST_SUITE_REGISTRATION( CookieBoxTest );

void CookieBoxTest::testParse() {
  CookieBox box;
  string str = "JSESSIONID=123456789; expires=Sat, 16-02-2006 19:38:00 JST; path=/; domain=localhost; secure";
  Cookie c;
  box.parse(c, str);
  CPPUNIT_ASSERT_EQUAL(string("JSESSIONID"), c.name);
  CPPUNIT_ASSERT_EQUAL(string("123456789"), c.value);
  CPPUNIT_ASSERT_EQUAL(string("Sat, 16-02-2006 19:38:00 JST"), c.expires);
  
  CPPUNIT_ASSERT_EQUAL(string("/"), c.path);
  
  CPPUNIT_ASSERT_EQUAL(string("localhost"), c.domain);
  CPPUNIT_ASSERT_EQUAL(true, c.secure);

  string str2 = "JSESSIONID=123456789";
  box.parse(c, str2);
  CPPUNIT_ASSERT_EQUAL(string("JSESSIONID"), c.name);
  CPPUNIT_ASSERT_EQUAL(string("123456789"), c.value);
  CPPUNIT_ASSERT_EQUAL(string(""), c.expires);
  
  CPPUNIT_ASSERT_EQUAL(string(""), c.path);
  
  CPPUNIT_ASSERT_EQUAL(string(""), c.domain);
  CPPUNIT_ASSERT_EQUAL(false, c.secure);  
}

void CookieBoxTest::testCriteriaFind() {
  Cookie c1("SESSIONID1", "1", "", "/downloads", "rednoah.com", false);
  Cookie c2("SESSIONID2", "2", "", "/downloads", "rednoah.com", false);
  Cookie c3("USER", "user", "", "/home", "aria.rednoah.com", false);
  Cookie c4("PASS", "pass", "", "/downloads", "rednoah.com", true);

  CookieBox box;
  box.add(c1);
  box.add(c2);
  box.add(c3);
  box.add(c4);

  Cookies result1 = box.criteriaFind("rednoah.com", "/downloads", false);
  CPPUNIT_ASSERT_EQUAL(2, (int)result1.size());
  Cookies::iterator itr = result1.begin();
  CPPUNIT_ASSERT_EQUAL(string("SESSIONID1=1"), (*itr).toString());
  itr++;
  CPPUNIT_ASSERT_EQUAL(string("SESSIONID2=2"), (*itr).toString());

  result1 = box.criteriaFind("rednoah.com", "/downloads", true);
  CPPUNIT_ASSERT_EQUAL(3, (int)result1.size());
  itr = result1.begin();
  CPPUNIT_ASSERT_EQUAL(string("SESSIONID1=1"), (*itr).toString());
  itr++;
  CPPUNIT_ASSERT_EQUAL(string("SESSIONID2=2"), (*itr).toString());
  itr++;
  CPPUNIT_ASSERT_EQUAL(string("PASS=pass"), (*itr).toString());

  result1 = box.criteriaFind("aria.rednoah.com", "/", false);
  CPPUNIT_ASSERT_EQUAL(0, (int)result1.size());

  result1 = box.criteriaFind("aria.rednoah.com", "/home", false);
  CPPUNIT_ASSERT_EQUAL(1, (int)result1.size());
  itr = result1.begin();
  CPPUNIT_ASSERT_EQUAL(string("USER=user"), (*itr).toString());
  

}

