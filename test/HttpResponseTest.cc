#include "HttpResponse.h"
#include "prefs.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class HttpResponseTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HttpResponseTest);
  CPPUNIT_TEST(testGetContentLength_null);
  CPPUNIT_TEST(testGetContentLength_contentLength);
  //CPPUNIT_TEST(testGetContentLength_range);
  CPPUNIT_TEST(testGetEntityLength);
  CPPUNIT_TEST(testDeterminFilename_without_ContentDisposition);
  CPPUNIT_TEST(testDeterminFilename_with_ContentDisposition_zero_length);
  CPPUNIT_TEST(testDeterminFilename_with_ContentDisposition);
  CPPUNIT_TEST(testGetRedirectURI_without_Location);
  CPPUNIT_TEST(testGetRedirectURI_with_Location);
  CPPUNIT_TEST(testIsRedirect);
  CPPUNIT_TEST(testIsTransferEncodingSpecified);
  CPPUNIT_TEST(testGetTransferEncoding);
  CPPUNIT_TEST(testGetTransferDecoder);
  CPPUNIT_TEST(testValidateResponse);
  CPPUNIT_TEST(testValidateResponse_good_range);
  CPPUNIT_TEST(testValidateResponse_bad_range);
  CPPUNIT_TEST(testValidateResponse_chunked);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testGetContentLength_null();
  void testGetContentLength_contentLength();
  void testGetEntityLength();
  void testDeterminFilename_without_ContentDisposition();
  void testDeterminFilename_with_ContentDisposition_zero_length();
  void testDeterminFilename_with_ContentDisposition();
  void testGetRedirectURI_without_Location();
  void testGetRedirectURI_with_Location();
  void testIsRedirect();
  void testIsTransferEncodingSpecified();
  void testGetTransferEncoding();
  void testGetTransferDecoder();
  void testValidateResponse();
  void testValidateResponse_good_range();
  void testValidateResponse_bad_range();
  void testValidateResponse_chunked();
};


CPPUNIT_TEST_SUITE_REGISTRATION( HttpResponseTest );

void HttpResponseTest::testGetContentLength_null()
{
  HttpResponse httpResponse;

  CPPUNIT_ASSERT_EQUAL((int64_t)0, httpResponse.getContentLength());
}

void HttpResponseTest::testGetContentLength_contentLength()
{
  HttpResponse httpResponse;

  HttpHeaderHandle httpHeader = new HttpHeader();
  httpHeader->put("Content-Length", "4294967296");

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT_EQUAL((int64_t)4294967296LL, httpResponse.getContentLength());
}

void HttpResponseTest::testGetEntityLength()
{
  HttpResponse httpResponse;

  HttpHeaderHandle httpHeader = new HttpHeader();
  httpHeader->put("Content-Length", "4294967296");

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT_EQUAL((int64_t)4294967296LL, httpResponse.getEntityLength());

  httpHeader->put("Content-Range", "bytes 1-4294967296/4294967297");

  CPPUNIT_ASSERT_EQUAL((int64_t)4294967297LL, httpResponse.getEntityLength());

}

