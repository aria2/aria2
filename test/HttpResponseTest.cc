#include "HttpResponse.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
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
  // CPPUNIT_TEST(testGetContentLength_range);
  CPPUNIT_TEST(testGetEntityLength);
  CPPUNIT_TEST(testGetContentType);
  CPPUNIT_TEST(testDetermineFilename_without_ContentDisposition);
  CPPUNIT_TEST(testDetermineFilename_with_ContentDisposition_zero_length);
  CPPUNIT_TEST(testDetermineFilename_with_ContentDisposition);
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
  CPPUNIT_TEST(testProcessRedirect);
  CPPUNIT_TEST(testRetrieveCookie);
  CPPUNIT_TEST(testSupportsPersistentConnection);
  CPPUNIT_TEST(testGetMetalinKHttpEntries);
  CPPUNIT_TEST(testGetDigest);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testGetContentLength_null();
  void testGetContentLength_contentLength();
  void testGetEntityLength();
  void testGetContentType();
  void testDetermineFilename_without_ContentDisposition();
  void testDetermineFilename_with_ContentDisposition_zero_length();
  void testDetermineFilename_with_ContentDisposition();
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
  void testProcessRedirect();
  void testRetrieveCookie();
  void testSupportsPersistentConnection();
  void testGetMetalinKHttpEntries();
  void testGetDigest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(HttpResponseTest);

void HttpResponseTest::testGetContentLength_null()
{
  HttpResponse httpResponse;

  CPPUNIT_ASSERT_EQUAL((int64_t)0LL, httpResponse.getContentLength());
}

void HttpResponseTest::testGetContentLength_contentLength()
{
  HttpResponse httpResponse;

  auto httpHeader = make_unique<HttpHeader>();
  httpHeader->put(HttpHeader::CONTENT_LENGTH, "4294967296");

  httpResponse.setHttpHeader(std::move(httpHeader));

  CPPUNIT_ASSERT_EQUAL((int64_t)4294967296LL, httpResponse.getContentLength());
}

void HttpResponseTest::testGetEntityLength()
{
  HttpResponse httpResponse;

  auto httpHeader = make_unique<HttpHeader>();
  httpHeader->put(HttpHeader::CONTENT_LENGTH, "4294967296");

  httpResponse.setHttpHeader(std::move(httpHeader));
  CPPUNIT_ASSERT_EQUAL((int64_t)4294967296LL, httpResponse.getEntityLength());

  httpResponse.getHttpHeader()->put(HttpHeader::CONTENT_RANGE,
                                    "bytes 1-4294967296/4294967297");
  CPPUNIT_ASSERT_EQUAL((int64_t)4294967297LL, httpResponse.getEntityLength());
}

void HttpResponseTest::testGetContentType()
{
  HttpResponse httpResponse;
  auto httpHeader = make_unique<HttpHeader>();
  httpHeader->put(HttpHeader::CONTENT_TYPE,
                  "application/metalink+xml; charset=UTF-8");
  httpResponse.setHttpHeader(std::move(httpHeader));
  // See parameter is ignored.
  CPPUNIT_ASSERT_EQUAL(std::string("application/metalink+xml"),
                       httpResponse.getContentType());
}

void HttpResponseTest::testDetermineFilename_without_ContentDisposition()
{
  HttpResponse httpResponse;
  auto httpRequest = make_unique<HttpRequest>();
  auto request = std::make_shared<Request>();
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);

  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  httpResponse.setHttpRequest(std::move(httpRequest));

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"),
                       httpResponse.determineFilename(false));
}

void HttpResponseTest::
    testDetermineFilename_with_ContentDisposition_zero_length()
{
  HttpResponse httpResponse;
  auto httpHeader = make_unique<HttpHeader>();
  httpHeader->put(HttpHeader::CONTENT_DISPOSITION, "attachment; filename=\"\"");
  auto httpRequest = make_unique<HttpRequest>();
  auto request = std::make_shared<Request>();
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);

  httpResponse.setHttpHeader(std::move(httpHeader));
  httpResponse.setHttpRequest(std::move(httpRequest));

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"),
                       httpResponse.determineFilename(false));
}

