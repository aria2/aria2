#include "HttpResponse.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "prefs.h"
#include "PiecedSegment.h"
#include "Piece.h"
#include "Request.h"
#include "HttpHeader.h"
#include "HttpRequest.h"
#include "Exception.h"
#include "A2STR.h"
#include "DlRetryEx.h"
#include "CookieStorage.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "StreamFilter.h"
#include "MetalinkHttpEntry.h"
#include "Option.h"
#include "Checksum.h"

namespace aria2 {

class HttpResponseTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HttpResponseTest);
  CPPUNIT_TEST(testGetContentLength_null);
  CPPUNIT_TEST(testGetContentLength_contentLength);
  //CPPUNIT_TEST(testGetContentLength_range);
  CPPUNIT_TEST(testGetEntityLength);
  CPPUNIT_TEST(testGetContentType);
  CPPUNIT_TEST(testDeterminFilename_without_ContentDisposition);
  CPPUNIT_TEST(testDeterminFilename_with_ContentDisposition_zero_length);
  CPPUNIT_TEST(testDeterminFilename_with_ContentDisposition);
  CPPUNIT_TEST(testGetRedirectURI_without_Location);
  CPPUNIT_TEST(testGetRedirectURI_with_Location);
  CPPUNIT_TEST(testIsRedirect);
  CPPUNIT_TEST(testIsTransferEncodingSpecified);
  CPPUNIT_TEST(testGetTransferEncoding);
  CPPUNIT_TEST(testGetTransferEncodingStreamFilter);
  CPPUNIT_TEST(testIsContentEncodingSpecified);
  CPPUNIT_TEST(testGetContentEncoding);
  CPPUNIT_TEST(testGetContentEncodingStreamFilter);
  CPPUNIT_TEST(testValidateResponse);
  CPPUNIT_TEST(testValidateResponse_good_range);
  CPPUNIT_TEST(testValidateResponse_bad_range);
  CPPUNIT_TEST(testValidateResponse_chunked);
  CPPUNIT_TEST(testValidateResponse_withIfModifiedSince);
  CPPUNIT_TEST(testHasRetryAfter);
  CPPUNIT_TEST(testProcessRedirect);
  CPPUNIT_TEST(testRetrieveCookie);
  CPPUNIT_TEST(testSupportsPersistentConnection);
  CPPUNIT_TEST(testGetMetalinKHttpEntries);
#ifdef ENABLE_MESSAGE_DIGEST
  CPPUNIT_TEST(testGetDigest);
#endif // ENABLE_MESSAGE_DIGEST
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testGetContentLength_null();
  void testGetContentLength_contentLength();
  void testGetEntityLength();
  void testGetContentType();
  void testDeterminFilename_without_ContentDisposition();
  void testDeterminFilename_with_ContentDisposition_zero_length();
  void testDeterminFilename_with_ContentDisposition();
  void testGetRedirectURI_without_Location();
  void testGetRedirectURI_with_Location();
  void testIsRedirect();
  void testIsTransferEncodingSpecified();
  void testGetTransferEncoding();
  void testGetTransferEncodingStreamFilter();
  void testIsContentEncodingSpecified();
  void testGetContentEncoding();
  void testGetContentEncodingStreamFilter();
  void testValidateResponse();
  void testValidateResponse_good_range();
  void testValidateResponse_bad_range();
  void testValidateResponse_chunked();
  void testValidateResponse_withIfModifiedSince();
  void testHasRetryAfter();
  void testProcessRedirect();
  void testRetrieveCookie();
  void testSupportsPersistentConnection();
  void testGetMetalinKHttpEntries();
#ifdef ENABLE_MESSAGE_DIGEST
  void testGetDigest();
#endif // ENABLE_MESSAGE_DIGEST
};


CPPUNIT_TEST_SUITE_REGISTRATION( HttpResponseTest );

void HttpResponseTest::testGetContentLength_null()
{
  HttpResponse httpResponse;

  CPPUNIT_ASSERT_EQUAL((uint64_t)0ULL, httpResponse.getContentLength());
}

void HttpResponseTest::testGetContentLength_contentLength()
{
  HttpResponse httpResponse;

  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpHeader->put("Content-Length", "4294967296");

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT_EQUAL((uint64_t)4294967296ULL, httpResponse.getContentLength());
}

