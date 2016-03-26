#include "CookieBox.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class CookieBoxTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(CookieBoxTest);
  CPPUNIT_TEST(testCriteriaFind);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testCriteriaFind();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CookieBoxTest);

void CookieBoxTest::testCriteriaFind()
{
  // 1181473200 = Sun Jun 10 11:00:00 2007 GMT
  Cookie c1("SESSIONID1", "1", 1181473200, "/downloads", "rednoah.com", false);
  Cookie c2("SESSIONID2", "2", 1181473200, "/downloads", "rednoah.com", false);
  Cookie c3("USER", "user", "/home", "aria.rednoah.com", false);
  Cookie c4("PASS", "pass", "/downloads", "rednoah.com", true);

  CookieBox box;
  box.add(c1);
  box.add(c2);
  box.add(c3);
  box.add(c4);

  Cookies result1 =
      box.criteriaFind("rednoah.com", "/downloads", 1181473100, false);
  CPPUNIT_ASSERT_EQUAL(2, (int)result1.size());
  Cookies::iterator itr = result1.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("SESSIONID1=1"), (*itr).toString());
  itr++;
  CPPUNIT_ASSERT_EQUAL(std::string("SESSIONID2=2"), (*itr).toString());

  result1 = box.criteriaFind("rednoah.com", "/downloads", 1181473100, true);
  CPPUNIT_ASSERT_EQUAL(3, (int)result1.size());
  itr = result1.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("SESSIONID1=1"), (*itr).toString());
  itr++;
  CPPUNIT_ASSERT_EQUAL(std::string("SESSIONID2=2"), (*itr).toString());
  itr++;
  CPPUNIT_ASSERT_EQUAL(std::string("PASS=pass"), (*itr).toString());

  result1 = box.criteriaFind("aria.rednoah.com", "/", 1181473100, false);
  CPPUNIT_ASSERT_EQUAL(0, (int)result1.size());

  result1 = box.criteriaFind("aria.rednoah.com", "/home", 1181473100, false);
  CPPUNIT_ASSERT_EQUAL(1, (int)result1.size());
  itr = result1.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("USER=user"), (*itr).toString());

  result1 = box.criteriaFind("rednoah.com", "/downloads", 1181473200, false);
  CPPUNIT_ASSERT_EQUAL(0, (int)result1.size());
}

} // namespace aria2
