#include "AuthConfigFactory.h"
#include "Netrc.h"
#include "prefs.h"
#include "Request.h"
#include "AuthConfig.h"
#include "Option.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class AuthConfigFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(AuthConfigFactoryTest);
  CPPUNIT_TEST(testCreateAuthConfig_http);
  CPPUNIT_TEST(testCreateAuthConfigForHttpProxy);
  CPPUNIT_TEST(testCreateAuthConfig_ftp);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testCreateAuthConfig_http();
  void testCreateAuthConfigForHttpProxy();
  void testCreateAuthConfig_ftp();
};


CPPUNIT_TEST_SUITE_REGISTRATION( AuthConfigFactoryTest );

void AuthConfigFactoryTest::testCreateAuthConfig_http()
{
  SharedHandle<Request> req = new Request();
  req->setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");

  Option option;
  option.put(PREF_NO_NETRC, V_FALSE);

  AuthConfigFactory factory(&option);

  // without auth info
  CPPUNIT_ASSERT_EQUAL(std::string(":"),
		       factory.createAuthConfig(req)->getAuthText());

  // with Netrc: disabled by default
  SharedHandle<Netrc> netrc = new Netrc();
  netrc->addAuthenticator(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount"));
  factory.setNetrc(netrc);
  CPPUNIT_ASSERT_EQUAL(std::string(":"),
		       factory.createAuthConfig(req)->getAuthText());

  // with Netrc + user defined
  option.put(PREF_HTTP_USER, "userDefinedUser");
  option.put(PREF_HTTP_PASSWD, "userDefinedPassword");
  CPPUNIT_ASSERT_EQUAL(std::string("userDefinedUser:userDefinedPassword"),
		       factory.createAuthConfig(req)->getAuthText());

  // username and password in URI: disabled by default.
  req->setUrl("http://aria2user:aria2password@localhost/download/aria2-1.0.0.tar.bz2");
  CPPUNIT_ASSERT_EQUAL(std::string("userDefinedUser:userDefinedPassword"),
		       factory.createAuthConfig(req)->getAuthText());

//   CPPUNIT_ASSERT_EQUAL(std::string("aria2user:aria2password"),
// 		       factory.createAuthConfig(req)->getAuthText());
}

void AuthConfigFactoryTest::testCreateAuthConfigForHttpProxy()
{
  SharedHandle<Request> req = new Request();
  req->setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");
  // with Netrc
  SharedHandle<Netrc> netrc = new Netrc();
  netrc->addAuthenticator(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount"));

  Option option;
  option.put(PREF_NO_NETRC, V_FALSE);

  AuthConfigFactory factory(&option);
  factory.setNetrc(netrc);

  // netrc is not used in http proxy auth
  CPPUNIT_ASSERT_EQUAL(std::string(":"),
		       factory.createAuthConfigForHttpProxy(req)->getAuthText());

  option.put(PREF_HTTP_PROXY_USER, "userDefinedUser");
  option.put(PREF_HTTP_PROXY_PASSWD, "userDefinedPassword");
  CPPUNIT_ASSERT_EQUAL(std::string("userDefinedUser:userDefinedPassword"),
		       factory.createAuthConfigForHttpProxy(req)->getAuthText());

}

void AuthConfigFactoryTest::testCreateAuthConfig_ftp()
{
  SharedHandle<Request> req = new Request();
  req->setUrl("ftp://localhost/download/aria2-1.0.0.tar.bz2");

  Option option;
  option.put(PREF_NO_NETRC, V_FALSE);

  AuthConfigFactory factory(&option);

  // without auth info
  CPPUNIT_ASSERT_EQUAL(std::string("anonymous:ARIA2USER@"),
		       factory.createAuthConfig(req)->getAuthText());

  // with Netrc
  SharedHandle<Netrc> netrc = new Netrc();
  netrc->addAuthenticator(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount"));
  factory.setNetrc(netrc);
  CPPUNIT_ASSERT_EQUAL(std::string("default:defaultpassword"),
		       factory.createAuthConfig(req)->getAuthText());

  // disable Netrc
  option.put(PREF_NO_NETRC, V_TRUE);
  CPPUNIT_ASSERT_EQUAL(std::string("anonymous:ARIA2USER@"),
		       factory.createAuthConfig(req)->getAuthText());

  // with Netrc + user defined
  option.put(PREF_NO_NETRC, V_FALSE);
  option.put(PREF_FTP_USER, "userDefinedUser");
  option.put(PREF_FTP_PASSWD, "userDefinedPassword");
  CPPUNIT_ASSERT_EQUAL(std::string("userDefinedUser:userDefinedPassword"),
		       factory.createAuthConfig(req)->getAuthText());

  // username and password in URI
  req->setUrl("ftp://aria2user:aria2password@localhost/download/aria2-1.0.0.tar.bz2");
  CPPUNIT_ASSERT_EQUAL(std::string("aria2user:aria2password"),
 		       factory.createAuthConfig(req)->getAuthText());
}

} // namespace aria2
