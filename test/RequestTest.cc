#include "Request.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Netrc.h"
#include "DefaultAuthResolver.h"
#include "NetrcAuthResolver.h"

namespace aria2 {

class RequestTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestTest);
  CPPUNIT_TEST(testSetUrl1);
  CPPUNIT_TEST(testSetUrl2);
  CPPUNIT_TEST(testSetUrl3);
  CPPUNIT_TEST(testSetUrl4);
  CPPUNIT_TEST(testSetUrl5);
  CPPUNIT_TEST(testSetUrl6);
  CPPUNIT_TEST(testSetUrl7);
  CPPUNIT_TEST(testSetUrl8);
  CPPUNIT_TEST(testSetUrl9);
  CPPUNIT_TEST(testSetUrl10);
  CPPUNIT_TEST(testSetUrl11);
  CPPUNIT_TEST(testSetUrl12);
  CPPUNIT_TEST(testSetUrl13);
  CPPUNIT_TEST(testSetUrl14);
  CPPUNIT_TEST(testSetUrl15);
  CPPUNIT_TEST(testSetUrl16);
  CPPUNIT_TEST(testSetUrl17);
  CPPUNIT_TEST(testSetUrl18);
  CPPUNIT_TEST(testSetUrl19);
  CPPUNIT_TEST(testSetUrl20);
  CPPUNIT_TEST(testSetUrl_username);
  CPPUNIT_TEST(testSetUrl_usernamePassword);
  CPPUNIT_TEST(testSetUrl_zeroUsername);
  CPPUNIT_TEST(testSetUrl_supportsPersistentConnection);
  CPPUNIT_TEST(testSetUrl_ipv6);
  CPPUNIT_TEST(testRedirectUrl);
  CPPUNIT_TEST(testRedirectUrl2);
  CPPUNIT_TEST(testRedirectUrl_supportsPersistentConnection);
  CPPUNIT_TEST(testResetUrl);
  CPPUNIT_TEST(testResetUrl_supportsPersistentConnection);
  CPPUNIT_TEST(testInnerLink);
  CPPUNIT_TEST(testInnerLinkInReferer);
  CPPUNIT_TEST(testGetURIHost);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testSetUrl1();
  void testSetUrl2();
  void testSetUrl3();
  void testSetUrl4();
  void testSetUrl5();
  void testSetUrl6();
  void testSetUrl7();
  void testSetUrl8();
  void testSetUrl9();
  void testSetUrl10();
  void testSetUrl11();
  void testSetUrl12();
  void testSetUrl13();
  void testSetUrl14();
  void testSetUrl15();
  void testSetUrl16();
  void testSetUrl17();
  void testSetUrl18();
  void testSetUrl19();
  void testSetUrl20();
  void testSetUrl_username();
  void testSetUrl_usernamePassword();
  void testSetUrl_zeroUsername();
  void testSetUrl_supportsPersistentConnection();
  void testSetUrl_ipv6();
  void testRedirectUrl();
  void testRedirectUrl2();
  void testRedirectUrl_supportsPersistentConnection();
  void testResetUrl();
  void testResetUrl_supportsPersistentConnection();
  void testInnerLink();
  void testInnerLinkInReferer();
  void testGetURIHost();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestTest );

void RequestTest::testSetUrl1() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/"), req.getUrl());
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/"), req.getCurrentUrl());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPreviousUrl());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPassword());
  CPPUNIT_ASSERT(!req.isIPv6LiteralAddress());
}

void RequestTest::testSetUrl2() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com:8080/index.html");
  req.setReferer("http://aria.rednoah.com:8080");

  CPPUNIT_ASSERT(v);

  // referer is unchaged
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com:8080"), req.getReferer());
  // previousUrl must equal to referer;
  CPPUNIT_ASSERT_EQUAL(req.getReferer(), req.getPreviousUrl());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)8080, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUrl3() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/aria2/index.html");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUrl4() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/aria2/aria3/index.html");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2/aria3"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUrl5() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/aria2/aria3/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2/aria3"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUrl6() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/aria2/aria3");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("aria3"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUrl7() {
  Request req;
  bool v = req.setUrl("http://");

  CPPUNIT_ASSERT(!v);
}

void RequestTest::testSetUrl8() {
  Request req;
  bool v = req.setUrl("http:/aria.rednoah.com");

  CPPUNIT_ASSERT(!v);
}

void RequestTest::testSetUrl9() {
  Request req;
  bool v = req.setUrl("h");

  CPPUNIT_ASSERT(!v);
}

