#include "UriListParser.h"
#include "Exception.h"
#include "Util.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class UriListParserTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UriListParserTest);
  CPPUNIT_TEST(testHasNext);
  CPPUNIT_TEST_SUITE_END();
private:
  std::string list2String(const std::deque<std::string>& src);
public:
  void setUp() {
  }

  void testHasNext();
};


CPPUNIT_TEST_SUITE_REGISTRATION( UriListParserTest );

std::string UriListParserTest::list2String(const std::deque<std::string>& src)
{
  std::ostringstream strm;
  std::copy(src.begin(), src.end(), std::ostream_iterator<std::string>(strm, " "));
  return Util::trim(strm.str());
}

void UriListParserTest::testHasNext()
{
  UriListParser flp;

  std::ifstream in("filelist1.txt");

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/index.html http://localhost2/index.html"), list2String(flp.parseNext(in)));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://localhost/aria2.tar.bz2"),
		       list2String(flp.parseNext(in)));
  CPPUNIT_ASSERT_EQUAL(std::string(""),
		       list2String(flp.parseNext(in)));
  CPPUNIT_ASSERT(!in);
}

} // namespace aria2