void HttpResponseTest::testGetEntityLength()
{
  HttpResponse httpResponse;

  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpHeader->put("Content-Length", "4294967296");

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT_EQUAL((uint64_t)4294967296ULL, httpResponse.getEntityLength());

  httpHeader->put("Content-Range", "bytes 1-4294967296/4294967297");

  CPPUNIT_ASSERT_EQUAL((uint64_t)4294967297ULL, httpResponse.getEntityLength());

}

void HttpResponseTest::testGetContentType()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpHeader->put("content-type", "application/metalink+xml; charset=UTF-8");
  httpResponse.setHttpHeader(httpHeader);
  // See paramter is ignored.
  CPPUNIT_ASSERT_EQUAL(std::string("application/metalink+xml"),
                       httpResponse.getContentType());
}

void HttpResponseTest::testDeterminFilename_without_ContentDisposition()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  SharedHandle<HttpRequest> httpRequest(new HttpRequest());
  SharedHandle<Request> request(new Request());
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);

  httpResponse.setHttpHeader(httpHeader);
  httpResponse.setHttpRequest(httpRequest);

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"),
                       httpResponse.determinFilename());
}

void HttpResponseTest::testDeterminFilename_with_ContentDisposition_zero_length
()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpHeader->put("Content-Disposition", "attachment; filename=\"\"");
  SharedHandle<HttpRequest> httpRequest(new HttpRequest());
  SharedHandle<Request> request(new Request());
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);

  httpResponse.setHttpHeader(httpHeader);
  httpResponse.setHttpRequest(httpRequest);

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"),
                       httpResponse.determinFilename());
}

void HttpResponseTest::testDeterminFilename_with_ContentDisposition()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpHeader->put("Content-Disposition",
                  "attachment; filename=\"aria2-current.tar.bz2\"");
  SharedHandle<HttpRequest> httpRequest(new HttpRequest());
  SharedHandle<Request> request(new Request());
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);

  httpResponse.setHttpHeader(httpHeader);
  httpResponse.setHttpRequest(httpRequest);

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-current.tar.bz2"),
                       httpResponse.determinFilename());
}

void HttpResponseTest::testGetRedirectURI_without_Location()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       httpResponse.getRedirectURI());  
}

void HttpResponseTest::testGetRedirectURI_with_Location()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpHeader->put("Location", "http://localhost/download/aria2-1.0.0.tar.bz2");
  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT_EQUAL
    (std::string("http://localhost/download/aria2-1.0.0.tar.bz2"),
     httpResponse.getRedirectURI());
}

void HttpResponseTest::testIsRedirect()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpHeader->setStatusCode(200);
  httpHeader->put("Location", "http://localhost/download/aria2-1.0.0.tar.bz2");

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT(!httpResponse.isRedirect());

  httpHeader->setStatusCode(301);

  CPPUNIT_ASSERT(httpResponse.isRedirect());  
}

void HttpResponseTest::testIsTransferEncodingSpecified()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT(!httpResponse.isTransferEncodingSpecified());  

  httpHeader->put("Transfer-Encoding", "chunked");

  CPPUNIT_ASSERT(httpResponse.isTransferEncodingSpecified());
}

void HttpResponseTest::testGetTransferEncoding()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT_EQUAL(std::string(""), httpResponse.getTransferEncoding());  

  httpHeader->put("Transfer-Encoding", "chunked");

  CPPUNIT_ASSERT_EQUAL(std::string("chunked"),
                       httpResponse.getTransferEncoding());
}

void HttpResponseTest::testGetTransferEncodingStreamFilter()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT(!httpResponse.getTransferEncodingStreamFilter());

  httpHeader->put("Transfer-Encoding", "chunked");

  CPPUNIT_ASSERT(httpResponse.getTransferEncodingStreamFilter());
}

void HttpResponseTest::testIsContentEncodingSpecified()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  
  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT(!httpResponse.isContentEncodingSpecified());

  httpHeader->put("Content-Encoding", "gzip");

  CPPUNIT_ASSERT(httpResponse.isContentEncodingSpecified());
}

void HttpResponseTest::testGetContentEncoding()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  
  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT_EQUAL(A2STR::NIL, httpResponse.getContentEncoding());

  httpHeader->put("Content-Encoding", "gzip");

  CPPUNIT_ASSERT_EQUAL(std::string("gzip"), httpResponse.getContentEncoding());
}

void HttpResponseTest::testGetContentEncodingStreamFilter()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  
  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT(!httpResponse.getContentEncodingStreamFilter());

