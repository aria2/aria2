#include "HttpRequest.h"
#include "prefs.h"
#include "AuthConfigFactory.h"
#include "PiecedSegment.h"
#include "Piece.h"
#include "Range.h"
#include "Request.h"
#include "CookieBox.h"
#include "Option.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class HttpRequestTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HttpRequestTest);
  CPPUNIT_TEST(testGetStartByte);
  CPPUNIT_TEST(testGetEndByte);
  CPPUNIT_TEST(testCreateRequest);
  CPPUNIT_TEST(testCreateRequest_ftp);
  CPPUNIT_TEST(testCreateRequest_with_cookie);
  CPPUNIT_TEST(testCreateProxyRequest);
  CPPUNIT_TEST(testIsRangeSatisfied);
  CPPUNIT_TEST(testUserAgent);
  CPPUNIT_TEST(testUserHeaders);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testGetStartByte();
  void testGetEndByte();
  void testCreateRequest();
  void testCreateRequest_ftp();
  void testCreateRequest_with_cookie();
  void testCreateProxyRequest();
  void testIsRangeSatisfied();
  void testUserAgent();
  void testUserHeaders();
};


CPPUNIT_TEST_SUITE_REGISTRATION( HttpRequestTest );

void HttpRequestTest::testGetStartByte()
{
  HttpRequest httpRequest;
  SharedHandle<Piece> p(new Piece(1, 1024));
  SharedHandle<Segment> segment(new PiecedSegment(1024, p));

  CPPUNIT_ASSERT_EQUAL(0LL, httpRequest.getStartByte());

  httpRequest.setSegment(segment);
  
  CPPUNIT_ASSERT_EQUAL(1024LL, httpRequest.getStartByte());

}

void HttpRequestTest::testGetEndByte()
{
  size_t index = 1;
  size_t length = 1024*1024-1024;
  size_t segmentLength = 1024*1024;

  HttpRequest httpRequest;
  SharedHandle<Piece> piece(new Piece(index, length));
  SharedHandle<Segment> segment(new PiecedSegment(segmentLength, piece));

  CPPUNIT_ASSERT_EQUAL(0LL, httpRequest.getEndByte());

  httpRequest.setSegment(segment);

  CPPUNIT_ASSERT_EQUAL(0LL, httpRequest.getEndByte());

  SharedHandle<Request> request(new Request());
  request->supportsPersistentConnection(true);
  request->setPipeliningHint(true);

  httpRequest.setRequest(request);

  CPPUNIT_ASSERT_EQUAL((off_t)segmentLength*index+length-1,
		       httpRequest.getEndByte());


  request->setPipeliningHint(false);

  CPPUNIT_ASSERT_EQUAL(0LL, httpRequest.getEndByte());
}

