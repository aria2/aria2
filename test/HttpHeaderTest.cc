#include "HttpHeader.h"
#include "Range.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class HttpHeaderTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HttpHeaderTest);
  CPPUNIT_TEST(testGetRange);
  CPPUNIT_TEST(testGet);
  CPPUNIT_TEST(testClearField);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testGetRange();
  void testGet();
  void testClearField();
};


CPPUNIT_TEST_SUITE_REGISTRATION( HttpHeaderTest );

void HttpHeaderTest::testGetRange()
{
  {
    HttpHeader httpHeader;
    httpHeader.put("Content-Range",
                   "9223372036854775800-9223372036854775801/9223372036854775807");
    
    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((off_t)9223372036854775800LL, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((off_t)9223372036854775801LL, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((uint64_t)9223372036854775807ULL, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("Content-Range",
                   "9223372036854775800-9223372036854775801/9223372036854775807");
     
    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((off_t)9223372036854775800LL, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((off_t)9223372036854775801LL, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((uint64_t)9223372036854775807ULL, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("Content-Range", "bytes */1024");

    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((uint64_t)0, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("Content-Range", "bytes 0-9/*");

    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((uint64_t)0, range->getEntityLength());
  }
}

void HttpHeaderTest::testGet()
{
  HttpHeader h;
  h.put("A", "100");
  h.put("a", "101");
  h.put("B", "200");
  
  std::vector<std::string> r(h.get("A"));
  CPPUNIT_ASSERT_EQUAL((size_t)2, r.size());
  CPPUNIT_ASSERT_EQUAL(std::string("100"), r[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("101"), r[1]);
}

void HttpHeaderTest::testClearField()
{
  HttpHeader h;
  h.setStatusCode(200);
  h.setVersion(HttpHeader::HTTP_1_1);
  h.put("Foo", "Bar");
  
  CPPUNIT_ASSERT_EQUAL(std::string("Bar"), h.getFirst("Foo"));

  h.clearField();

  CPPUNIT_ASSERT_EQUAL(std::string(""), h.getFirst("Foo"));
  CPPUNIT_ASSERT_EQUAL(200, h.getStatusCode());
  CPPUNIT_ASSERT_EQUAL(std::string(HttpHeader::HTTP_1_1), h.getVersion());
}

} // namespace aria2
