#include "Request.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Netrc.h"
#include "DefaultAuthResolver.h"
#include "NetrcAuthResolver.h"

namespace aria2 {

class RequestTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestTest);
  CPPUNIT_TEST(testSetUri1);
  CPPUNIT_TEST(testSetUri2);
  CPPUNIT_TEST(testSetUri3);
  CPPUNIT_TEST(testSetUri4);
  CPPUNIT_TEST(testSetUri5);
  CPPUNIT_TEST(testSetUri6);
  CPPUNIT_TEST(testSetUri7);
  CPPUNIT_TEST(testSetUri8);
  CPPUNIT_TEST(testSetUri9);
  CPPUNIT_TEST(testSetUri10);
  CPPUNIT_TEST(testSetUri11);
  CPPUNIT_TEST(testSetUri12);
  CPPUNIT_TEST(testSetUri13);
  CPPUNIT_TEST(testSetUri14);
  CPPUNIT_TEST(testSetUri15);
  CPPUNIT_TEST(testSetUri16);
  CPPUNIT_TEST(testSetUri17);
  CPPUNIT_TEST(testSetUri18);
  CPPUNIT_TEST(testSetUri19);
  CPPUNIT_TEST(testSetUri20);
  CPPUNIT_TEST(testSetUri_username);
  CPPUNIT_TEST(testSetUri_usernamePassword);
  CPPUNIT_TEST(testSetUri_zeroUsername);
  CPPUNIT_TEST(testSetUri_supportsPersistentConnection);
  CPPUNIT_TEST(testSetUri_ipv6);
  CPPUNIT_TEST(testRedirectUri);
  CPPUNIT_TEST(testRedirectUri2);
  CPPUNIT_TEST(testRedirectUri_supportsPersistentConnection);
  CPPUNIT_TEST(testResetUri);
  CPPUNIT_TEST(testResetUri_supportsPersistentConnection);
  CPPUNIT_TEST(testInnerLink);
  CPPUNIT_TEST(testInnerLinkInReferer);
  CPPUNIT_TEST(testGetURIHost);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testSetUri1();
  void testSetUri2();
  void testSetUri3();
  void testSetUri4();
  void testSetUri5();
  void testSetUri6();
  void testSetUri7();
  void testSetUri8();
  void testSetUri9();
  void testSetUri10();
  void testSetUri11();
  void testSetUri12();
  void testSetUri13();
  void testSetUri14();
  void testSetUri15();
  void testSetUri16();
  void testSetUri17();
  void testSetUri18();
  void testSetUri19();
  void testSetUri20();
  void testSetUri_username();
  void testSetUri_usernamePassword();
  void testSetUri_zeroUsername();
  void testSetUri_supportsPersistentConnection();
  void testSetUri_ipv6();
  void testRedirectUri();
  void testRedirectUri2();
  void testRedirectUri_supportsPersistentConnection();
  void testResetUri();
  void testResetUri_supportsPersistentConnection();
  void testInnerLink();
  void testInnerLinkInReferer();
  void testGetURIHost();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestTest );

void RequestTest::testSetUri1() {
  Request req;
  bool v = req.setUri("http://aria.rednoah.com/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/"), req.getUri());
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/"), req.getCurrentUri());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPreviousUri());
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