void HttpResponseTest::testDeterminFilename_without_ContentDisposition()
{
  HttpResponse httpResponse;
  HttpHeaderHandle httpHeader = new HttpHeader();
  HttpRequestHandle httpRequest = new HttpRequest();
  RequestHandle request = new Request();
  request->setUrl("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);

  httpResponse.setHttpHeader(httpHeader);
  httpResponse.setHttpRequest(httpRequest);

  CPPUNIT_ASSERT_EQUAL(string("aria2-1.0.0.tar.bz2"),
		       httpResponse.determinFilename());
}

void HttpResponseTest::testDeterminFilename_with_ContentDisposition_zero_length()
{
  HttpResponse httpResponse;
  HttpHeaderHandle httpHeader = new HttpHeader();
  httpHeader->put("Content-Disposition", "attachment; filename=\"\"");
  HttpRequestHandle httpRequest = new HttpRequest();
  RequestHandle request = new Request();
  request->setUrl("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);

  httpResponse.setHttpHeader(httpHeader);
  httpResponse.setHttpRequest(httpRequest);

  CPPUNIT_ASSERT_EQUAL(string("aria2-1.0.0.tar.bz2"),
		       httpResponse.determinFilename());
}

void HttpResponseTest::testDeterminFilename_with_ContentDisposition()
{
  HttpResponse httpResponse;
  HttpHeaderHandle httpHeader = new HttpHeader();
  httpHeader->put("Content-Disposition", "attachment; filename=\"aria2-current.tar.bz2\"");
  HttpRequestHandle httpRequest = new HttpRequest();
  RequestHandle request = new Request();
  request->setUrl("http://localhost/archives/aria2-1.0.0.tar.bz2");
  httpRequest->setRequest(request);

  httpResponse.setHttpHeader(httpHeader);
  httpResponse.setHttpRequest(httpRequest);

  CPPUNIT_ASSERT_EQUAL(string("aria2-current.tar.bz2"),
		       httpResponse.determinFilename());
}

void HttpResponseTest::testGetRedirectURI_without_Location()
{
  HttpResponse httpResponse;
  HttpHeaderHandle httpHeader = new HttpHeader();

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT_EQUAL(string(""),
		       httpResponse.getRedirectURI());  
}

void HttpResponseTest::testGetRedirectURI_with_Location()
{
  HttpResponse httpResponse;
  HttpHeaderHandle httpHeader = new HttpHeader();
  httpHeader->put("Location", "http://localhost/download/aria2-1.0.0.tar.bz2");
  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT_EQUAL(string("http://localhost/download/aria2-1.0.0.tar.bz2"),
		       httpResponse.getRedirectURI());
}

void HttpResponseTest::testIsRedirect()
{
  HttpResponse httpResponse;
  HttpHeaderHandle httpHeader = new HttpHeader();
  httpHeader->put("Location", "http://localhost/download/aria2-1.0.0.tar.bz2");

  httpResponse.setHttpHeader(httpHeader);
  httpResponse.setStatus(200);

  CPPUNIT_ASSERT(!httpResponse.isRedirect());

  httpResponse.setStatus(304);

  CPPUNIT_ASSERT(httpResponse.isRedirect());  
}

void HttpResponseTest::testIsTransferEncodingSpecified()
{
  HttpResponse httpResponse;
  HttpHeaderHandle httpHeader = new HttpHeader();

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT(!httpResponse.isTransferEncodingSpecified());  

  httpHeader->put("Transfer-Encoding", "chunked");

  CPPUNIT_ASSERT(httpResponse.isTransferEncodingSpecified());
}

void HttpResponseTest::testGetTransferEncoding()
{
  HttpResponse httpResponse;
  HttpHeaderHandle httpHeader = new HttpHeader();

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT_EQUAL(string(""), httpResponse.getTransferEncoding());  

  httpHeader->put("Transfer-Encoding", "chunked");

  CPPUNIT_ASSERT_EQUAL(string("chunked"), httpResponse.getTransferEncoding());
}

void HttpResponseTest::testGetTransferDecoder()
{
  HttpResponse httpResponse;
  HttpHeaderHandle httpHeader = new HttpHeader();

  httpResponse.setHttpHeader(httpHeader);

  CPPUNIT_ASSERT(httpResponse.getTransferDecoder().isNull());  

  httpHeader->put("Transfer-Encoding", "chunked");

  CPPUNIT_ASSERT(!httpResponse.getTransferDecoder().isNull());
}

void HttpResponseTest::testValidateResponse()
{
  HttpResponse httpResponse;

  httpResponse.setStatus(401);

  try {
    httpResponse.validateResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    delete e;
  }

  httpResponse.setStatus(505);

  try {
    httpResponse.validateResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    delete e;
  }

  httpResponse.setStatus(304);
  HttpHeaderHandle httpHeader = new HttpHeader();
  httpResponse.setHttpHeader(httpHeader);
  try {
    httpResponse.validateResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    delete e;
  }

  httpHeader->put("Location", "http://localhost/archives/aria2-1.0.0.tar.bz2");

  try {
    httpResponse.validateResponse();
  } catch(Exception* e) {
    delete e;
    CPPUNIT_FAIL("exception must not be thrown.");
  }
}
 
void HttpResponseTest::testValidateResponse_good_range()
{
  HttpResponse httpResponse;
  HttpHeaderHandle httpHeader = new HttpHeader();
  httpResponse.setHttpHeader(httpHeader);

  HttpRequestHandle httpRequest = new HttpRequest();
  SegmentHandle segment = new Segment();
  segment->index = 1;
  segment->length = 1024*1024;
  segment->segmentLength = 1024*1024;
  segment->writtenLength = 0;
  httpRequest->setSegment(segment);
  RequestHandle request = new Request();
  request->setUrl("http://localhost/archives/aria2-1.0.0.tar.bz2");
  request->setKeepAlive(false);
  httpRequest->setRequest(request);
  httpResponse.setHttpRequest(httpRequest);
  httpResponse.setStatus(206);
  httpHeader->put("Content-Range", "bytes 1048576-10485760/10485761");
  
  try {
    httpResponse.validateResponse();
  } catch(Exception* e) {
    cerr << e->getMsg() << endl;
    delete e;
    CPPUNIT_FAIL("exception must not be thrown.");
  }
}

void HttpResponseTest::testValidateResponse_bad_range()
{
  HttpResponse httpResponse;
  HttpHeaderHandle httpHeader = new HttpHeader();
  httpResponse.setHttpHeader(httpHeader);

  HttpRequestHandle httpRequest = new HttpRequest();
  SegmentHandle segment = new Segment();
  segment->index = 1;
  segment->length = 1024*1024;
  segment->segmentLength = 1024*1024;
  segment->writtenLength = 0;
  httpRequest->setSegment(segment);
  RequestHandle request = new Request();
  request->setUrl("http://localhost/archives/aria2-1.0.0.tar.bz2");
  request->setKeepAlive(false);
  httpRequest->setRequest(request);
  httpResponse.setHttpRequest(httpRequest);
  httpResponse.setStatus(206);
  httpHeader->put("Content-Range", "bytes 0-10485760/10485761");

  try {
    httpResponse.validateResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    delete e;
  }
}

void HttpResponseTest::testValidateResponse_chunked()
{
  HttpResponse httpResponse;
  HttpHeaderHandle httpHeader = new HttpHeader();
  httpResponse.setHttpHeader(httpHeader);

  HttpRequestHandle httpRequest = new HttpRequest();
  SegmentHandle segment = new Segment();
  segment->index = 1;
  segment->length = 1024*1024;
  segment->segmentLength = 1024*1024;
  segment->writtenLength = 0;
  httpRequest->setSegment(segment);
  RequestHandle request = new Request();
  request->setUrl("http://localhost/archives/aria2-1.0.0.tar.bz2");
  request->setKeepAlive(false);
  httpRequest->setRequest(request);
  httpResponse.setHttpRequest(httpRequest);
  httpResponse.setStatus(206);
  httpHeader->put("Content-Range", "bytes 0-10485760/10485761");
  httpHeader->put("Transfer-Encoding", "chunked");

  // if transfer-encoding is specified, then range validation is skipped.
  try {
    httpResponse.validateResponse();
  } catch(Exception* e) {
    delete e;
    CPPUNIT_FAIL("exception must not be thrown.");
  }
}
