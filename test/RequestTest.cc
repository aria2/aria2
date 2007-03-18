#include "Request.h"
#include "Netrc.h"
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
  CPPUNIT_TEST(testRedirectUrl);
  CPPUNIT_TEST(testRedirectUrl2);
  CPPUNIT_TEST(testResetUrl);
  CPPUNIT_TEST(testSafeChar);
  CPPUNIT_TEST(testInnerLink);
  CPPUNIT_TEST(testMetalink);
  CPPUNIT_TEST(testResolveHttpAuthConfigItem);
  CPPUNIT_TEST(testResolveHttpAuthConfigItem_noCandidate);
  CPPUNIT_TEST(testResolveHttpProxyAuthConfigItem);
  CPPUNIT_TEST(testResolveHttpProxyAuthConfigItem_noCandidate);
  CPPUNIT_TEST(testResolveFtpAuthConfigItem);
  CPPUNIT_TEST(testResolveFtpAuthConfigItem_noCandidate);
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
  void testRedirectUrl();
  void testRedirectUrl2();
  void testResetUrl();
  void testSafeChar();
  void testInnerLink();
  void testMetalink();
  void testResolveHttpAuthConfigItem();
  void testResolveHttpAuthConfigItem_noCandidate();
  void testResolveHttpProxyAuthConfigItem();
  void testResolveHttpProxyAuthConfigItem_noCandidate();
  void testResolveFtpAuthConfigItem();
  void testResolveFtpAuthConfigItem_noCandidate();
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
  CPPUNIT_ASSERT_EQUAL(80, req.getPort());
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
  CPPUNIT_ASSERT_EQUAL(8080, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("index.html"), req.getFile());
}

void RequestTest::testSetUrl3() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/aria2/index.html");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/aria2"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("index.html"), req.getFile());
}

void RequestTest::testSetUrl4() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/aria2/aria3/index.html");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/aria2/aria3"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("index.html"), req.getFile());
}

void RequestTest::testSetUrl5() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/aria2/aria3/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/aria2/aria3"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string(""), req.getFile());
}

void RequestTest::testSetUrl6() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/aria2/aria3");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(80, req.getPort());
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
  CPPUNIT_ASSERT_EQUAL(8080, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("abc?query"), req.getFile());
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
  // previousUrl must be updated
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com:8080/aria2/index.html"), req.getPreviousUrl());
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.co.jp"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(80, req.getPort());
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

  // previousUrl is updated
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com/download.html"), req.getPreviousUrl());
  // referer is unchagned
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com/"), req.getReferer());

  bool v3 = req.redirectUrl("http://aria.rednoah.com/error.html");

  // previousUrl is update
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com/403.html"), req.getPreviousUrl());
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
  CPPUNIT_ASSERT_EQUAL(8080, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/aria2"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("index.html"), req.getFile());
}

void RequestTest::testSafeChar() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/|<>");
  CPPUNIT_ASSERT(!v);
}

void RequestTest::testInnerLink() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/index.html#download");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("index.html"), req.getFile());
}

void RequestTest::testMetalink() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/download/aria.tar.bz2#!metalink3!http://aria2.sourceforge.net/download/aria.metalink");
  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("aria2.sourceforge.net"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/download"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string("aria.metalink"), req.getFile());

  bool v2 = req.setUrl("http://aria.rednoah.com/download/aria.tar.bz2#!metalink3!");
  CPPUNIT_ASSERT(!v2);
}

