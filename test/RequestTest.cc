#include "Request.h"
#include "Netrc.h"
#include "DefaultAuthResolver.h"
#include "NetrcAuthResolver.h"
#include <cppunit/extensions/HelperMacros.h>

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
  CPPUNIT_TEST(testSetUrl17);
  CPPUNIT_TEST(testRedirectUrl);
  CPPUNIT_TEST(testRedirectUrl2);
  CPPUNIT_TEST(testResetUrl);
  CPPUNIT_TEST(testInnerLink);
  CPPUNIT_TEST(testResolveHttpAuthConfig);
  CPPUNIT_TEST(testResolveHttpAuthConfig_noCandidate);
  CPPUNIT_TEST(testResolveHttpProxyAuthConfig);
  CPPUNIT_TEST(testResolveFtpAuthConfig);
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
  void testSetUrl17();
  void testRedirectUrl();
  void testRedirectUrl2();
  void testResetUrl();
  void testInnerLink();
  void testResolveHttpAuthConfig();
  void testResolveHttpAuthConfig_noCandidate();
  void testResolveHttpProxyAuthConfig();
  void testResolveFtpAuthConfig();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestTest );

void RequestTest::testSetUrl1() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com/"), req.getUrl());
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com/"), req.getCurrentUrl());
  CPPUNIT_ASSERT_EQUAL(string(""), req.getPreviousUrl());
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((int32_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string(""), req.getFile());
}

void RequestTest::testSetUrl2() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com:8080/index.html");
  req.setReferer("http://aria.rednoah.com:8080");

  CPPUNIT_ASSERT(v);

  // referer is unchaged
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com:8080"), req.getReferer());
  // previousUrl must equal to referer;
  CPPUNIT_ASSERT_EQUAL(req.getReferer(), req.getPreviousUrl());
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((int32_t)8080, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("index.html"), req.getFile());
}

void RequestTest::testSetUrl3() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/aria2/index.html");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((int32_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/aria2"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("index.html"), req.getFile());
}

void RequestTest::testSetUrl4() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/aria2/aria3/index.html");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((int32_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/aria2/aria3"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("index.html"), req.getFile());
}

void RequestTest::testSetUrl5() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/aria2/aria3/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((int32_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/aria2/aria3"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string(""), req.getFile());
}

void RequestTest::testSetUrl6() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/aria2/aria3");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((int32_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/aria2"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("aria3"), req.getFile());
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
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("?query/"), req.getFile());
}

void RequestTest::testSetUrl12() {
  Request req;
  bool v = req.setUrl("http://host?query");
  
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("?query"), req.getFile());
}

void RequestTest::testSetUrl13() {
  Request req;
  bool v = req.setUrl("http://host/?query");
  
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("?query"), req.getFile());
}

void RequestTest::testSetUrl14() {
  Request req;
  bool v = req.setUrl("http://host:8080/abc?query");
  
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL((int32_t)8080, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("abc?query"), req.getFile());
}

void RequestTest::testSetUrl17()
{
  Request req;
  bool v = req.setUrl("http://host:80/file<with%2 %20space/file with space;param?a=/?");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(string("host"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/file%3cwith%252%20%20space"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("file%20with%20space;param?a=/?"), req.getFile());
}

void RequestTest::testRedirectUrl() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com:8080/aria2/index.html");
  
  bool v2 = req.redirectUrl("http://aria.rednoah.co.jp/");
  CPPUNIT_ASSERT(v2);
  // url must be the same
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com:8080/aria2/index.html"),
		       req.getUrl());
  // currentUrl must be updated
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.co.jp/"), req.getCurrentUrl());
  // previousUrl must be "" when redirection
  CPPUNIT_ASSERT_EQUAL(string(""), req.getPreviousUrl());
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.co.jp"), req.getHost());
  CPPUNIT_ASSERT_EQUAL((int32_t)80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string(""), req.getFile());
}

void RequestTest::testRedirectUrl2() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/download.html");
  CPPUNIT_ASSERT_EQUAL(string(""), req.getPreviousUrl());
  req.setReferer("http://aria.rednoah.com/");
  // previousUrl is updated when referer is specified
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com/"), req.getPreviousUrl());
  bool v2 = req.redirectUrl("http://aria.rednoah.com/403.html");

  // previousUrl must be "" when redirection
  CPPUNIT_ASSERT_EQUAL(string(""), req.getPreviousUrl());
  // referer is unchagned
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com/"), req.getReferer());

  bool v3 = req.redirectUrl("http://aria.rednoah.com/error.html");

  // previousUrl must be "" when redirection
  CPPUNIT_ASSERT_EQUAL(string(""), req.getPreviousUrl());
}
  
