#include "HttpHeaderProcessor.h"
#include "HttpHeader.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class HttpHeaderProcessorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HttpHeaderProcessorTest);
  CPPUNIT_TEST(testUpdate1);
  CPPUNIT_TEST(testUpdate2);
  CPPUNIT_TEST(testGetPutBackDataLength);
  CPPUNIT_TEST(testGetPutBackDataLength_nullChar);
  CPPUNIT_TEST(testGetHttpStatusHeader);
  CPPUNIT_TEST(testGetHttpStatusHeader_empty);
  CPPUNIT_TEST(testGetHttpStatusHeader_statusOnly);
  CPPUNIT_TEST(testGetHttpStatusHeader_insufficientStatusLength);
  CPPUNIT_TEST(testBeyondLimit);
  CPPUNIT_TEST(testGetHeaderString);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testUpdate1();
  void testUpdate2();
  void testGetPutBackDataLength();
  void testGetPutBackDataLength_nullChar();
  void testGetHttpStatusHeader();
  void testGetHttpStatusHeader_empty();
  void testGetHttpStatusHeader_statusOnly();
  void testGetHttpStatusHeader_insufficientStatusLength();
  void testBeyondLimit();
  void testGetHeaderString();
};


CPPUNIT_TEST_SUITE_REGISTRATION( HttpHeaderProcessorTest );

void HttpHeaderProcessorTest::testUpdate1()
{
  HttpHeaderProcessor proc;
  std::string hd1 = "HTTP/1.1 200 OK\r\n";
  proc.update(hd1);
  CPPUNIT_ASSERT(!proc.eoh());
  proc.update("\r\n");
  CPPUNIT_ASSERT(proc.eoh());
}

void HttpHeaderProcessorTest::testUpdate2()
{
  HttpHeaderProcessor proc;
  std::string hd1 = "HTTP/1.1 200 OK\n";
  proc.update(hd1);
  CPPUNIT_ASSERT(!proc.eoh());
  proc.update("\n");
  CPPUNIT_ASSERT(proc.eoh());
}

void HttpHeaderProcessorTest::testGetPutBackDataLength()
{
  HttpHeaderProcessor proc;
  std::string hd1 = "HTTP/1.1 200 OK\r\n"
    "\r\nputbackme";
  proc.update(hd1);
  CPPUNIT_ASSERT(proc.eoh());
  CPPUNIT_ASSERT_EQUAL((int32_t)9, proc.getPutBackDataLength());

  proc.clear();

  std::string hd2 = "HTTP/1.1 200 OK\n"
    "\nputbackme";
  proc.update(hd2);
  CPPUNIT_ASSERT(proc.eoh());
  CPPUNIT_ASSERT_EQUAL((int32_t)9, proc.getPutBackDataLength());
}

void HttpHeaderProcessorTest::testGetPutBackDataLength_nullChar()
{
  HttpHeaderProcessor proc;
  const char* x = "HTTP/1.1 200 OK\r\n"
    "foo: foo\0bar\r\n"
    "\r\nputbackme";
  std::string hd1(&x[0], &x[42]);
  proc.update(hd1);
  CPPUNIT_ASSERT(proc.eoh());
  CPPUNIT_ASSERT_EQUAL((int32_t)9, proc.getPutBackDataLength());
}