#ifdef HAVE_ZLIB
  httpHeader->put("Content-Encoding", "gzip");
  {
    SharedHandle<StreamFilter> filter =
      httpResponse.getContentEncodingStreamFilter();
    CPPUNIT_ASSERT(filter);
    CPPUNIT_ASSERT_EQUAL(std::string("GZipDecodingStreamFilter"),
                         filter->getName());
  }
  httpHeader.reset(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);
  httpHeader->put("Content-Encoding", "deflate");
  {
    SharedHandle<StreamFilter> filter =
      httpResponse.getContentEncodingStreamFilter();
    CPPUNIT_ASSERT(filter);
    CPPUNIT_ASSERT_EQUAL(std::string("GZipDecodingStreamFilter"),
                         filter->getName());
  }
#endif // HAVE_ZLIB
  httpHeader.reset(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);
  httpHeader->put("Content-Encoding", "bzip2");
  {
    SharedHandle<StreamFilter> filter =
      httpResponse.getContentEncodingStreamFilter();
    CPPUNIT_ASSERT(!filter);
  }
}

void HttpResponseTest::testValidateResponse()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);

  httpHeader->setStatusCode(301);

  try {
    httpResponse.validateResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }

  httpHeader->put("Location", "http://localhost/archives/aria2-1.0.0.tar.bz2");
  try {
    httpResponse.validateResponse();
  } catch(Exception& e) {
    CPPUNIT_FAIL("exception must not be thrown.");
  }
}
 
void HttpResponseTest::testValidateResponse_good_range()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);

  SharedHandle<HttpRequest> httpRequest(new HttpRequest());
  SharedHandle<Piece> p(new Piece(1, 1024*1024));
  SharedHandle<Segment> segment(new PiecedSegment(1024*1024, p));
  httpRequest->setSegment(segment);
  SharedHandle<FileEntry> fileEntry(new FileEntry("file", 1024*1024*10, 0));
  httpRequest->setFileEntry(fileEntry);
  SharedHandle<Request> request(new Request());
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);
  httpResponse.setHttpRequest(httpRequest);
  httpHeader->setStatusCode(206);
  httpHeader->put("Content-Range", "bytes 1048576-10485760/10485760");
  
  try {
    httpResponse.validateResponse();
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
    CPPUNIT_FAIL("exception must not be thrown.");
  }
}

void HttpResponseTest::testValidateResponse_bad_range()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);

  SharedHandle<HttpRequest> httpRequest(new HttpRequest());
  SharedHandle<Piece> p(new Piece(1, 1024*1024));
  SharedHandle<Segment> segment(new PiecedSegment(1024*1024, p));
  httpRequest->setSegment(segment);
  SharedHandle<FileEntry> fileEntry(new FileEntry("file", 1024*1024*10, 0));
  httpRequest->setFileEntry(fileEntry);
  SharedHandle<Request> request(new Request());
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);
  httpResponse.setHttpRequest(httpRequest);
  httpHeader->setStatusCode(206);
  httpHeader->put("Content-Range", "bytes 0-10485760/10485761");

  try {
    httpResponse.validateResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
}

void HttpResponseTest::testValidateResponse_chunked()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);

  SharedHandle<HttpRequest> httpRequest(new HttpRequest());
  SharedHandle<Piece> p(new Piece(1, 1024*1024));
  SharedHandle<Segment> segment(new PiecedSegment(1024*1024, p));
  httpRequest->setSegment(segment);
  SharedHandle<FileEntry> fileEntry(new FileEntry("file", 1024*1024*10, 0));
  httpRequest->setFileEntry(fileEntry);
  SharedHandle<Request> request(new Request());
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);
  httpResponse.setHttpRequest(httpRequest);
  httpHeader->setStatusCode(206);
  httpHeader->put("Content-Range", "bytes 0-10485760/10485761");
  httpHeader->put("Transfer-Encoding", "chunked");

  // if transfer-encoding is specified, then range validation is skipped.
  try {
    httpResponse.validateResponse();
  } catch(Exception& e) {
    CPPUNIT_FAIL("exception must not be thrown.");
  }
}

void HttpResponseTest::testValidateResponse_withIfModifiedSince()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);
  httpHeader->setStatusCode(304);
  SharedHandle<HttpRequest> httpRequest(new HttpRequest());
  httpResponse.setHttpRequest(httpRequest);
  try {
    httpResponse.validateResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
  httpRequest->setIfModifiedSinceHeader("Fri, 16 Jul 2010 12:56:59 GMT");
  httpResponse.validateResponse();
}

