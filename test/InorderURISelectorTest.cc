#include "InorderURISelector.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "array_fun.h"
#include "FileEntry.h"

namespace aria2 {

class InorderURISelectorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(InorderURISelectorTest);
  CPPUNIT_TEST(testSelect);
  CPPUNIT_TEST_SUITE_END();
private:
  FileEntry fileEntry_;

  SharedHandle<InorderURISelector> sel;
  
public:
  void setUp()
  {
    static const char* urisSrc[] = {
      "http://alpha/file",
      "ftp://alpha/file",
      "http://bravo/file"
    };
    std::vector<std::string> uris;
    uris.assign(vbegin(urisSrc), vend(urisSrc));
    
    fileEntry_.setUris(uris);

    sel.reset(new InorderURISelector());
  }

  void tearDown() {}

  void testSelect();
};


CPPUNIT_TEST_SUITE_REGISTRATION(InorderURISelectorTest);

void InorderURISelectorTest::testSelect()
{
  std::vector<std::pair<size_t, std::string> > usedHosts;
  CPPUNIT_ASSERT_EQUAL(std::string("http://alpha/file"),
                       sel->select(&fileEntry_, usedHosts));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://alpha/file"),
                       sel->select(&fileEntry_, usedHosts));
  CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"),
                       sel->select(&fileEntry_, usedHosts));
  CPPUNIT_ASSERT_EQUAL(std::string(""), sel->select(&fileEntry_, usedHosts));
}

} // namespace aria2
