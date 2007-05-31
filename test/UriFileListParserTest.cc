#include "UriFileListParser.h"
#include "Exception.h"
#include "Util.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class UriFileListParserTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UriFileListParserTest);
  CPPUNIT_TEST(testHasNext);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testHasNext();
};


CPPUNIT_TEST_SUITE_REGISTRATION( UriFileListParserTest );

string list2String(const Strings& src)
{
  ostringstream strm;
  copy(src.begin(), src.end(), ostream_iterator<string>(strm, " "));
  return Util::trim(strm.str());
}

void UriFileListParserTest::testHasNext()
{
  UriFileListParser flp("filelist1.txt");

  CPPUNIT_ASSERT(flp.hasNext());
  CPPUNIT_ASSERT_EQUAL(string("http://localhost/index.html http://localhost2/index.html"), list2String(flp.next()));
  CPPUNIT_ASSERT(flp.hasNext());
  CPPUNIT_ASSERT_EQUAL(string("ftp://localhost/aria2.tar.bz2"),
		       list2String(flp.next()));
  CPPUNIT_ASSERT(flp.hasNext());
  CPPUNIT_ASSERT_EQUAL(string(""),
		       list2String(flp.next()));
  CPPUNIT_ASSERT(!flp.hasNext());
}