void HttpRequestTest::testCreateRequest()
{
  SharedHandle<Piece> p;

  Option option;
  option.put(PREF_HTTP_AUTH_ENABLED, V_FALSE);
  option.put(PREF_HTTP_PROXY_ENABLED, V_FALSE);
  option.put(PREF_HTTP_PROXY_METHOD, V_TUNNEL);
  option.put(PREF_HTTP_PROXY_AUTH_ENABLED, V_FALSE);
  option.put(PREF_HTTP_USER, "aria2user");
  option.put(PREF_HTTP_PASSWD, "aria2passwd");
  option.put(PREF_HTTP_PROXY_USER, "aria2proxyuser");
  option.put(PREF_HTTP_PROXY_PASSWD, "aria2proxypasswd");

  SharedHandle<AuthConfigFactory> authConfigFactory(new AuthConfigFactory(&option));
  SingletonHolder<SharedHandle<AuthConfigFactory> >::instance(authConfigFactory);

  SharedHandle<Request> request(new Request());
  request->supportsPersistentConnection(true);

  request->setUrl("http://localhost:8080/archives/aria2-1.0.0.tar.bz2");

  p.reset(new Piece(0, 1024));
  SharedHandle<Segment> segment(new PiecedSegment(1024, p));

  HttpRequest httpRequest;
  httpRequest.setRequest(request);
  httpRequest.setSegment(segment);

  // remove "Connection: close" and add end byte range
  request->setPipeliningHint(true);  
  
  std::string expectedText = "GET /archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Range: bytes=0-1023\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  request->setPipeliningHint(false);

  expectedText = "GET /archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  p.reset(new Piece(1, 1024*1024));
  segment.reset(new PiecedSegment(1024*1024, p));
  httpRequest.setSegment(segment);

  expectedText = "GET /archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Range: bytes=1048576-\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  request->setPipeliningHint(true);

  expectedText = "GET /archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Range: bytes=1048576-2097151\r\n"
    "\r\n";
  
  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  // redirection clears persistent connection falg
  request->redirectUrl("http://localhost:8080/archives/download/aria2-1.0.0.tar.bz2");

  expectedText = "GET /archives/download/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Range: bytes=1048576-\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  request->supportsPersistentConnection(true);
  request->setPipeliningHint(false);

  // this only removes "Connection: close".
  request->setKeepAliveHint(true);

  expectedText = "GET /archives/download/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Range: bytes=1048576-\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  request->setKeepAliveHint(false);

  request->resetUrl();

  p.reset(new Piece(0, 1024*1024));
  segment.reset(new PiecedSegment(1024*1024, p));
  httpRequest.setSegment(segment);

  // enable http auth
  option.put(PREF_HTTP_AUTH_ENABLED, V_TRUE);
  
  httpRequest.configure(&option);

  expectedText = "GET /archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Authorization: Basic YXJpYTJ1c2VyOmFyaWEycGFzc3dk\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  // enable http proxy auth
  option.put(PREF_HTTP_PROXY_AUTH_ENABLED, V_TRUE);

  httpRequest.configure(&option);

  expectedText = "GET /archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Authorization: Basic YXJpYTJ1c2VyOmFyaWEycGFzc3dk\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  option.put(PREF_HTTP_PROXY_ENABLED, V_TRUE);

  httpRequest.configure(&option);

  expectedText = "GET /archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Authorization: Basic YXJpYTJ1c2VyOmFyaWEycGFzc3dk\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  option.put(PREF_HTTP_PROXY_METHOD, V_GET);

  httpRequest.configure(&option);

  expectedText = "GET http://localhost:8080/archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Proxy-Connection: close\r\n"
    "Proxy-Authorization: Basic YXJpYTJwcm94eXVzZXI6YXJpYTJwcm94eXBhc3N3ZA==\r\n"
    "Authorization: Basic YXJpYTJ1c2VyOmFyaWEycGFzc3dk\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  request->setPipeliningHint(true);

  expectedText = "GET http://localhost:8080/archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Range: bytes=0-1048575\r\n"
    "Proxy-Connection: Keep-Alive\r\n"
    "Proxy-Authorization: Basic YXJpYTJwcm94eXVzZXI6YXJpYTJwcm94eXBhc3N3ZA==\r\n"
    "Authorization: Basic YXJpYTJ1c2VyOmFyaWEycGFzc3dk\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  request->setPipeliningHint(false);

  option.put(PREF_HTTP_PROXY_AUTH_ENABLED, V_FALSE);

  httpRequest.configure(&option);

  expectedText = "GET http://localhost:8080/archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Proxy-Connection: close\r\n"
    "Authorization: Basic YXJpYTJ1c2VyOmFyaWEycGFzc3dk\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());  
}