void HttpResponseTest::testDetermineFilename_with_ContentDisposition()
{
  HttpResponse httpResponse;
  auto httpHeader = make_unique<HttpHeader>();
  httpHeader->put(HttpHeader::CONTENT_DISPOSITION,
                  "attachment; filename=\"aria2-current.tar.bz2\"");
  auto httpRequest = make_unique<HttpRequest>();
  auto request = std::make_shared<Request>();
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);

  httpResponse.setHttpHeader(std::move(httpHeader));
  httpResponse.setHttpRequest(std::move(httpRequest));

  CPPUNIT_ASSERT_EQUAL(std::string("aria2-current.tar.bz2"),
                       httpResponse.determineFilename(false));
}

void HttpResponseTest::testGetRedirectURI_without_Location()
{
  HttpResponse httpResponse;

  httpResponse.setHttpHeader(make_unique<HttpHeader>());

  CPPUNIT_ASSERT_EQUAL(std::string(""), httpResponse.getRedirectURI());
}

void HttpResponseTest::testGetRedirectURI_with_Location()
{
  HttpResponse httpResponse;
  auto httpHeader = make_unique<HttpHeader>();
  httpHeader->put(HttpHeader::LOCATION,
                  "http://localhost/download/aria2-1.0.0.tar.bz2");
  httpResponse.setHttpHeader(std::move(httpHeader));

  CPPUNIT_ASSERT_EQUAL(
      std::string("http://localhost/download/aria2-1.0.0.tar.bz2"),
      httpResponse.getRedirectURI());
}

void HttpResponseTest::testIsRedirect()
{
  HttpResponse httpResponse;
  httpResponse.setHttpHeader(make_unique<HttpHeader>());

  httpResponse.getHttpHeader()->setStatusCode(301);
  CPPUNIT_ASSERT(!httpResponse.isRedirect());

  httpResponse.getHttpHeader()->setStatusCode(200);
  CPPUNIT_ASSERT(!httpResponse.isRedirect());

  httpResponse.getHttpHeader()->put(
      HttpHeader::LOCATION, "http://localhost/download/aria2-1.0.0.tar.bz2");

  CPPUNIT_ASSERT(!httpResponse.isRedirect());

  httpResponse.getHttpHeader()->setStatusCode(300);
  CPPUNIT_ASSERT(httpResponse.isRedirect());

  httpResponse.getHttpHeader()->setStatusCode(301);
  CPPUNIT_ASSERT(httpResponse.isRedirect());

  httpResponse.getHttpHeader()->setStatusCode(302);
  CPPUNIT_ASSERT(httpResponse.isRedirect());

  httpResponse.getHttpHeader()->setStatusCode(303);
  CPPUNIT_ASSERT(httpResponse.isRedirect());

  httpResponse.getHttpHeader()->setStatusCode(304);
  CPPUNIT_ASSERT(!httpResponse.isRedirect());

  httpResponse.getHttpHeader()->setStatusCode(305);
  CPPUNIT_ASSERT(!httpResponse.isRedirect());

  httpResponse.getHttpHeader()->setStatusCode(306);
  CPPUNIT_ASSERT(!httpResponse.isRedirect());

  httpResponse.getHttpHeader()->setStatusCode(307);
  CPPUNIT_ASSERT(httpResponse.isRedirect());

  httpResponse.getHttpHeader()->setStatusCode(308);
  CPPUNIT_ASSERT(httpResponse.isRedirect());

  httpResponse.getHttpHeader()->setStatusCode(309);
  CPPUNIT_ASSERT(!httpResponse.isRedirect());
}

void HttpResponseTest::testIsTransferEncodingSpecified()
{
  HttpResponse httpResponse;

  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  CPPUNIT_ASSERT(!httpResponse.isTransferEncodingSpecified());

  httpResponse.getHttpHeader()->put(HttpHeader::TRANSFER_ENCODING, "chunked");
  CPPUNIT_ASSERT(httpResponse.isTransferEncodingSpecified());
}

