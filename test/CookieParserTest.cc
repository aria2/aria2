#include "CookieParser.h"
#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class CookieParserTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(CookieParserTest);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST(testParse_file);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testParse();
  
  void testParse_file();
};


CPPUNIT_TEST_SUITE_REGISTRATION( CookieParserTest );

void CookieParserTest::testParse()
{
  string str = "JSESSIONID=123456789; expires=Sun, 2007-06-10 11:00:00 GMT; path=/; domain=localhost; secure";
  Cookie c = CookieParser().parse(str);
  CPPUNIT_ASSERT(c.good());
  CPPUNIT_ASSERT_EQUAL(string("JSESSIONID"), c.name);
  CPPUNIT_ASSERT_EQUAL(string("123456789"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)1181473200, c.expires);  
  CPPUNIT_ASSERT_EQUAL(string("/"), c.path);
  CPPUNIT_ASSERT_EQUAL(string("localhost"), c.domain);
  CPPUNIT_ASSERT_EQUAL(true, c.secure);
  CPPUNIT_ASSERT_EQUAL(false, c.onetime);

  string str2 = "JSESSIONID=123456789";
  c = CookieParser().parse(str2);
  CPPUNIT_ASSERT(c.good());
  CPPUNIT_ASSERT_EQUAL(string("JSESSIONID"), c.name);
  CPPUNIT_ASSERT_EQUAL(string("123456789"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)0, c.expires);
  CPPUNIT_ASSERT_EQUAL(string(""), c.path);
  CPPUNIT_ASSERT_EQUAL(string(""), c.domain);
  CPPUNIT_ASSERT_EQUAL(false, c.secure);
  CPPUNIT_ASSERT_EQUAL(true, c.onetime);

  string str3 = "";
  c = CookieParser().parse(str3);
  CPPUNIT_ASSERT(!c.good());
}

void CookieParserTest::testParse_file()
{
  ifstream f("cookietest.txt");

  Cookies cookies = CookieParser().parse(f);

  CPPUNIT_ASSERT_EQUAL(3, (int32_t)cookies.size());

  Cookie c = cookies[0];
  CPPUNIT_ASSERT_EQUAL(string("JSESSIONID"), c.name);
  CPPUNIT_ASSERT_EQUAL(string("123456789"), c.value);

  c = cookies[1];
  CPPUNIT_ASSERT_EQUAL(string("user"), c.name);
  CPPUNIT_ASSERT_EQUAL(string("me"), c.value);

  c = cookies[2];
  CPPUNIT_ASSERT_EQUAL(string("passwd"), c.name);
  CPPUNIT_ASSERT_EQUAL(string("secret"), c.value);
}