void RequestTest::testSetUri2() {
  Request req;
  bool v = req.setUri("http://aria.rednoah.com:8080/index.html");
  req.setReferer("http://aria.rednoah.com:8080");

  CPPUNIT_ASSERT(v);

  // referer is unchaged
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com:8080"), req.getReferer());
  // previousUri must equal to referer;
  CPPUNIT_ASSERT_EQUAL(req.getReferer(), req.getPreviousUri());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)8080, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUri3() {
  Request req;
  bool v = req.setUri("http://aria.rednoah.com/aria2/index.html");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUri4() {
  Request req;
  bool v = req.setUri("http://aria.rednoah.com/aria2/aria3/index.html");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2/aria3"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUri5() {
  Request req;
  bool v = req.setUri("http://aria.rednoah.com/aria2/aria3/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2/aria3"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUri6() {
  Request req;
  bool v = req.setUri("http://aria.rednoah.com/aria2/aria3");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/aria2"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("aria3"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUri7() {
  Request req;
  bool v = req.setUri("http://");

  CPPUNIT_ASSERT(!v);
}

void RequestTest::testSetUri8() {
  Request req;
  bool v = req.setUri("http:/aria.rednoah.com");

  CPPUNIT_ASSERT(!v);
}

void RequestTest::testSetUri9() {
  Request req;
  bool v = req.setUri("h");

  CPPUNIT_ASSERT(!v);
}

void RequestTest::testSetUri10() {
  Request req;
  bool v = req.setUri("");

  CPPUNIT_ASSERT(!v);
}

void RequestTest::testSetUri11() {
  Request req;
  bool v = req.setUri("http://host?query/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("?query/"), req.getQuery());
}

void RequestTest::testSetUri12() {
  Request req;
  bool v = req.setUri("http://host?query");
  
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("?query"), req.getQuery());
}

void RequestTest::testSetUri13() {
  Request req;
  bool v = req.setUri("http://host/?query");
  
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("?query"), req.getQuery());
}

void RequestTest::testSetUri14() {
  Request req;
  bool v = req.setUri("http://host:8080/abc?query");
  
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL((uint16_t)8080, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("abc"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("?query"), req.getQuery());
}

void RequestTest::testSetUri15()
{
  Request req;
  // 2 slashes after host name and dir
  bool v = req.setUri("http://host//dir1/dir2//file");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/dir1/dir2"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("file"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUri16()
{
  Request req;
  // 2 slashes before file
  bool v = req.setUri("http://host//file");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("file"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testSetUri17()
{
  Request req;
  bool v = req.setUri("http://host:80/file<with%2 %20space/file with space;param%?a=/?");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/file%3Cwith%252%20%20space"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("file%20with%20space;param%25"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("?a=/?"), req.getQuery());
  CPPUNIT_ASSERT_EQUAL(std::string("http://host:80/file%3Cwith%252%20%20space"
                                   "/file%20with%20space;param%25?a=/?"),
                       req.getCurrentUri());
  CPPUNIT_ASSERT_EQUAL(std::string("http://host:80/file<with%2 %20space"
                                   "/file with space;param%?a=/?"),
                       req.getUri());
}

void RequestTest::testSetUri18() {
  Request req;
  bool v = req.setUri("http://1/");

  CPPUNIT_ASSERT(v);
}

void RequestTest::testSetUri19() {
  Request req;
  // No host
  bool v = req.setUri("http://user@");

  CPPUNIT_ASSERT(!v);
}

void RequestTest::testSetUri20() {
  Request req;
  bool v;
  // Invalid port
  v = req.setUri("http://localhost:65536");
  CPPUNIT_ASSERT(!v);
  v = req.setUri("http://localhost:65535");
  CPPUNIT_ASSERT(v);
  v = req.setUri("http://localhost:-80");
  CPPUNIT_ASSERT(!v);
}

void RequestTest::testRedirectUri() {
  Request req;
  req.supportsPersistentConnection(false);
  req.setUri("http://aria.rednoah.com:8080/aria2/index.html");
  
  bool v2 = req.redirectUri("http://aria.rednoah.co.jp/");
  CPPUNIT_ASSERT(v2);
  // persistent connection flag is set to be true after redirection
  CPPUNIT_ASSERT(req.supportsPersistentConnection());
  // uri must be the same
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com:8080/aria2/"
                                   "index.html"),
                       req.getUri());
  // currentUri must be updated
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.co.jp/"),
                       req.getCurrentUri());
  // previousUri must be "" when redirection
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPreviousUri());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(std::string("aria.rednoah.co.jp"), req.getHost());
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
  // See redirect count is incremented.
  CPPUNIT_ASSERT_EQUAL((unsigned int)1, req.getRedirectCount());

  // Give abosulute path
  CPPUNIT_ASSERT(req.redirectUri("/abspath/to/file"));
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.co.jp/abspath/to/file"),
                       req.getCurrentUri());
  CPPUNIT_ASSERT_EQUAL((unsigned int)2, req.getRedirectCount());

  // Give relative path
  CPPUNIT_ASSERT(req.redirectUri("relativepath/to/file"));
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.co.jp/abspath/to/"
                                   "relativepath/to/file"),
                       req.getCurrentUri());
  CPPUNIT_ASSERT_EQUAL((unsigned int)3, req.getRedirectCount());

  // White space in path and fragment is appended.
  CPPUNIT_ASSERT(req.redirectUri("http://example.org/white space#aria2"));
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/white%20space"),
                       req.getCurrentUri());
}

void RequestTest::testRedirectUri2() {
  Request req;
  req.setUri("http://aria.rednoah.com/download.html");
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPreviousUri());
  req.setReferer("http://aria.rednoah.com/");
  // previousUri is updated when referer is specified
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/"), req.getPreviousUri());
  req.redirectUri("http://aria.rednoah.com/403.html");

  // previousUri must be "" when redirection
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPreviousUri());
  // referer is unchagned
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/"), req.getReferer());

  req.redirectUri("http://aria.rednoah.com/error.html");

  // previousUri must be "" when redirection
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPreviousUri());
}
  