void HttpResponseTest::testGetTransferEncoding()
{
  HttpResponse httpResponse;

  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  CPPUNIT_ASSERT_EQUAL(std::string(""), httpResponse.getTransferEncoding());

  httpResponse.getHttpHeader()->put(HttpHeader::TRANSFER_ENCODING, "chunked");
  CPPUNIT_ASSERT_EQUAL(std::string("chunked"),
                       httpResponse.getTransferEncoding());
}

void HttpResponseTest::testGetTransferEncodingStreamFilter()
{
  HttpResponse httpResponse;

  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  CPPUNIT_ASSERT(!httpResponse.getTransferEncodingStreamFilter());

  httpResponse.getHttpHeader()->put(HttpHeader::TRANSFER_ENCODING, "chunked");
  CPPUNIT_ASSERT(httpResponse.getTransferEncodingStreamFilter());
}

void HttpResponseTest::testIsContentEncodingSpecified()
{
  HttpResponse httpResponse;

  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  CPPUNIT_ASSERT(!httpResponse.isContentEncodingSpecified());

  httpResponse.getHttpHeader()->put(HttpHeader::CONTENT_ENCODING, "gzip");
  CPPUNIT_ASSERT(httpResponse.isContentEncodingSpecified());
}

void HttpResponseTest::testGetContentEncoding()
{
  HttpResponse httpResponse;

  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  CPPUNIT_ASSERT_EQUAL(A2STR::NIL, httpResponse.getContentEncoding());

  httpResponse.getHttpHeader()->put(HttpHeader::CONTENT_ENCODING, "gzip");
  CPPUNIT_ASSERT_EQUAL(std::string("gzip"), httpResponse.getContentEncoding());
}

void HttpResponseTest::testGetContentEncodingStreamFilter()
{
  HttpResponse httpResponse;

  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  CPPUNIT_ASSERT(!httpResponse.getContentEncodingStreamFilter());

#ifdef HAVE_ZLIB
  httpResponse.getHttpHeader()->put(HttpHeader::CONTENT_ENCODING, "gzip");
  {
    std::shared_ptr<StreamFilter> filter =
        httpResponse.getContentEncodingStreamFilter();
    CPPUNIT_ASSERT(filter);
    CPPUNIT_ASSERT_EQUAL(std::string("GZipDecodingStreamFilter"),
                         filter->getName());
  }
  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  httpResponse.getHttpHeader()->put(HttpHeader::CONTENT_ENCODING, "deflate");
  {
    std::shared_ptr<StreamFilter> filter =
        httpResponse.getContentEncodingStreamFilter();
    CPPUNIT_ASSERT(filter);
    CPPUNIT_ASSERT_EQUAL(std::string("GZipDecodingStreamFilter"),
                         filter->getName());
  }
#endif // HAVE_ZLIB
  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  httpResponse.getHttpHeader()->put(HttpHeader::CONTENT_ENCODING, "bzip2");
  {
    std::shared_ptr<StreamFilter> filter =
        httpResponse.getContentEncodingStreamFilter();
    CPPUNIT_ASSERT(!filter);
  }
}

void HttpResponseTest::testValidateResponse()
{
  HttpResponse httpResponse;
  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  httpResponse.getHttpHeader()->setStatusCode(301);

  try {
    httpResponse.validateResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    // success
  }

  httpResponse.getHttpHeader()->put(HttpHeader::LOCATION, "http://a/b");
  httpResponse.validateResponse();

  httpResponse.getHttpHeader()->setStatusCode(201);
  try {
    httpResponse.validateResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    // success
  }
}

