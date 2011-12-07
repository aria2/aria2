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
  CPPUNIT_TEST(testFill);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testGetRange();
  void testFindAll();
  void testClearField();
  void testFill();
};


CPPUNIT_TEST_SUITE_REGISTRATION( HttpHeaderTest );

void HttpHeaderTest::testGetRange()
{
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range",
                   "9223372036854775800-9223372036854775801/9223372036854775807");
    
    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((off_t)9223372036854775800LL, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((off_t)9223372036854775801LL, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((off_t)9223372036854775807LL, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range",
                   "9223372036854775800-9223372036854775801/9223372036854775807");
     
    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((off_t)9223372036854775800LL, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((off_t)9223372036854775801LL, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((off_t)9223372036854775807LL, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range", "bytes */1024");

    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range", "bytes 0-9/*");

    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range", "bytes */*");

    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range", "bytes 0");

    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getEntityLength());
  }
  {
    HttpHeader httpHeader;
    httpHeader.put("content-range", "bytes 0/");

    SharedHandle<Range> range = httpHeader.getRange();

    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getStartByte());
    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getEndByte());
    CPPUNIT_ASSERT_EQUAL((off_t)0, range->getEntityLength());
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

void HttpHeaderTest::testFill()
{
  std::string s =
    "Host: aria2.sourceforge.net\r\n"
    "Connection: close \r\n" // trailing white space
    "Multi-Line: text1\r\n"
    "  text2\r\n"
    "  text3\r\n"
    "Duplicate: foo\r\n"
    "Duplicate: bar\r\n";
  HttpHeader h;
  h.fill(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sourceforge.net"),
                       h.find("host"));
  CPPUNIT_ASSERT_EQUAL(std::string("close"),
                       h.find("connection"));
  CPPUNIT_ASSERT_EQUAL(std::string("text1 text2 text3"),
                       h.find("multi-line"));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"),
                       h.findAll("duplicate")[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("bar"),
                       h.findAll("duplicate")[1]);
}

} // namespace aria2