void RequestTest::testResolveHttpAuthConfigItem()
{
  Request req;
  req.setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");
  // with no authConfig
  CPPUNIT_ASSERT(req.resolveHttpAuthConfigItem().isNull());

  // with Netrc
  NetrcHandle netrc = new Netrc();
  netrc->addAuthenticator(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount"));
  NetrcSingletonHolder::instance(netrc);
  CPPUNIT_ASSERT(!req.resolveHttpAuthConfigItem().isNull());
  AuthConfigItemHandle authConfig1 = req.resolveHttpAuthConfigItem();
  CPPUNIT_ASSERT_EQUAL(string("default"), authConfig1->getUser());
  CPPUNIT_ASSERT_EQUAL(string("defaultpassword"), authConfig1->getPassword());

  // with Netrc + user defined
  AuthConfigHandle authConfig = new AuthConfig();
  authConfig->setHttpAuthConfigItem("userDefinedUser", "userDefinedPassword");
  req.setUserDefinedAuthConfig(authConfig);
  CPPUNIT_ASSERT(!req.resolveHttpAuthConfigItem().isNull());
  AuthConfigItemHandle authConfig2 = req.resolveHttpAuthConfigItem();
  CPPUNIT_ASSERT_EQUAL(string("userDefinedUser"), authConfig2->getUser());
  CPPUNIT_ASSERT_EQUAL(string("userDefinedPassword"), authConfig2->getPassword());
}

void RequestTest::testResolveHttpAuthConfigItem_noCandidate()
{
  Request req;
  req.setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");

  NetrcHandle netrc = new Netrc();
  netrc->addAuthenticator(new Authenticator("localhost2", "default", "defaultpassword", "defaultaccount"));
  NetrcSingletonHolder::instance(netrc);
  CPPUNIT_ASSERT(req.resolveHttpAuthConfigItem().isNull());
}

void RequestTest::testResolveHttpProxyAuthConfigItem()
{
  Request req;
  req.setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");
  // with no authConfig
  CPPUNIT_ASSERT(req.resolveHttpProxyAuthConfigItem().isNull());

  // with Netrc
  NetrcHandle netrc = new Netrc();
  netrc->addAuthenticator(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount"));
  NetrcSingletonHolder::instance(netrc);
  CPPUNIT_ASSERT(!req.resolveHttpProxyAuthConfigItem().isNull());
  AuthConfigItemHandle authConfig1 = req.resolveHttpProxyAuthConfigItem();
  CPPUNIT_ASSERT_EQUAL(string("default"), authConfig1->getUser());
  CPPUNIT_ASSERT_EQUAL(string("defaultpassword"), authConfig1->getPassword());

  // with Netrc + user defined
  AuthConfigHandle authConfig = new AuthConfig();
  authConfig->setHttpProxyAuthConfigItem("userDefinedUser", "userDefinedPassword");
  req.setUserDefinedAuthConfig(authConfig);
  CPPUNIT_ASSERT(!req.resolveHttpProxyAuthConfigItem().isNull());
  AuthConfigItemHandle authConfig2 = req.resolveHttpProxyAuthConfigItem();
  CPPUNIT_ASSERT_EQUAL(string("userDefinedUser"), authConfig2->getUser());
  CPPUNIT_ASSERT_EQUAL(string("userDefinedPassword"), authConfig2->getPassword());
}

void RequestTest::testResolveHttpProxyAuthConfigItem_noCandidate()
{
  Request req;
  req.setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");

  NetrcHandle netrc = new Netrc();
  netrc->addAuthenticator(new Authenticator("localhost2", "default", "defaultpassword", "defaultaccount"));
  NetrcSingletonHolder::instance(netrc);
  CPPUNIT_ASSERT(req.resolveHttpProxyAuthConfigItem().isNull());
}

void RequestTest::testResolveFtpAuthConfigItem()
{
  Request req;
  req.setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");
  // with no authConfig
  CPPUNIT_ASSERT(!req.resolveFtpAuthConfigItem().isNull());
  CPPUNIT_ASSERT_EQUAL(string("anonymous"), req.resolveFtpAuthConfigItem()->getUser());
  CPPUNIT_ASSERT_EQUAL(string("ARIA2USER@"), req.resolveFtpAuthConfigItem()->getPassword());

  // with Netrc
  NetrcHandle netrc = new Netrc();
  netrc->addAuthenticator(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount"));
  NetrcSingletonHolder::instance(netrc);
  CPPUNIT_ASSERT(!req.resolveFtpAuthConfigItem().isNull());
  AuthConfigItemHandle authConfig1 = req.resolveFtpAuthConfigItem();
  CPPUNIT_ASSERT_EQUAL(string("default"), authConfig1->getUser());
  CPPUNIT_ASSERT_EQUAL(string("defaultpassword"), authConfig1->getPassword());

  // with Netrc + user defined
  AuthConfigHandle authConfig = new AuthConfig();
  authConfig->setFtpAuthConfigItem("userDefinedUser", "userDefinedPassword");
  req.setUserDefinedAuthConfig(authConfig);
  CPPUNIT_ASSERT(!req.resolveFtpAuthConfigItem().isNull());
  AuthConfigItemHandle authConfig2 = req.resolveFtpAuthConfigItem();
  CPPUNIT_ASSERT_EQUAL(string("userDefinedUser"), authConfig2->getUser());
  CPPUNIT_ASSERT_EQUAL(string("userDefinedPassword"), authConfig2->getPassword());
}

void RequestTest::testResolveFtpAuthConfigItem_noCandidate()
{
  Request req;
  req.setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");

  NetrcHandle netrc = new Netrc();
  netrc->addAuthenticator(new Authenticator("localhost2", "default", "defaultpassword", "defaultaccount"));
  NetrcSingletonHolder::instance(netrc);
  CPPUNIT_ASSERT(!req.resolveFtpAuthConfigItem().isNull());
  CPPUNIT_ASSERT_EQUAL(string("anonymous"), req.resolveFtpAuthConfigItem()->getUser());
  CPPUNIT_ASSERT_EQUAL(string("ARIA2USER@"), req.resolveFtpAuthConfigItem()->getPassword());
}