void HttpHeaderProcessorTest::testGetHttpStatusHeader()
{
  HttpHeaderProcessor proc;
  std::string hd = "HTTP/1.1 200 OK\r\n"
    "Date: Mon, 25 Jun 2007 16:04:59 GMT\r\n"
    "Server: Apache/2.2.3 (Debian)\r\n"
    "Last-Modified: Tue, 12 Jun 2007 14:28:43 GMT\r\n"
    "ETag: \"594065-23e3-50825cc0\"\r\n"
    "Accept-Ranges: bytes\r\n"
    "Content-Length: 9187\r\n"
    "Connection: close\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "\r\n";

  proc.update(hd);

  std::pair<std::string, SharedHandle<HttpHeader> > statusHeader = proc.getHttpStatusHeader();
  std::string status = statusHeader.first;
  SharedHandle<HttpHeader> header = statusHeader.second;
  CPPUNIT_ASSERT_EQUAL(std::string("200"), status);
  CPPUNIT_ASSERT_EQUAL(std::string("Mon, 25 Jun 2007 16:04:59 GMT"), header->getFirst("Date"));
  CPPUNIT_ASSERT_EQUAL(std::string("Apache/2.2.3 (Debian)"), header->getFirst("Server"));
  CPPUNIT_ASSERT_EQUAL((int64_t)9187LL, header->getFirstAsLLInt("Content-Length"));
  CPPUNIT_ASSERT_EQUAL(std::string("text/html; charset=UTF-8"), header->getFirst("Content-Type"));
}

void HttpHeaderProcessorTest::testGetHttpStatusHeader_empty()
{
  HttpHeaderProcessor proc;

  try {
    std::pair<std::string, SharedHandle<HttpHeader> > statusHeader = proc.getHttpStatusHeader();
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(DlRetryEx* ex) {
    std::cout << ex->getMsg() << std::endl;
    delete ex;
  }
  
}

void HttpHeaderProcessorTest::testGetHttpStatusHeader_statusOnly()
{
  HttpHeaderProcessor proc;

  std::string hd = "HTTP/1.1 200\r\n\r\n";
  proc.update(hd);
  std::pair<std::string, SharedHandle<HttpHeader> > statusHeader = proc.getHttpStatusHeader();
  CPPUNIT_ASSERT_EQUAL(std::string("200"), statusHeader.first);
  CPPUNIT_ASSERT(!statusHeader.second.isNull());
}

void HttpHeaderProcessorTest::testGetHttpStatusHeader_insufficientStatusLength()
{
  HttpHeaderProcessor proc;

  std::string hd = "HTTP/1.1 20\r\n\r\n";
  proc.update(hd);  
  try {
    std::pair<std::string, SharedHandle<HttpHeader> > statusHeader = proc.getHttpStatusHeader();
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(DlRetryEx* ex) {
    std::cout << ex->getMsg() << std::endl;
    delete ex;
  }
  
}

void HttpHeaderProcessorTest::testBeyondLimit()
{
  HttpHeaderProcessor proc;
  proc.setHeaderLimit(20);

  std::string hd1 = "HTTP/1.1 200 OK\r\n";
  std::string hd2 = "Date: Mon, 25 Jun 2007 16:04:59 GMT\r\n";

  proc.update(hd1);
  
  try {
    proc.update(hd2);
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(DlAbortEx* ex) {
    std::cout << ex->getMsg() << std::endl;
    delete ex;
  }
}

void HttpHeaderProcessorTest::testGetHeaderString()
{
  HttpHeaderProcessor proc;
  std::string hd = "HTTP/1.1 200 OK\r\n"
    "Date: Mon, 25 Jun 2007 16:04:59 GMT\r\n"
    "Server: Apache/2.2.3 (Debian)\r\n"
    "Last-Modified: Tue, 12 Jun 2007 14:28:43 GMT\r\n"
    "ETag: \"594065-23e3-50825cc0\"\r\n"
    "Accept-Ranges: bytes\r\n"
    "Content-Length: 9187\r\n"
    "Connection: close\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "\r\nputbackme";

  proc.update(hd);

  CPPUNIT_ASSERT_EQUAL(std::string("HTTP/1.1 200 OK\r\n"
			      "Date: Mon, 25 Jun 2007 16:04:59 GMT\r\n"
			      "Server: Apache/2.2.3 (Debian)\r\n"
			      "Last-Modified: Tue, 12 Jun 2007 14:28:43 GMT\r\n"
			      "ETag: \"594065-23e3-50825cc0\"\r\n"
			      "Accept-Ranges: bytes\r\n"
			      "Content-Length: 9187\r\n"
			      "Connection: close\r\n"
			      "Content-Type: text/html; charset=UTF-8"),
		       proc.getHeaderString());
}

} // namespace aria2