void HttpResponseTest::testValidateResponse_good_range()
{
  HttpResponse httpResponse;

  httpResponse.setHttpHeader(make_unique<HttpHeader>());

  auto httpRequest = make_unique<HttpRequest>();
  auto p = std::make_shared<Piece>(1, 1_m);
  auto segment = std::make_shared<PiecedSegment>(1_m, p);
  httpRequest->setSegment(segment);
  auto fileEntry = std::make_shared<FileEntry>("file", 10_m, 0);
  httpRequest->setFileEntry(fileEntry);
  auto request = std::make_shared<Request>();
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);
  httpResponse.setHttpRequest(std::move(httpRequest));
  httpResponse.getHttpHeader()->setStatusCode(206);
  httpResponse.getHttpHeader()->put(HttpHeader::CONTENT_RANGE,
                                    "bytes 1048576-10485760/10485760");

  try {
    httpResponse.validateResponse();
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
    CPPUNIT_FAIL("exception must not be thrown.");
  }
}

void HttpResponseTest::testValidateResponse_bad_range()
{
  HttpResponse httpResponse;

  httpResponse.setHttpHeader(make_unique<HttpHeader>());

  auto httpRequest = make_unique<HttpRequest>();
  auto p = std::make_shared<Piece>(1, 1_m);
  auto segment = std::make_shared<PiecedSegment>(1_m, p);
  httpRequest->setSegment(segment);
  auto fileEntry = std::make_shared<FileEntry>("file", 10_m, 0);
  httpRequest->setFileEntry(fileEntry);
  auto request = std::make_shared<Request>();
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);
  httpResponse.setHttpRequest(std::move(httpRequest));
  httpResponse.getHttpHeader()->setStatusCode(206);
  httpResponse.getHttpHeader()->put(HttpHeader::CONTENT_RANGE,
                                    "bytes 0-10485760/10485761");

  try {
    httpResponse.validateResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
}

void HttpResponseTest::testValidateResponse_chunked()
{
  HttpResponse httpResponse;
  httpResponse.setHttpHeader(make_unique<HttpHeader>());

  auto httpRequest = make_unique<HttpRequest>();
  auto p = std::make_shared<Piece>(1, 1_m);
  auto segment = std::make_shared<PiecedSegment>(1_m, p);
  httpRequest->setSegment(segment);
  auto fileEntry = std::make_shared<FileEntry>("file", 10_m, 0);
  httpRequest->setFileEntry(fileEntry);
  auto request = std::make_shared<Request>();
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);
  httpResponse.setHttpRequest(std::move(httpRequest));
  httpResponse.getHttpHeader()->setStatusCode(206);
  httpResponse.getHttpHeader()->put(HttpHeader::CONTENT_RANGE,
                                    "bytes 0-10485760/10485761");
  httpResponse.getHttpHeader()->put(HttpHeader::TRANSFER_ENCODING, "chunked");

  // if transfer-encoding is specified, then range validation is skipped.
  try {
    httpResponse.validateResponse();
  }
  catch (Exception& e) {
    CPPUNIT_FAIL("exception must not be thrown.");
  }
}