void HttpResponseTest::testHasRetryAfter()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);

  httpHeader->put("Retry-After", "60");

  CPPUNIT_ASSERT(httpResponse.hasRetryAfter());
  CPPUNIT_ASSERT_EQUAL((time_t)60, httpResponse.getRetryAfter());
}

void HttpResponseTest::testProcessRedirect()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);

  SharedHandle<HttpRequest> httpRequest(new HttpRequest());
  SharedHandle<Request> request(new Request());
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);
  httpResponse.setHttpRequest(httpRequest);
  
  httpHeader->put("Location", "http://mirror/aria2-1.0.0.tar.bz2");
  httpResponse.processRedirect();

  httpHeader->clearField();

  // Test for percent-encode
  httpHeader->put("Location", "http://example.org/white space#aria2");
  httpResponse.processRedirect();
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/white%20space"),
                       request->getCurrentUri());

  httpHeader->clearField();

  // Give unsupported scheme
  httpHeader->put("Location", "unsupported://mirror/aria2-1.0.0.tar.bz2");
  try {
    httpResponse.processRedirect();
    CPPUNIT_FAIL("DlRetryEx exception must be thrown.");
  } catch(DlRetryEx& e) {
    // Success
  } catch(...) {
    CPPUNIT_FAIL("DlRetryEx exception must be thrown.");
  }
}

void HttpResponseTest::testRetrieveCookie()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);

  SharedHandle<HttpRequest> httpRequest(new HttpRequest());
  SharedHandle<Request> request(new Request());
  request->setUri("http://www.aria2.org/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);
  SharedHandle<CookieStorage> st(new CookieStorage());
  httpRequest->setCookieStorage(st);
  httpResponse.setHttpRequest(httpRequest);

  httpHeader->put("Set-Cookie", "k1=v1; expires=Sun, 10-Jun-2007 11:00:00 GMT;"
                  "path=/; domain=.aria2.org;");
  httpHeader->put("Set-Cookie", "k2=v2; expires=Sun, 01-Jan-38 00:00:00 GMT;"
                  "path=/; domain=.aria2.org;");
  httpHeader->put("Set-Cookie", "k3=v3;");

  httpResponse.retrieveCookie();

  CPPUNIT_ASSERT_EQUAL((size_t)2, st->size());

  std::vector<Cookie> cookies;
  st->dumpCookie(std::back_inserter(cookies));
  CPPUNIT_ASSERT_EQUAL(std::string("k2=v2"), cookies[0].toString());
  CPPUNIT_ASSERT_EQUAL(std::string("k3=v3"), cookies[1].toString());
}

void HttpResponseTest::testSupportsPersistentConnection()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);
  SharedHandle<HttpRequest> httpRequest(new HttpRequest());
  httpResponse.setHttpRequest(httpRequest);

  httpHeader->setVersion("HTTP/1.1");
  CPPUNIT_ASSERT(httpResponse.supportsPersistentConnection());
  httpHeader->put("Connection", "close");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->clearField();
  httpHeader->put("Connection", "keep-alive");
  CPPUNIT_ASSERT(httpResponse.supportsPersistentConnection());
  httpHeader->clearField();

  httpHeader->setVersion("HTTP/1.0");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->put("Connection", "close");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->clearField();
  httpHeader->put("Connection", "keep-alive");
  CPPUNIT_ASSERT(httpResponse.supportsPersistentConnection());
  httpHeader->clearField();

  // test proxy connection
  SharedHandle<Request> proxyRequest(new Request());
  httpRequest->setProxyRequest(proxyRequest);
  
  httpHeader->setVersion("HTTP/1.1");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->put("Connection", "close");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->clearField();
  httpHeader->put("Connection", "keep-alive");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->clearField();
  httpHeader->put("Proxy-Connection", "keep-alive");
  CPPUNIT_ASSERT(httpResponse.supportsPersistentConnection());
  httpHeader->put("Connection", "close");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->clearField();
  httpHeader->put("Proxy-Connection", "close");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->clearField();

  httpHeader->setVersion("HTTP/1.0");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->put("Connection", "close");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->clearField();
  httpHeader->put("Connection", "keep-alive");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->put("Proxy-Connection", "keep-alive");
  CPPUNIT_ASSERT(httpResponse.supportsPersistentConnection());
  httpHeader->clearField();
  httpHeader->put("Proxy-Connection", "keep-alive");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->clearField();
  httpHeader->put("Proxy-Connection", "close");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpHeader->clearField();
}

