#include "Request.h"

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
  CPPUNIT_TEST(testRedirectUrl);
  CPPUNIT_TEST(testResetUrl);
  CPPUNIT_TEST(testSafeChar);
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
  void testRedirectUrl();
  void testResetUrl();
  void testSafeChar();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestTest );

void RequestTest::testSetUrl1() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com/");

  CPPUNIT_ASSERT(v);
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com/"), req.getUrl());
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com/"), req.getCurrentUrl());
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.com"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string(""), req.getFile());
}

void RequestTest::testSetUrl2() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com:8080/index.html");

  CPPUNIT_ASSERT(v);
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
  CPPUNIT_ASSERT_EQUAL(string("http"), req.getProtocol());
  CPPUNIT_ASSERT_EQUAL(string("aria.rednoah.co.jp"), req.getHost());
  CPPUNIT_ASSERT_EQUAL(80, req.getPort());
  CPPUNIT_ASSERT_EQUAL(string("/"), req.getDir());
  CPPUNIT_ASSERT_EQUAL(string(""), req.getFile());
}

void RequestTest::testResetUrl() {
  Request req;
  bool v = req.setUrl("http://aria.rednoah.com:8080/aria2/index.html");
  
  bool v2 = req.redirectUrl("ftp://aria.rednoah.co.jp/");

  bool v3 = req.resetUrl();
  CPPUNIT_ASSERT(v3);
  // currentUrl must equal to url
  CPPUNIT_ASSERT_EQUAL(string("http://aria.rednoah.com:8080/aria2/index.html"), req.getUrl());
  CPPUNIT_ASSERT_EQUAL(req.getUrl(), req.getCurrentUrl());
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
