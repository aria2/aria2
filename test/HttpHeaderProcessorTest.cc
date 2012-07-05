#include "HttpHeaderProcessor.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "HttpHeader.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"

namespace aria2 {

class HttpHeaderProcessorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HttpHeaderProcessorTest);
  CPPUNIT_TEST(testParse1);
  CPPUNIT_TEST(testParse2);
  CPPUNIT_TEST(testParse3);
  CPPUNIT_TEST(testGetLastBytesProcessed);
  CPPUNIT_TEST(testGetLastBytesProcessed_nullChar);
  CPPUNIT_TEST(testGetHttpResponseHeader);
  CPPUNIT_TEST(testGetHttpResponseHeader_statusOnly);
  CPPUNIT_TEST(testGetHttpResponseHeader_insufficientStatusLength);
  CPPUNIT_TEST(testGetHttpResponseHeader_nameStartsWs);
  CPPUNIT_TEST(testBeyondLimit);
  CPPUNIT_TEST(testGetHeaderString);
  CPPUNIT_TEST(testGetHttpRequestHeader);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testParse1();
  void testParse2();
  void testParse3();
  void testGetLastBytesProcessed();
  void testGetLastBytesProcessed_nullChar();
  void testGetHttpResponseHeader();
  void testGetHttpResponseHeader_statusOnly();
  void testGetHttpResponseHeader_insufficientStatusLength();
  void testGetHttpResponseHeader_nameStartsWs();
  void testBeyondLimit();
  void testGetHeaderString();
  void testGetHttpRequestHeader();
};


CPPUNIT_TEST_SUITE_REGISTRATION( HttpHeaderProcessorTest );

void HttpHeaderProcessorTest::testParse1()
{
  HttpHeaderProcessor proc(HttpHeaderProcessor::CLIENT_PARSER);
  std::string hd1 = "HTTP/1.1 200 OK\r\n";
  CPPUNIT_ASSERT(!proc.parse(hd1));
  CPPUNIT_ASSERT(proc.parse("\r\n"));
}

void HttpHeaderProcessorTest::testParse2()
{
  HttpHeaderProcessor proc(HttpHeaderProcessor::CLIENT_PARSER);
  std::string hd1 = "HTTP/1.1 200 OK\n";
  CPPUNIT_ASSERT(!proc.parse(hd1));
  CPPUNIT_ASSERT(proc.parse("\n"));
}

void HttpHeaderProcessorTest::testParse3()
{
  HttpHeaderProcessor proc(HttpHeaderProcessor::SERVER_PARSER);
  std::string s =
    "GET / HTTP/1.1\r\n"
    "Host: aria2.sourceforge.net\r\n"
    "Connection: close \r\n" // trailing white space (BWS)
    "Multi-Line: text1\r\n" // Multi-line header
    "  text2\r\n"
    "  text3\r\n"
    "Duplicate: foo\r\n"
    "Duplicate: bar\r\n"
    "No-value:\r\n"
    "\r\n";
  CPPUNIT_ASSERT(proc.parse(s));
  SharedHandle<HttpHeader> h = proc.getResult();
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sourceforge.net"),
                       h->find("host"));
  CPPUNIT_ASSERT_EQUAL(std::string("close"),
                       h->find("connection"));
  CPPUNIT_ASSERT_EQUAL(std::string("text1 text2 text3"),
                       h->find("multi-line"));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"),
                       h->findAll("duplicate")[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("bar"),
                       h->findAll("duplicate")[1]);
  CPPUNIT_ASSERT_EQUAL(std::string(""), h->find("no-value"));
  CPPUNIT_ASSERT(h->defined("no-value"));
}

void HttpHeaderProcessorTest::testGetLastBytesProcessed()
{
  HttpHeaderProcessor proc(HttpHeaderProcessor::CLIENT_PARSER);
  std::string hd1 =
    "HTTP/1.1 200 OK\r\n"
    "\r\nputbackme";
  CPPUNIT_ASSERT(proc.parse(hd1));
  CPPUNIT_ASSERT_EQUAL((size_t)19, proc.getLastBytesProcessed());

  proc.clear();

  std::string hd2 =
    "HTTP/1.1 200 OK\n"
    "\nputbackme";
  CPPUNIT_ASSERT(proc.parse(hd2));
  CPPUNIT_ASSERT_EQUAL((size_t)17, proc.getLastBytesProcessed());
}

void HttpHeaderProcessorTest::testGetLastBytesProcessed_nullChar()
{
  HttpHeaderProcessor proc(HttpHeaderProcessor::CLIENT_PARSER);
  const char x[] =
    "HTTP/1.1 200 OK\r\n"
    "foo: foo\0bar\r\n"
    "\r\nputbackme";
  std::string hd1(&x[0], &x[sizeof(x)-1]);
  CPPUNIT_ASSERT(proc.parse(hd1));
  CPPUNIT_ASSERT_EQUAL((size_t)33, proc.getLastBytesProcessed());
}