void RequestTest::testResetUrl() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com:8080/aria2/index.html");
  req.setReferer("http://aria.rednoah.com:8080/");
  bool v2 = req.redirectUrl("ftp://aria.rednoah.co.jp/");

  bool v3 = req.resetUrl();
  CPPUNIT_ASSERT(v3);
  // currentUrl must equal to url
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com:8080/aria2/index.html"), req.getUrl());
  CPPUNIT_ASSERT_EQUAL(req.getUrl(), req.getCurrentUrl());
  // previousUrl must equal to referer
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com:8080/"), req.getPreviousUrl());
  // referer is unchanged
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com:8080/"), req.getReferer());
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL((int32_t)8080, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/aria2"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("index.html"), req.getFile());
}

void RequestTest::testInnerLink() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/index.html#download");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("index.html"), req.getFile());
}

void RequestTest::testResolveHttpAuthConfig()
{
  Request req;
  req.setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");
  // with DefaultAuthResolver
  DefaultAuthResolverHandle defaultAuthResolver = new DefaultAuthResolver();
  req.setHttpAuthResolver(defaultAuthResolver);
  CPPUNIT_ASSERT(!req.resolveHttpAuthConfig().isNull());
  CPPUNIT_ASSERT_EQUAL(string(":"),
		       req.resolveHttpAuthConfig()->getAuthText());

  // with Netrc
  NetrcHandle netrc = new Netrc();
  netrc->addAuthenticator(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount"));
  NetrcAuthResolverHandle netrcAuthResolver = new NetrcAuthResolver();
  netrcAuthResolver->setNetrc(netrc);
  req.setHttpAuthResolver(netrcAuthResolver);
  AuthConfigHandle authConfig1 = req.resolveHttpAuthConfig();
  CPPUNIT_ASSERT(!authConfig1.isNull());
  CPPUNIT_ASSERT_EQUAL(string("default:defaultpassword"),
		       authConfig1->getAuthText());

  // with Netrc + user defined
  AuthConfigHandle authConfig =
    new AuthConfig("userDefinedUser", "userDefinedPassword");
  netrcAuthResolver->setUserDefinedAuthConfig(authConfig);
  AuthConfigHandle authConfig2 = req.resolveHttpAuthConfig();
  CPPUNIT_ASSERT(!authConfig2.isNull());
  CPPUNIT_ASSERT_EQUAL(string("userDefinedUser:userDefinedPassword"),
		       authConfig2->getAuthText());
}

void RequestTest::testResolveHttpAuthConfig_noCandidate()
{
  Request req;
  req.setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");

  DefaultAuthResolverHandle defaultAuthResolver = new DefaultAuthResolver();
  req.setHttpAuthResolver(defaultAuthResolver);
  CPPUNIT_ASSERT_EQUAL(string(":"),
		       req.resolveHttpAuthConfig()->getAuthText());
}

void RequestTest::testResolveHttpProxyAuthConfig()
{
  Request req;
  req.setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");
  // with Netrc
  NetrcHandle netrc = new Netrc();
  netrc->addAuthenticator(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount"));
  NetrcAuthResolverHandle netrcAuthResolver = new NetrcAuthResolver();
  netrcAuthResolver->setNetrc(netrc);
  req.setHttpProxyAuthResolver(netrcAuthResolver);
  AuthConfigHandle authConfig1 = req.resolveHttpProxyAuthConfig();
  CPPUNIT_ASSERT(!authConfig1.isNull());
  CPPUNIT_ASSERT_EQUAL(string("default:defaultpassword"),
		       authConfig1->getAuthText());
}

void RequestTest::testResolveFtpAuthConfig()
{
  Request req;
  req.setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");
  // with Netrc
  NetrcHandle netrc = new Netrc();
  netrc->addAuthenticator(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount"));
  NetrcAuthResolverHandle netrcAuthResolver = new NetrcAuthResolver();
  netrcAuthResolver->setNetrc(netrc);
  req.setFtpAuthResolver(netrcAuthResolver);
  AuthConfigHandle authConfig1 = req.resolveFtpAuthConfig();
  CPPUNIT_ASSERT(!authConfig1.isNull());
  CPPUNIT_ASSERT_EQUAL(string("default:defaultpassword"),
		       authConfig1->getAuthText());
}