void HttpResponseTest::testValidateResponse_withIfModifiedSince()
{
  HttpResponse httpResponse;
  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  httpResponse.getHttpHeader()->setStatusCode(304);
  auto httpRequest = make_unique<HttpRequest>();
  httpResponse.setHttpRequest(std::move(httpRequest));
  try {
    httpResponse.validateResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  httpRequest = make_unique<HttpRequest>();
  httpRequest->setIfModifiedSinceHeader("Fri, 16 Jul 2010 12:56:59 GMT");
  httpResponse.setHttpRequest(std::move(httpRequest));
  httpResponse.validateResponse();
}

void HttpResponseTest::testProcessRedirect()
{
  HttpResponse httpResponse;

  httpResponse.setHttpHeader(make_unique<HttpHeader>());

  auto httpRequest = make_unique<HttpRequest>();
  auto request = std::make_shared<Request>();
  request->setUri("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);
  httpResponse.setHttpRequest(std::move(httpRequest));

  httpResponse.getHttpHeader()->put(HttpHeader::LOCATION,
                                    "http://mirror/aria2-1.0.0.tar.bz2");
  httpResponse.processRedirect();

  httpResponse.getHttpHeader()->clearField();

  // Test for percent-encode
  httpResponse.getHttpHeader()->put(HttpHeader::LOCATION,
                                    "http://example.org/white space#aria2");
  httpResponse.processRedirect();
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/white%20space"),
                       request->getCurrentUri());

  httpResponse.getHttpHeader()->clearField();

  // Give unsupported scheme
  httpResponse.getHttpHeader()->put(HttpHeader::LOCATION,
                                    "unsupported://mirror/aria2-1.0.0.tar.bz2");
  try {
    httpResponse.processRedirect();
    CPPUNIT_FAIL("DlRetryEx exception must be thrown.");
  }
  catch (DlRetryEx& e) {
    // Success
  }
  catch (...) {
    CPPUNIT_FAIL("DlRetryEx exception must be thrown.");
  }
}

void HttpResponseTest::testRetrieveCookie()
{
  HttpResponse httpResponse;

  httpResponse.setHttpHeader(make_unique<HttpHeader>());

  auto httpRequest = make_unique<HttpRequest>();
  auto request = std::make_shared<Request>();
  request->setUri("http://www.aria2.org/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);
  CookieStorage st;
  httpRequest->setCookieStorage(&st);
  httpResponse.setHttpRequest(std::move(httpRequest));

  httpResponse.getHttpHeader()->put(
      HttpHeader::SET_COOKIE, "k1=v1; expires=Sun, 10-Jun-2007 11:00:00 GMT;"
                              "path=/; domain=.aria2.org;");
  httpResponse.getHttpHeader()->put(
      HttpHeader::SET_COOKIE, "k2=v2; expires=Sun, 01-Jan-38 00:00:00 GMT;"
                              "path=/; domain=.aria2.org;");
  httpResponse.getHttpHeader()->put(HttpHeader::SET_COOKIE, "k3=v3;");

  httpResponse.retrieveCookie();

  CPPUNIT_ASSERT_EQUAL((size_t)2, st.size());

  auto cookies = std::vector<const Cookie*>{};
  st.dumpCookie(std::back_inserter(cookies));
  std::sort(std::begin(cookies), std::end(cookies), CookieSorter());
  CPPUNIT_ASSERT_EQUAL(std::string("k2=v2"), cookies[0]->toString());
  CPPUNIT_ASSERT_EQUAL(std::string("k3=v3"), cookies[1]->toString());
}

void HttpResponseTest::testSupportsPersistentConnection()
{
  HttpResponse httpResponse;
  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  httpResponse.setHttpRequest(make_unique<HttpRequest>());

  httpResponse.getHttpHeader()->setVersion("HTTP/1.1");
  CPPUNIT_ASSERT(httpResponse.supportsPersistentConnection());
  httpResponse.getHttpHeader()->put(HttpHeader::CONNECTION, "close");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpResponse.getHttpHeader()->clearField();
  httpResponse.getHttpHeader()->put(HttpHeader::CONNECTION, "keep-alive");
  CPPUNIT_ASSERT(httpResponse.supportsPersistentConnection());
  httpResponse.getHttpHeader()->clearField();

  httpResponse.getHttpHeader()->setVersion("HTTP/1.0");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpResponse.getHttpHeader()->put(HttpHeader::CONNECTION, "close");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpResponse.getHttpHeader()->clearField();
  httpResponse.getHttpHeader()->put(HttpHeader::CONNECTION, "keep-alive");
  CPPUNIT_ASSERT(httpResponse.supportsPersistentConnection());
  httpResponse.getHttpHeader()->clearField();

  // test proxy connection
  auto httpRequest = make_unique<HttpRequest>();
  httpRequest->setProxyRequest(std::make_shared<Request>());
  httpResponse.setHttpRequest(std::move(httpRequest));

  httpResponse.getHttpHeader()->setVersion("HTTP/1.1");
  CPPUNIT_ASSERT(httpResponse.supportsPersistentConnection());
  httpResponse.getHttpHeader()->put(HttpHeader::CONNECTION, "close");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpResponse.getHttpHeader()->clearField();
  httpResponse.getHttpHeader()->put(HttpHeader::CONNECTION, "keep-alive");
  CPPUNIT_ASSERT(httpResponse.supportsPersistentConnection());
  httpResponse.getHttpHeader()->clearField();

  httpResponse.getHttpHeader()->setVersion("HTTP/1.0");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpResponse.getHttpHeader()->put(HttpHeader::CONNECTION, "close");
  CPPUNIT_ASSERT(!httpResponse.supportsPersistentConnection());
  httpResponse.getHttpHeader()->clearField();
  httpResponse.getHttpHeader()->put(HttpHeader::CONNECTION, "keep-alive");
  CPPUNIT_ASSERT(httpResponse.supportsPersistentConnection());
  httpResponse.getHttpHeader()->clearField();
}

void HttpResponseTest::testGetMetalinKHttpEntries()
{
  HttpResponse httpResponse;
  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  std::shared_ptr<Option> option(new Option());

  httpResponse.getHttpHeader()->put(
      HttpHeader::LINK, "<http://uri1/>; rel=duplicate; pri=1; pref; geo=JP");
  httpResponse.getHttpHeader()->put(HttpHeader::LINK,
                                    "<http://uri2/>; rel=duplicate");
  httpResponse.getHttpHeader()->put(
      HttpHeader::LINK, "<http://uri3/>;;;;;;;;rel=duplicate;;;;;pri=2;;;;;");
  httpResponse.getHttpHeader()->put(HttpHeader::LINK,
                                    "<http://uri4/>;rel=duplicate;=pri=1;pref");
  httpResponse.getHttpHeader()->put(HttpHeader::LINK,
                                    "<http://describedby>; rel=describedby");
  httpResponse.getHttpHeader()->put(HttpHeader::LINK, "<http://norel/>");
  httpResponse.getHttpHeader()->put(HttpHeader::LINK,
                                    "<baduri>; rel=duplicate; pri=-1;");
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

void HttpResponseTest::testGetDigest()
{
  HttpResponse httpResponse;
  httpResponse.setHttpHeader(make_unique<HttpHeader>());
  std::shared_ptr<Option> option(new Option());
  // Python binascii.hexlify(base64.b64decode(B64ED_HASH)) is handy to
  // retrieve ascii hex hash string.
  httpResponse.getHttpHeader()->put(HttpHeader::DIGEST,
                                    "SHA-1=82AD8itGL/oYQ5BTPFANiYnp9oE=");
  httpResponse.getHttpHeader()->put(HttpHeader::DIGEST, "NOT_SUPPORTED");
  httpResponse.getHttpHeader()->put(
      HttpHeader::DIGEST, "SHA-224=rQdowoLHQJTMVZ3rF7vmYOIzUXlu7F+FcMbPnA==");
  httpResponse.getHttpHeader()->put(
      HttpHeader::DIGEST, "SHA-224=6Ik6LNZ1iPy6cbmlKO4NHfvxzaiurmHilMyhGA==");
  httpResponse.getHttpHeader()->put(
      HttpHeader::DIGEST,
      "SHA-256=+D8nGudz3G/kpkVKQeDrI3xD57v0UeQmzGCZOk03nsU=,"
      "MD5=LJDK2+9ClF8Nz/K5WZd/+A==");
  std::vector<Checksum> result;
  httpResponse.getDigest(result);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result.size());

  Checksum c = result[0];
  CPPUNIT_ASSERT_EQUAL(std::string("sha-256"), c.getHashType());
  CPPUNIT_ASSERT_EQUAL(
      std::string(
          "f83f271ae773dc6fe4a6454a41e0eb237c43e7bbf451e426cc60993a4d379ec5"),
      util::toHex(c.getDigest()));

  c = result[1];
  CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), c.getHashType());
  CPPUNIT_ASSERT_EQUAL(std::string("f36003f22b462ffa184390533c500d8989e9f681"),
                       util::toHex(c.getDigest()));
}

} // namespace aria2