void RequestTest::testSetUrl10() {
  Request req;
  bool v = req.setUrl("");

  CPPUNIT_ASSERT(!v);
}

void RequestTest::testSetUrl11() {
  Request req;
  bool v = req.setUrl("http://host?query/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("?query/"), req.getQuery());
}

void RequestTest::testSetUrl12() {
  Request req;
  bool v = req.setUrl("http://host?query");
  
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("?query"), req.getQuery());
}

void RequestTest::testSetUrl13() {
  Request req;
  bool v = req.setUrl("http://host/?query");
  
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("?query"), req.getQuery());
}

void RequestTest::testSetUrl14() {
  Request req;
  bool v = req.setUrl("http://host:8080/abc?query");
  
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL((uint16_t)8080, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("abc"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("?query"), req.getQuery());
}

void RequestTest::testSetUrl15()
{
  Request req;
  // 2 slashes after host name and dir
  bool v = req.setUrl("http://host//dir1/dir2//file");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/dir1/dir2"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("file"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUrl16()
{
  Request req;
  // 2 slashes before file
  bool v = req.setUrl("http://host//file");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("file"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUrl17()
{
  Request req;
  bool v = req.setUrl("http://host:80/file<with%2 %20space/file with space;param%?a=/?");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/file%3Cwith%252%20%20space"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("file%20with%20space;param%25"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("?a=/?"), req.getQuery());
  CPPUNIT_ASSERT_EQUAL(std::string("http://host:80/file%3Cwith%252%20%20space"
                                   "/file%20with%20space;param%25?a=/?"),
                       req.getCurrentUrl());
  CPPUNIT_ASSERT_EQUAL(std::string("http://host:80/file<with%2 %20space"
                                   "/file with space;param%?a=/?"),
                       req.getUrl());
}

void RequestTest::testSetUrl18() {
  Request req;
  bool v = req.setUrl("http://1/");

  CPPUNIT_ASSERT(v);
}

void RequestTest::testSetUrl19() {
  Request req;
  // No host
  bool v = req.setUrl("http://user@");

  CPPUNIT_ASSERT(!v);
}

void RequestTest::testSetUrl20() {
  Request req;
  bool v;
  // Invalid port
  v = req.setUrl("http://localhost:65536");
  CPPUNIT_ASSERT(!v);
  v = req.setUrl("http://localhost:65535");
  CPPUNIT_ASSERT(v);
  v = req.setUrl("http://localhost:-80");
  CPPUNIT_ASSERT(!v);
}

void RequestTest::testRedirectUrl() {
  Request req;
  req.supportsPersistentConnection(false);
  req.setUrl("http://aria.rednoah.com:8080/aria2/index.html");
  
  bool v2 = req.redirectUrl("http://aria.rednoah.co.jp/");
  CPPUNIT_ASSERT(v2);
  // persistent connection flag is set to be true after redirection
  CPPUNIT_ASSERT(req.supportsPersistentConnection());
  // url must be the same
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com:8080/aria2/"
                                   "index.html"),
                       req.getUrl());
  // currentUrl must be updated
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.co.jp/"),
                       req.getCurrentUrl());
  // previousUrl must be "" when redirection
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPreviousUrl());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.co.jp"), req.getHost());
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
  // See redirect count is incremented.
  CPPUNIT_ASSERT_EQUAL((unsigned int)1, req.getRedirectCount());

  // Give abosulute path
  CPPUNIT_ASSERT(req.redirectUrl("/abspath/to/file"));
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.co.jp/abspath/to/file"),
                       req.getCurrentUrl());
  CPPUNIT_ASSERT_EQUAL((unsigned int)2, req.getRedirectCount());

  // Give relative path
  CPPUNIT_ASSERT(req.redirectUrl("relativepath/to/file"));
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.co.jp/abspath/to/"
                                   "relativepath/to/file"),
                       req.getCurrentUrl());
  CPPUNIT_ASSERT_EQUAL((unsigned int)3, req.getRedirectCount());

  // White space in path
  CPPUNIT_ASSERT(req.redirectUrl("http://example.org/white space"));
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/white%20space"),
                       req.getCurrentUrl());
}

