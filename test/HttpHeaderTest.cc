#include "HttpHeader.h"
#include "Range.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class HttpHeaderTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HttpHeaderTest);
  CPPUNIT_TEST(testGetRange);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testGetRange();

};


CPPUNIT_TEST_SUITE_REGISTRATION( HttpHeaderTest );

void HttpHeaderTest::testGetRange()
{
  {
    HttpHeader httpHeader;
    httpHeader.put("Content-Range", "bytes 1-499/1234");
    
    SharedHandle<Range> range = httpHeader.getRange();
    
    CPPUNIT_ASSERT_EQUAL((int64_t)1, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)499, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)1234, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("Content-Range",
		   "9223372036854775800-9223372036854775801/9223372036854775807");
    
    SharedHandle<Range> range = httpHeader.getRange();
    
    CPPUNIT_ASSERT_EQUAL(9223372036854775800LL, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL(9223372036854775801LL, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL(9223372036854775807LL, range->getEntityLength());
  }
}

} // namespace aria2
