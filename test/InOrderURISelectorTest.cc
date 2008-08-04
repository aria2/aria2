#include "InOrderURISelector.h"
#include "Exception.h"
#include "Util.h"
#include "array_fun.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class InOrderURISelectorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(InOrderURISelectorTest);
  CPPUNIT_TEST(testSelect);
  CPPUNIT_TEST_SUITE_END();
private:

  std::deque<std::string> uris;

  SharedHandle<InOrderURISelector> sel;
  
public:
  void setUp()
  {
    static const char* urisSrc[] = {
      "http://alpha/file",
      "ftp://alpha/file",
      "http://bravo/file"
    };
    uris.assign(&urisSrc[0], &urisSrc[arrayLength(urisSrc)]);
    
    sel.reset(new InOrderURISelector());
  }

  void tearDown() {}

  void testSelect();
};


CPPUNIT_TEST_SUITE_REGISTRATION(InOrderURISelectorTest);

void InOrderURISelectorTest::testSelect()
{
  CPPUNIT_ASSERT_EQUAL(std::string("http://alpha/file"), sel->select(uris));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://alpha/file"), sel->select(uris));
  CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"), sel->select(uris));
  CPPUNIT_ASSERT_EQUAL(std::string(""), sel->select(uris));
}

} // namespace aria2
