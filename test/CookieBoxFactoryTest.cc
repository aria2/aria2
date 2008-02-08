#include "CookieBoxFactory.h"
#include "CookieBox.h"
#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class CookieBoxFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(CookieBoxFactoryTest);
  CPPUNIT_TEST(testLoadDefaultCookie);
  CPPUNIT_TEST(testCreateNewInstance);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testLoadDefaultCookie();
  void testCreateNewInstance();
};


CPPUNIT_TEST_SUITE_REGISTRATION( CookieBoxFactoryTest );

void CookieBoxFactoryTest::testLoadDefaultCookie()
{
  std::ifstream f("nscookietest.txt");

  CookieBoxFactory factory;

  factory.loadDefaultCookie(f);

  Cookies cookies = factory.getDefaultCookies();

  CPPUNIT_ASSERT_EQUAL((int32_t)4, (int32_t)cookies.size());

  Cookie c = cookies[0];
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)1181473200, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.domain);

  c = cookies[1];
  CPPUNIT_ASSERT_EQUAL(std::string("user"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string("me"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)1181473200, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.domain);

  c = cookies[2];
  CPPUNIT_ASSERT_EQUAL(std::string("passwd"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string("secret"), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/cgi-bin"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.domain);

  c = cookies[3];
  CPPUNIT_ASSERT_EQUAL(std::string("novalue"), c.name);
  CPPUNIT_ASSERT_EQUAL(std::string(""), c.value);
  CPPUNIT_ASSERT_EQUAL((time_t)2147483647, c.expires);
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c.path);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c.domain);
}

void CookieBoxFactoryTest::testCreateNewInstance()
{
  std::ifstream f("nscookietest.txt");
  CookieBoxFactory factory;
  factory.loadDefaultCookie(f);
  SharedHandle<CookieBox> box = factory.createNewInstance();
  std::deque<Cookie> cookies = box->criteriaFind("localhost", "/", 0, true);

  CPPUNIT_ASSERT_EQUAL((int32_t)3, (int32_t)cookies.size());
}

} // namespace aria2
