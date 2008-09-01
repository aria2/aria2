#include "CookieParser.h"
#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

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
  std::string str = "JSESSIONID=123456789; expires=Sun, 2007-06-10 11:00:00 GMT; path=/; domain=localhost; secure";
  Cookie c = CookieParser().parse(str);
  CPPUNIT_ASSERT(c.good());
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)1181473200, c.getExpiry());  
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.getDomain());
  CPPUNIT_ASSERT_EQUAL(true, c.isSecureCookie());
  CPPUNIT_ASSERT_EQUAL(false, c.isOnetimeCookie());

  std::string str2 = "JSESSIONID=123456789";
  c = CookieParser().parse(str2, "default.domain", "/default/path");
  CPPUNIT_ASSERT(c.good());
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c.getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)0, c.getExpiry());
  CPPUNIT_ASSERT_EQUAL(std::string("default.domain"), c.getDomain());
  CPPUNIT_ASSERT_EQUAL(std::string("/default/path"), c.getPath());
  CPPUNIT_ASSERT_EQUAL(false, c.isSecureCookie());
  CPPUNIT_ASSERT_EQUAL(true, c.isOnetimeCookie());

  std::string str3 = "";
  c = CookieParser().parse(str3);
  CPPUNIT_ASSERT(!c.good());

  std::string str4 = "UID=300; expires=Wed, 1890-01-01 0:0:0 GMT;";
  c = CookieParser().parse(str4, "localhost", "/");
  CPPUNIT_ASSERT(c.good());
  CPPUNIT_ASSERT(c.isOnetimeCookie());
}

void CookieParserTest::testParse_file()
{
  std::ifstream f("cookietest.txt");

  Cookies cookies = CookieParser().parse(f);

  CPPUNIT_ASSERT_EQUAL((int32_t)3, (int32_t)cookies.size());

  Cookie c = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c.getValue());

  c = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string("user"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("me"), c.getValue());

  c = cookies[2];
  CPPUNIT_ASSERT_EQUAL(std::string("passwd"), c.getName());
  CPPUNIT_ASSERT_EQUAL(std::string("secret"), c.getValue());
}

} // namespace aria2