void RequestTest::testResetUri() {
  Request req;
  req.setUri("http://aria.rednoah.com:8080/aria2/index.html");
  req.setReferer("http://aria.rednoah.com:8080/");
  req.redirectUri("ftp://aria.rednoah.co.jp/index_en.html?view=wide");

  bool v3 = req.resetUri();
  CPPUNIT_ASSERT(v3);
  // currentUri must equal to uri
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com:8080/aria2/index.html"), req.getUri());
  CPPUNIT_ASSERT_EQUAL(req.getUri(), req.getCurrentUri());
  // previousUri must equal to referer
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com:8080/"), req.getPreviousUri());
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
  bool v = req.setUri("http://aria.rednoah.com/index.html#download");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/index.html"
                                   "#download"),
                       req.getUri());
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/index.html"),
                       req.getCurrentUri());
  CPPUNIT_ASSERT_EQUAL(std::string("index.html"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getQuery());
}

void RequestTest::testInnerLinkInReferer() {
  Request req;
  req.setReferer("http://aria.rednoah.com/home.html#top");
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria.rednoah.com/home.html"),
                       req.getReferer());
}

void RequestTest::testSetUri_zeroUsername()
{
  Request req;
  CPPUNIT_ASSERT(req.setUri("ftp://@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/download"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPassword());

  CPPUNIT_ASSERT(req.setUri("ftp://:@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/download"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPassword());

  CPPUNIT_ASSERT(req.setUri("ftp://:pass@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/download"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string("pass"), req.getPassword());

}

void RequestTest::testSetUri_username()
{
  Request req;
  CPPUNIT_ASSERT(req.setUri("ftp://aria2user@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/download"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2user"), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPassword());
}

void RequestTest::testSetUri_usernamePassword()
{
  Request req;
  CPPUNIT_ASSERT(req.setUri("ftp://aria2user%40:aria2pass%40@localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("/download"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-1.0.0.tar.bz2"), req.getFile());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2user@"), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2pass@"), req.getPassword());

  // make sure that after new uri is set, username and password are updated.
  CPPUNIT_ASSERT(req.setUri("ftp://localhost/download/aria2-1.0.0.tar.bz2"));
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getUsername());
  CPPUNIT_ASSERT_EQUAL(std::string(""), req.getPassword());

}

void RequestTest::testSetUri_supportsPersistentConnection()
{
  Request req;
  CPPUNIT_ASSERT(req.setUri("http://host/file"));
  req.supportsPersistentConnection(false);
  CPPUNIT_ASSERT(!req.supportsPersistentConnection());
  req.setUri("http://host/file");
  CPPUNIT_ASSERT(req.supportsPersistentConnection());
}

void RequestTest::testResetUri_supportsPersistentConnection()
{
  Request req;
  CPPUNIT_ASSERT(req.setUri("http://host/file"));
  req.supportsPersistentConnection(false);
  CPPUNIT_ASSERT(!req.supportsPersistentConnection());
  req.resetUri();
  CPPUNIT_ASSERT(req.supportsPersistentConnection());
}

void RequestTest::testRedirectUri_supportsPersistentConnection()
{
  Request req;
  CPPUNIT_ASSERT(req.setUri("http://host/file"));
  req.supportsPersistentConnection(false);
  CPPUNIT_ASSERT(!req.supportsPersistentConnection());
  req.redirectUri("http://host/file");
  CPPUNIT_ASSERT(req.supportsPersistentConnection());
}

void RequestTest::testSetUri_ipv6()
{
  Request req;
  CPPUNIT_ASSERT(!req.setUri("http://[::1"));
  CPPUNIT_ASSERT(req.setUri("http://[::1]"));
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), req.getHost());

  CPPUNIT_ASSERT(req.setUri("http://[::1]:8000/dir/file"));
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), req.getHost());
  CPPUNIT_ASSERT_EQUAL((uint16_t)8000, req.getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("/dir"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("file"), req.getFile());
  CPPUNIT_ASSERT(req.isIPv6LiteralAddress());
}

void RequestTest::testGetURIHost()
{
  Request req;
  CPPUNIT_ASSERT(req.setUri("http://[::1]"));
  CPPUNIT_ASSERT_EQUAL(std::string("[::1]"), req.getURIHost());
}

} // namespace aria2
