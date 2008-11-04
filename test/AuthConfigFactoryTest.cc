#include "AuthConfigFactory.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Netrc.h"
#include "prefs.h"
#include "Request.h"
#include "AuthConfig.h"
#include "Option.h"

namespace aria2 {

class AuthConfigFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(AuthConfigFactoryTest);
  CPPUNIT_TEST(testCreateAuthConfig_http);
  CPPUNIT_TEST(testCreateAuthConfig_ftp);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testCreateAuthConfig_http();
  void testCreateAuthConfig_ftp();
};


CPPUNIT_TEST_SUITE_REGISTRATION( AuthConfigFactoryTest );

void AuthConfigFactoryTest::testCreateAuthConfig_http()
{
  SharedHandle<Request> req(new Request());
  req->setUrl("http://localhost/download/aria2-1.0.0.tar.bz2");

  Option option;
  option.put(PREF_NO_NETRC, V_FALSE);

  AuthConfigFactory factory(&option);

  // without auth info
  CPPUNIT_ASSERT_EQUAL(std::string(":"),
		       factory.createAuthConfig(req)->getAuthText());

  // with Netrc
  SharedHandle<Netrc> netrc(new Netrc());
  netrc->addAuthenticator
    (SharedHandle<Authenticator>(new Authenticator("localhost",
						   "localhostuser",
						   "localhostpass",
						   "localhostacct")));
  netrc->addAuthenticator
    (SharedHandle<Authenticator>(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount")));
  factory.setNetrc(netrc);

  CPPUNIT_ASSERT_EQUAL(std::string("localhostuser:localhostpass"),
		       factory.createAuthConfig(req)->getAuthText());

  // See default token in netrc is ignored.
  SharedHandle<Request> mirrorReq(new Request());
  req->setUrl("http://mirror/");
  CPPUNIT_ASSERT_EQUAL(std::string(":"),
		       factory.createAuthConfig(mirrorReq)->getAuthText());

  // with Netrc + user defined
  option.put(PREF_HTTP_USER, "userDefinedUser");
  option.put(PREF_HTTP_PASSWD, "userDefinedPassword");
  CPPUNIT_ASSERT_EQUAL(std::string("userDefinedUser:userDefinedPassword"),
		       factory.createAuthConfig(req)->getAuthText());

  // username and password in URI: disabled by default.
  req->setUrl("http://aria2user:aria2password@localhost/download/aria2-1.0.0.tar.bz2");
  CPPUNIT_ASSERT_EQUAL(std::string("userDefinedUser:userDefinedPassword"),
		       factory.createAuthConfig(req)->getAuthText());  
}

void AuthConfigFactoryTest::testCreateAuthConfig_ftp()
{
  SharedHandle<Request> req(new Request());
  req->setUrl("ftp://localhost/download/aria2-1.0.0.tar.bz2");

  Option option;
  option.put(PREF_NO_NETRC, V_FALSE);

  AuthConfigFactory factory(&option);

  // without auth info
  CPPUNIT_ASSERT_EQUAL(std::string("anonymous:ARIA2USER@"),
		       factory.createAuthConfig(req)->getAuthText());

  // with Netrc
  SharedHandle<Netrc> netrc(new Netrc());
  netrc->addAuthenticator
    (SharedHandle<Authenticator>(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount")));
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