void HttpRequestTest::testCreateRequest_ftp()
{
  Option option;
  option.put(PREF_HTTP_AUTH_ENABLED, V_FALSE);
  option.put(PREF_HTTP_PROXY_ENABLED, V_FALSE);
  option.put(PREF_HTTP_PROXY_METHOD, V_TUNNEL);
  option.put(PREF_HTTP_PROXY_AUTH_ENABLED, V_FALSE);
  option.put(PREF_HTTP_USER, "aria2user");
  option.put(PREF_HTTP_PASSWD, "aria2passwd");
  option.put(PREF_HTTP_PROXY_USER, "aria2proxyuser");
  option.put(PREF_HTTP_PROXY_PASSWD, "aria2proxypasswd");

  SharedHandle<AuthConfigFactory> authConfigFactory
    (new AuthConfigFactory(&option));
  SingletonHolder<SharedHandle<AuthConfigFactory> >::instance(authConfigFactory);

  SharedHandle<Request> request(new Request());
  request->setUrl("ftp://localhost:8080/archives/aria2-1.0.0.tar.bz2");

  HttpRequest httpRequest;
  SharedHandle<Piece> p(new Piece(0, 1024*1024));
  SharedHandle<Segment> segment
    (new PiecedSegment(1024*1024, p));
  httpRequest.setRequest(request);
  httpRequest.setSegment(segment);

  httpRequest.configure(&option);

  std::string expectedText = "GET ftp://localhost:8080/archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  // How to enable HTTP proxy authorization in FTP download via HTTP proxy
  option.put(PREF_HTTP_PROXY_ENABLED, V_TRUE);
  option.put(PREF_HTTP_PROXY_METHOD, V_GET);
  option.put(PREF_HTTP_PROXY_AUTH_ENABLED, V_TRUE);

  httpRequest.configure(&option);

  expectedText = "GET ftp://localhost:8080/archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Proxy-Connection: close\r\n"
    "Proxy-Authorization: Basic YXJpYTJwcm94eXVzZXI6YXJpYTJwcm94eXBhc3N3ZA==\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());
  
}

void HttpRequestTest::testCreateRequest_with_cookie()
{
  SharedHandle<Request> request(new Request());
  request->setUrl("http://localhost/archives/aria2-1.0.0.tar.bz2");
  SharedHandle<Piece> p(new Piece(0, 1024*1024));
  SharedHandle<Segment> segment
    (new PiecedSegment(1024*1024, p));

  Cookie cookie1("name1", "value1", "/archives", "localhost", false);
  Cookie cookie2("name2", "value2", "/archives/download", "localhost", false);
  Cookie cookie3("name3", "value3", "/archives/download", "tt.localhost", false);
  Cookie cookie4("name4", "value4", "/archives/download", "tt.localhost", true);

  request->cookieBox->add(cookie1);
  request->cookieBox->add(cookie2);
  request->cookieBox->add(cookie3);
  request->cookieBox->add(cookie4);

  HttpRequest httpRequest;

  httpRequest.setRequest(request);
  httpRequest.setSegment(segment);

  std::string expectedText = "GET /archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Cookie: name1=value1;\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  request->setUrl("http://localhost/archives/download/aria2-1.0.0.tar.bz2");

  expectedText = "GET /archives/download/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Cookie: name1=value1;name2=value2;\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  request->setUrl("http://tt.localhost/archives/download/aria2-1.0.0.tar.bz2");

  expectedText = "GET /archives/download/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: tt.localhost\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Cookie: name1=value1;name2=value2;name3=value3;\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  request->setUrl("https://tt.localhost/archives/download/aria2-1.0.0.tar.bz2");

  expectedText = "GET /archives/download/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: tt.localhost\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Cookie: name1=value1;name2=value2;name3=value3;name4=value4;\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());
  
}

void HttpRequestTest::testCreateProxyRequest()
{
  SharedHandle<Request> request(new Request());
  request->setUrl("http://localhost/archives/aria2-1.0.0.tar.bz2");
  SharedHandle<Piece> p(new Piece(0, 1024*1024));
  SharedHandle<Segment> segment(new PiecedSegment(1024*1024, p));

  HttpRequest httpRequest;

  httpRequest.setRequest(request);
  httpRequest.setSegment(segment);

  request->supportsPersistentConnection(true);

  std::string expectedText = "CONNECT localhost:80 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Host: localhost:80\r\n"
    "Proxy-Connection: close\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createProxyRequest());

  // adds Keep-Alive header.
  request->setKeepAliveHint(true);

  expectedText = "CONNECT localhost:80 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Host: localhost:80\r\n"
    "Proxy-Connection: Keep-Alive\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createProxyRequest());

  request->setKeepAliveHint(false);
  // pipelining also adds Keep-Alive header.
  request->setPipeliningHint(true);

  expectedText = "CONNECT localhost:80 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Host: localhost:80\r\n"
    "Proxy-Connection: Keep-Alive\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createProxyRequest());  
}