void HttpResponseTest::testGetMetalinKHttpEntries()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);
  SharedHandle<Option> option(new Option());

  httpHeader->put("Link", "<http://uri1/>; rel=duplicate; pri=1; pref; geo=JP");
  httpHeader->put("Link", "<http://uri2/>; rel=duplicate");
  httpHeader->put("Link", "<http://uri3/>;;;;;;;;rel=duplicate;;;;;pri=2;;;;;");
  httpHeader->put("Link", "<http://uri4/>;rel=duplicate;=pri=1;pref");
  httpHeader->put("Link", "<http://describedby>; rel=describedby");
  httpHeader->put("Link", "<http://norel/>");
  httpHeader->put("Link", "<baduri>; rel=duplicate; pri=-1;");
  std::vector<MetalinkHttpEntry> result;
  httpResponse.getMetalinKHttpEntries(result, option);
  CPPUNIT_ASSERT_EQUAL((size_t)5, result.size());

  MetalinkHttpEntry e = result[0];
  CPPUNIT_ASSERT_EQUAL(std::string("http://uri1/"), e.uri);
  CPPUNIT_ASSERT_EQUAL(1, e.pri);
  CPPUNIT_ASSERT(e.pref);
  CPPUNIT_ASSERT_EQUAL(std::string("jp"), e.geo);

  e = result[1];
  CPPUNIT_ASSERT_EQUAL(std::string("http://uri4/"), e.uri);
  CPPUNIT_ASSERT_EQUAL(999999, e.pri);
  CPPUNIT_ASSERT(e.pref);
  CPPUNIT_ASSERT(e.geo.empty());

  e = result[2];
  CPPUNIT_ASSERT_EQUAL(std::string("http://uri3/"), e.uri);
  CPPUNIT_ASSERT_EQUAL(2, e.pri);
  CPPUNIT_ASSERT(!e.pref);
  CPPUNIT_ASSERT(e.geo.empty());

  e = result[3];
  CPPUNIT_ASSERT_EQUAL(std::string("http://uri2/"), e.uri);
  CPPUNIT_ASSERT_EQUAL(999999, e.pri);
  CPPUNIT_ASSERT(!e.pref);
  CPPUNIT_ASSERT(e.geo.empty());

  e = result[4];
  CPPUNIT_ASSERT_EQUAL(std::string("baduri"), e.uri);
  CPPUNIT_ASSERT_EQUAL(999999, e.pri);
  CPPUNIT_ASSERT(!e.pref);
  CPPUNIT_ASSERT(e.geo.empty());
}

#ifdef ENABLE_MESSAGE_DIGEST
void HttpResponseTest::testGetDigest()
{
  HttpResponse httpResponse;
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpResponse.setHttpHeader(httpHeader);
  SharedHandle<Option> option(new Option());
  // Python binascii.hexlify(base64.b64decode(B64ED_HASH)) is handy to
  // retrieve ascii hex hash string.
  httpHeader->put("Digest", "SHA-1=82AD8itGL/oYQ5BTPFANiYnp9oE=");
  httpHeader->put("Digest", "NOT_SUPPORTED");
  httpHeader->put("Digest", "SHA-224=rQdowoLHQJTMVZ3rF7vmYOIzUXlu7F+FcMbPnA==");
  httpHeader->put("Digest", "SHA-224=6Ik6LNZ1iPy6cbmlKO4NHfvxzaiurmHilMyhGA==");
  httpHeader->put("Digest",
                  "SHA-256=+D8nGudz3G/kpkVKQeDrI3xD57v0UeQmzGCZOk03nsU=,"
                  "MD5=LJDK2+9ClF8Nz/K5WZd/+A==");
  std::vector<Checksum> result;
  httpResponse.getDigest(result);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result.size());

  Checksum c = result[0];
  CPPUNIT_ASSERT_EQUAL(std::string("sha-256"), c.getHashType());
  CPPUNIT_ASSERT_EQUAL(std::string("f83f271ae773dc6fe4a6454a41e0eb237c43e7bbf451e426cc60993a4d379ec5"),
                       util::toHex(c.getDigest()));

  c = result[1];
  CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), c.getHashType());
  CPPUNIT_ASSERT_EQUAL(std::string("f36003f22b462ffa184390533c500d8989e9f681"),
                       util::toHex(c.getDigest()));
}
#endif // ENABLE_MESSAGE_DIGEST

} // namespace aria2
