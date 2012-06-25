#include "HttpHeader.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Range.h"
#include "DlAbortEx.h"

namespace aria2 {

class HttpHeaderTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HttpHeaderTest);
  CPPUNIT_TEST(testGetRange);
  CPPUNIT_TEST(testFindAll);
  CPPUNIT_TEST(testClearField);
  CPPUNIT_TEST(testFieldContains);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testGetRange();
  void testFindAll();
  void testClearField();
  void testFieldContains();
};

CPPUNIT_TEST_SUITE_REGISTRATION( HttpHeaderTest );

void HttpHeaderTest::testGetRange()
{
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range",
                   "9223372036854775800-9223372036854775801/9223372036854775807");
    
    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775800LL, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775801LL, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775807LL, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range",
                   "9223372036854775800-9223372036854775801/9223372036854775807");
     
    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775800LL, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775801LL, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775807LL, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range", "bytes */1024");

    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range", "bytes 0-9/*");

    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range", "bytes */*");

    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range", "bytes 0");

    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range", "bytes 0/");

    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range", "bytes 0-/3");
    try {
      httpHeader.getRange();
      CPPUNIT_FAIL("Exception must be thrown");
    } catch(const DlAbortEx& e) {
      // success
    }
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range", "bytes -0/3");
    try {
      httpHeader.getRange();
      CPPUNIT_FAIL("Exception must be thrown");
    } catch(const DlAbortEx& e) {
      // success
    }
  }
}

void HttpHeaderTest::testFindAll()
{
  HttpHeader h;
  h.put("A", "100");
  h.put("A", "101");
  h.put("B", "200");
  
  std::vector<std::string> r(h.findAll("A"));
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
  
  CPPUNIT_ASSERT_EQUAL(std::string("Bar"), h.find("Foo"));

  h.clearField();

  CPPUNIT_ASSERT_EQUAL(std::string(""), h.find("Foo"));
  CPPUNIT_ASSERT_EQUAL(200, h.getStatusCode());
  CPPUNIT_ASSERT_EQUAL(std::string(HttpHeader::HTTP_1_1), h.getVersion());
}

void HttpHeaderTest::testFieldContains()
{
  HttpHeader h;
  h.put("connection", "Keep-Alive, Upgrade");
  h.put("upgrade", "WebSocket");
  h.put("sec-websocket-version", "13");
  h.put("sec-websocket-version", "8, 7");
  CPPUNIT_ASSERT(h.fieldContains("connection", "upgrade"));
  CPPUNIT_ASSERT(h.fieldContains("connection", "keep-alive"));
  CPPUNIT_ASSERT(!h.fieldContains("connection", "close"));
  CPPUNIT_ASSERT(h.fieldContains("upgrade", "websocket"));
  CPPUNIT_ASSERT(!h.fieldContains("upgrade", "spdy"));
  CPPUNIT_ASSERT(h.fieldContains("sec-websocket-version", "13"));
  CPPUNIT_ASSERT(h.fieldContains("sec-websocket-version", "8"));
  CPPUNIT_ASSERT(!h.fieldContains("sec-websocket-version", "6"));
}

} // namespace aria2