void HttpRequestTest::testIsRangeSatisfied()
{
  SharedHandle<Request> request(new Request());
  request->supportsPersistentConnection(true);
  request->setUrl("http://localhost:8080/archives/aria2-1.0.0.tar.bz2");
  request->setPipeliningHint(false); // default: false
  SharedHandle<Piece> p(new Piece(0, 1024*1024));
  SharedHandle<Segment> segment(new PiecedSegment(1024*1024, p));

  HttpRequest httpRequest;

  httpRequest.setRequest(request);
  httpRequest.setSegment(segment);

  SharedHandle<Range> range(new Range());

  CPPUNIT_ASSERT(httpRequest.isRangeSatisfied(range));

  p.reset(new Piece(1, 1024*1024));
  segment.reset(new PiecedSegment(1024*1024, p));
  httpRequest.setSegment(segment);

  CPPUNIT_ASSERT(!httpRequest.isRangeSatisfied(range));

  uint64_t entityLength = segment->getSegmentLength()*10;

  range.reset(new Range(segment->getPosition(), 0, entityLength));

  CPPUNIT_ASSERT(httpRequest.isRangeSatisfied(range));

  httpRequest.setEntityLength(entityLength-1);

  CPPUNIT_ASSERT(!httpRequest.isRangeSatisfied(range));

  httpRequest.setEntityLength(entityLength);

  CPPUNIT_ASSERT(httpRequest.isRangeSatisfied(range));

  request->setPipeliningHint(true);

  CPPUNIT_ASSERT(!httpRequest.isRangeSatisfied(range));

  range.reset(new Range(segment->getPosition(),
			segment->getPosition()+segment->getLength()-1,
			entityLength));

  CPPUNIT_ASSERT(httpRequest.isRangeSatisfied(range));

  range.reset(new Range(0, segment->getPosition()+segment->getLength()-1,
			entityLength));

  CPPUNIT_ASSERT(!httpRequest.isRangeSatisfied(range));
}

void HttpRequestTest::testUserAgent()
{
  Option option;

  SharedHandle<AuthConfigFactory> authConfigFactory
    (new AuthConfigFactory(&option));
  SingletonHolder<SharedHandle<AuthConfigFactory> >::instance(authConfigFactory);

  SharedHandle<Request> request(new Request());
  request->setUrl("http://localhost:8080/archives/aria2-1.0.0.tar.bz2");

  SharedHandle<Piece> p(new Piece(0, 1024));
  SharedHandle<Segment> segment(new PiecedSegment(1024, p));

  HttpRequest httpRequest;
  httpRequest.setRequest(request);
  httpRequest.setSegment(segment);
  httpRequest.setUserAgent("aria2 (Linux)");

  std::string expectedText = "GET /archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2 (Linux)\r\n"
    "Accept: */*\r\n"
    "Host: localhost:8080\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());

  std::string expectedTextForProxy = "CONNECT localhost:8080 HTTP/1.1\r\n"
    "User-Agent: aria2 (Linux)\r\n"
    "Host: localhost:8080\r\n"
    "Proxy-Connection: close\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedTextForProxy, httpRequest.createProxyRequest());
}

void HttpRequestTest::testUserHeaders()
{
  SharedHandle<Request> request(new Request());
  request->setUrl("http://localhost/archives/aria2-1.0.0.tar.bz2");

  HttpRequest httpRequest;
  httpRequest.setRequest(request);
  httpRequest.setUserHeaders("X-ARIA2: v0.13\nX-ARIA2-DISTRIBUTE: enabled\n");

  std::string expectedText = "GET /archives/aria2-1.0.0.tar.bz2 HTTP/1.1\r\n"
    "User-Agent: aria2\r\n"
    "Accept: */*\r\n"
    "Host: localhost\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "X-ARIA2: v0.13\r\n"
    "X-ARIA2-DISTRIBUTE: enabled\r\n"
    "\r\n";

  CPPUNIT_ASSERT_EQUAL(expectedText, httpRequest.createRequest());
}


} // namespace aria2