void HttpHeaderProcessorTest::testGetHttpResponseHeader()
{
  HttpHeaderProcessor proc(HttpHeaderProcessor::CLIENT_PARSER);
  std::string hd =
    "HTTP/1.1 404 Not Found\r\n"
    "Date: Mon, 25 Jun 2007 16:04:59 GMT\r\n"
    "Server: Apache/2.2.3 (Debian)\r\n"
    "Last-Modified: Tue, 12 Jun 2007 14:28:43 GMT\r\n"
    "ETag: \"594065-23e3-50825cc0\"\r\n"
    "Accept-Ranges: bytes\r\n"
    "Content-Length: 9187\r\n"
    "Connection: close\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "\r\n"
    "Entity: body";

  CPPUNIT_ASSERT(proc.parse(hd));

  SharedHandle<HttpHeader> header = proc.getResult();
  CPPUNIT_ASSERT_EQUAL(404, header->getStatusCode());
  CPPUNIT_ASSERT_EQUAL(std::string("Not Found"), header->getReasonPhrase());
  CPPUNIT_ASSERT_EQUAL(std::string("HTTP/1.1"), header->getVersion());
  CPPUNIT_ASSERT_EQUAL(std::string("Mon, 25 Jun 2007 16:04:59 GMT"),
                       header->find("date"));
  CPPUNIT_ASSERT_EQUAL(std::string("Apache/2.2.3 (Debian)"),
                       header->find("server"));
  CPPUNIT_ASSERT_EQUAL((int64_t)9187LL,
                       header->findAsLLInt("content-length"));
  CPPUNIT_ASSERT_EQUAL(std::string("text/html; charset=UTF-8"),
                       header->find("content-type"));
  CPPUNIT_ASSERT(!header->defined("entity"));
}

void HttpHeaderProcessorTest::testGetHttpResponseHeader_statusOnly()
{
  HttpHeaderProcessor proc(HttpHeaderProcessor::CLIENT_PARSER);

  std::string hd = "HTTP/1.1 200\r\n\r\n";
  CPPUNIT_ASSERT(proc.parse(hd));
  SharedHandle<HttpHeader> header = proc.getResult();
  CPPUNIT_ASSERT_EQUAL(200, header->getStatusCode());
}

void HttpHeaderProcessorTest::testGetHttpResponseHeader_insufficientStatusLength()
{
  HttpHeaderProcessor proc(HttpHeaderProcessor::CLIENT_PARSER);

  std::string hd = "HTTP/1.1 20\r\n\r\n";
  try {
    proc.parse(hd);
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(DlAbortEx& ex) {
    // Success
  }
}

void HttpHeaderProcessorTest::testGetHttpResponseHeader_nameStartsWs()
{
  HttpHeaderProcessor proc(HttpHeaderProcessor::CLIENT_PARSER);

  std::string hd =
    "HTTP/1.1 200\r\n"
    " foo:bar\r\n"
    "\r\n";
  try {
    proc.parse(hd);
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(DlAbortEx& ex) {
    // Success
  }

  proc.clear();
  hd =
    "HTTP/1.1 200\r\n"
    ":foo:bar\r\n"
    "\r\n";
  try {
    proc.parse(hd);
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(DlAbortEx& ex) {
    // Success
  }

  proc.clear();
  hd =
    "HTTP/1.1 200\r\n"
    ":foo\r\n"
    "\r\n";
  try {
    proc.parse(hd);
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(DlAbortEx& ex) {
    // Success
  }
}

void HttpHeaderProcessorTest::testBeyondLimit()
{
  HttpHeaderProcessor proc(HttpHeaderProcessor::CLIENT_PARSER);

  std::string hd1 = "HTTP/1.1 200 OK\r\n";
  std::string hd2 = std::string(1025, 'A');

  proc.parse(hd1);
  try {
    proc.parse(hd2);
    CPPUNIT_FAIL("Exception must be thrown.");
  } catch(DlAbortEx& ex) {
    // Success
  }
}

void HttpHeaderProcessorTest::testGetHeaderString()
{
  HttpHeaderProcessor proc(HttpHeaderProcessor::CLIENT_PARSER);
  std::string hd =
    "HTTP/1.1 200 OK\r\n"
    "Date: Mon, 25 Jun 2007 16:04:59 GMT\r\n"
    "Server: Apache/2.2.3 (Debian)\r\n"
    "Last-Modified: Tue, 12 Jun 2007 14:28:43 GMT\r\n"
    "ETag: \"594065-23e3-50825cc0\"\r\n"
    "Accept-Ranges: bytes\r\n"
    "Content-Length: 9187\r\n"
    "Connection: close\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "\r\nputbackme";

  CPPUNIT_ASSERT(proc.parse(hd));

  CPPUNIT_ASSERT_EQUAL
    (std::string("HTTP/1.1 200 OK\r\n"
                 "Date: Mon, 25 Jun 2007 16:04:59 GMT\r\n"
                 "Server: Apache/2.2.3 (Debian)\r\n"
                 "Last-Modified: Tue, 12 Jun 2007 14:28:43 GMT\r\n"
                 "ETag: \"594065-23e3-50825cc0\"\r\n"
                 "Accept-Ranges: bytes\r\n"
                 "Content-Length: 9187\r\n"
                 "Connection: close\r\n"
                 "Content-Type: text/html; charset=UTF-8\r\n"
                 "\r\n"),
     proc.getHeaderString());
}

void HttpHeaderProcessorTest::testGetHttpRequestHeader()
{
  HttpHeaderProcessor proc(HttpHeaderProcessor::SERVER_PARSER);
  std::string request =
    "GET /index.html HTTP/1.1\r\n"
    "Host: host\r\n"
    "Connection: close\r\n"
    "\r\n"
    "Entity: body";

  CPPUNIT_ASSERT(proc.parse(request));

  SharedHandle<HttpHeader> httpHeader = proc.getResult();
  CPPUNIT_ASSERT_EQUAL(std::string("GET"), httpHeader->getMethod());
  CPPUNIT_ASSERT_EQUAL(std::string("/index.html"),httpHeader->getRequestPath());
  CPPUNIT_ASSERT_EQUAL(std::string("HTTP/1.1"), httpHeader->getVersion());
  CPPUNIT_ASSERT_EQUAL(std::string("close"),httpHeader->find("connection"));
  CPPUNIT_ASSERT(!httpHeader->defined("entity"));
}

} // namespace aria2