void RequestTest::testRedirectUrl2() {
  Request req;
  req.setUrl("http://aria.rednoah.com/download.html");
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPreviousUrl());
  req.setReferer("http://aria.rednoah.com/");
  // previousUrl is updated when referer is specified
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/"), req.getPreviousUrl());
  req.redirectUrl("http://aria.rednoah.com/403.html");

  // previousUrl must be "" when redirection
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPreviousUrl());
  // referer is unchagned
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/"), req.getReferer());

  req.redirectUrl("http://aria.rednoah.com/error.html");

  // previousUrl must be "" when redirection
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPreviousUrl());
}
  
void RequestTest::testResetUrl() {
  Request req;
  req.setUrl("http://aria.rednoah.com:8080/aria2/index.html");
  req.setReferer("http://aria.rednoah.com:8080/");
  req.redirectUrl("ftp://aria.rednoah.co.jp/index_en.html?view=wide");

  bool v3 = req.resetUrl();
  CPPUNIT_ASSERT(v3);
  // currentUrl must equal to url
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com:8080/aria2/index.html"), req.getUrl());
  CPPUNIT_ASSERT_EQUAL(req.getUrl(), req.getCurrentUrl());
  // previousUrl must equal to referer
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com:8080/"), req.getPreviousUrl());
  // referer is unchanged
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com:8080/"), req.getReferer());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)8080, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testInnerLink() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/index.html#download");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/index.html"
                                   "#download"),
                       req.getUrl());
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/index.html"),
                       req.getCurrentUrl());
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testInnerLinkInReferer() {
  Request req;
  req.setReferer("http://aria.rednoah.com/home.html#top");
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/home.html"),
                       req.getReferer());
}

void RequestTest::testSetUrl_zeroUsername()
{
  Request req;
  CPPUNIT_ASSERT(req.setUrl("ftp://@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/download"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPassword());

  CPPUNIT_ASSERT(req.setUrl("ftp://:@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/download"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPassword());

  CPPUNIT_ASSERT(req.setUrl("ftp://:pass@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/download"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string("pass"), req.getPassword());

}

void RequestTest::testSetUrl_username()
{
  Request req;
  CPPUNIT_ASSERT(req.setUrl("ftp://aria2user@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/download"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2user"), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPassword());
}

void RequestTest::testSetUrl_usernamePassword()
{
  Request req;
  CPPUNIT_ASSERT(req.setUrl("ftp://aria2user%40:aria2pass%40@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/download"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2user@"), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2pass@"), req.getPassword());

  // make sure that after new url is set, username and password are updated.
  CPPUNIT_ASSERT(req.setUrl("ftp://localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPassword());

}

void RequestTest::testSetUrl_supportsPersistentConnection()
{
  Request req;
  CPPUNIT_ASSERT(req.setUrl("http://host/file"));
  req.supportsPersistentConnection(false);
  CPPUNIT_ASSERT(!req.supportsPersistentConnection());
  req.setUrl("http://host/file");
  CPPUNIT_ASSERT(req.supportsPersistentConnection());
}

void RequestTest::testResetUrl_supportsPersistentConnection()
{
  Request req;
  CPPUNIT_ASSERT(req.setUrl("http://host/file"));
  req.supportsPersistentConnection(false);
  CPPUNIT_ASSERT(!req.supportsPersistentConnection());
  req.resetUrl();
  CPPUNIT_ASSERT(req.supportsPersistentConnection());
}

void RequestTest::testRedirectUrl_supportsPersistentConnection()
{
  Request req;
  CPPUNIT_ASSERT(req.setUrl("http://host/file"));
  req.supportsPersistentConnection(false);
  CPPUNIT_ASSERT(!req.supportsPersistentConnection());
  req.redirectUrl("http://host/file");
  CPPUNIT_ASSERT(req.supportsPersistentConnection());
}

void RequestTest::testSetUrl_ipv6()
{
  Request req;
  CPPUNIT_ASSERT(!req.setUrl("http://[::1"));
  CPPUNIT_ASSERT(req.setUrl("http://[::1]"));
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), req.getHost());

  CPPUNIT_ASSERT(req.setUrl("http://[::1]:8000/dir/file"));
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), req.getHost());
  CPPUNIT_ASSERT_EQUAL((uint16_t)8000, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("/dir"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("file"), req.getFile());
  CPPUNIT_ASSERT(req.isIPv6LiteralAddress());
}

void RequestTest::testGetURIHost()
{
  Request req;
  CPPUNIT_ASSERT(req.setUrl("http://[::1]"));
  CPPUNIT_ASSERT_EQUAL(std::string("[::1]"), req.getURIHost());
}

} // namespace aria2
