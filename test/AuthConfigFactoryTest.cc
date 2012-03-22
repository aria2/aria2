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
  CPPUNIT_TEST(testCreateAuthConfig_httpNoChallenge);
  CPPUNIT_TEST(testCreateAuthConfig_ftp);
  CPPUNIT_TEST(testUpdateBasicCred);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testCreateAuthConfig_http();
  void testCreateAuthConfig_httpNoChallenge();
  void testCreateAuthConfig_ftp();
  void testUpdateBasicCred();
};


CPPUNIT_TEST_SUITE_REGISTRATION( AuthConfigFactoryTest );

void AuthConfigFactoryTest::testCreateAuthConfig_http()
{
  SharedHandle<Request> req(new Request());
  req->setUri("http://localhost/download/aria2-1.0.0.tar.bz2");

  Option option;
  option.put(PREF_NO_NETRC, A2_V_FALSE);
  option.put(PREF_HTTP_AUTH_CHALLENGE, A2_V_TRUE);

  AuthConfigFactory factory;

  // without auth info
  CPPUNIT_ASSERT(!factory.createAuthConfig(req, &option));

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

  // not activated
  CPPUNIT_ASSERT(!factory.createAuthConfig(req, &option));

  CPPUNIT_ASSERT(factory.activateBasicCred("localhost", 80, "/", &option));

  CPPUNIT_ASSERT_EQUAL(std::string("localhostuser:localhostpass"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  // See default token in netrc is ignored.
  req->setUri("http://mirror/");

  CPPUNIT_ASSERT(!factory.activateBasicCred("mirror", 80, "/", &option));

  CPPUNIT_ASSERT(!factory.createAuthConfig(req, &option));

  // with Netrc + user defined
  option.put(PREF_HTTP_USER, "userDefinedUser");
  option.put(PREF_HTTP_PASSWD, "userDefinedPassword");

  CPPUNIT_ASSERT(!factory.createAuthConfig(req, &option));

  CPPUNIT_ASSERT(factory.activateBasicCred("mirror", 80, "/", &option));

  CPPUNIT_ASSERT_EQUAL(std::string("userDefinedUser:userDefinedPassword"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  // username and password in URI
  req->setUri("http://aria2user:aria2password@localhost/download/aria2-1.0.0.tar.bz2");
  CPPUNIT_ASSERT_EQUAL(std::string("aria2user:aria2password"),
                       factory.createAuthConfig(req, &option)->getAuthText());  
}

void AuthConfigFactoryTest::testCreateAuthConfig_httpNoChallenge()
{
  SharedHandle<Request> req(new Request());
  req->setUri("http://localhost/download/aria2-1.0.0.tar.bz2");

  Option option;
  option.put(PREF_NO_NETRC, A2_V_FALSE);

  AuthConfigFactory factory;

  // without auth info
  CPPUNIT_ASSERT(!factory.createAuthConfig(req, &option));

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

  // not activated
  CPPUNIT_ASSERT_EQUAL(std::string("localhostuser:localhostpass"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  // See default token in netrc is ignored.
  req->setUri("http://mirror/");

  CPPUNIT_ASSERT(!factory.createAuthConfig(req, &option));

  // with Netrc + user defined
  option.put(PREF_HTTP_USER, "userDefinedUser");
  option.put(PREF_HTTP_PASSWD, "userDefinedPassword");

  CPPUNIT_ASSERT_EQUAL(std::string("userDefinedUser:userDefinedPassword"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  // username and password in URI
  req->setUri("http://aria2user:aria2password@localhost/download/aria2-1.0.0.tar.bz2");
  CPPUNIT_ASSERT_EQUAL(std::string("aria2user:aria2password"),
                       factory.createAuthConfig(req, &option)->getAuthText());  
}

void AuthConfigFactoryTest::testCreateAuthConfig_ftp()
{
  SharedHandle<Request> req(new Request());
  req->setUri("ftp://localhost/download/aria2-1.0.0.tar.bz2");

  Option option;
  option.put(PREF_NO_NETRC, A2_V_FALSE);

  AuthConfigFactory factory;

  // without auth info
  CPPUNIT_ASSERT_EQUAL(std::string("anonymous:ARIA2USER@"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  // with Netrc
  SharedHandle<Netrc> netrc(new Netrc());
  netrc->addAuthenticator
    (SharedHandle<Authenticator>(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount")));
  factory.setNetrc(netrc);
  CPPUNIT_ASSERT_EQUAL(std::string("default:defaultpassword"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  // disable Netrc
  option.put(PREF_NO_NETRC, A2_V_TRUE);
  CPPUNIT_ASSERT_EQUAL(std::string("anonymous:ARIA2USER@"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  // with Netrc + user defined
  option.put(PREF_NO_NETRC, A2_V_FALSE);
  option.put(PREF_FTP_USER, "userDefinedUser");
  option.put(PREF_FTP_PASSWD, "userDefinedPassword");
  CPPUNIT_ASSERT_EQUAL(std::string("userDefinedUser:userDefinedPassword"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  // username and password in URI
  req->setUri("ftp://aria2user:aria2password@localhost/download/aria2-1.0.0.tar.bz2");
  CPPUNIT_ASSERT_EQUAL(std::string("aria2user:aria2password"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  // username in URI, but no password. We have DefaultAuthenticator
  // but username is not aria2user
  req->setUri("ftp://aria2user@localhost/download/aria2-1.0.0.tar.bz2");
  CPPUNIT_ASSERT_EQUAL(std::string("aria2user:userDefinedPassword"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  // Recreate netrc with entry for user aria2user
  netrc.reset(new Netrc());
  netrc->addAuthenticator
    (SharedHandle<Authenticator>(new Authenticator("localhost",
                                                   "aria2user",
                                                   "netrcpass",
                                                   "netrcacct")));
  factory.setNetrc(netrc);
  // This time, we can find same username "aria2user" in netrc, so the
  // password "netrcpass" is used, instead of "userDefinedPassword"
  CPPUNIT_ASSERT_EQUAL(std::string("aria2user:netrcpass"),
                       factory.createAuthConfig(req, &option)->getAuthText());
  // No netrc entry for host mirror, so "userDefinedPassword" is used.
  req->setUri("ftp://aria2user@mirror/download/aria2-1.0.0.tar.bz2");
  CPPUNIT_ASSERT_EQUAL(std::string("aria2user:userDefinedPassword"),
                       factory.createAuthConfig(req, &option)->getAuthText());
}

namespace {
SharedHandle<AuthConfigFactory::BasicCred>
createBasicCred(const std::string& user,
                const std::string& password,
                const std::string& host, uint16_t port,
                const std::string& path,
                bool activated = false)
{
  SharedHandle<AuthConfigFactory::BasicCred> bc
    (new AuthConfigFactory::BasicCred(user, password, host, port, path,
                                      activated));
  return bc;
}
} // namespace

void AuthConfigFactoryTest::testUpdateBasicCred()
{
  Option option;
  option.put(PREF_NO_NETRC, A2_V_FALSE);
  option.put(PREF_HTTP_AUTH_CHALLENGE, A2_V_TRUE);

  AuthConfigFactory factory;

  factory.updateBasicCred
    (createBasicCred("myname", "mypass", "localhost", 80, "/", true));
  factory.updateBasicCred
    (createBasicCred("price", "j38jdc", "localhost", 80, "/download", true));
  factory.updateBasicCred
    (createBasicCred("soap", "planB", "localhost", 80, "/download/beta", true));
  factory.updateBasicCred
    (createBasicCred("alice", "ium8", "localhost", 80, "/documents", true));
  factory.updateBasicCred
    (createBasicCred("jack", "jackx", "mirror", 80, "/doc", true));

  SharedHandle<Request> req(new Request());
  req->setUri("http://localhost/download/v2.6/Changelog");
  CPPUNIT_ASSERT_EQUAL(std::string("price:j38jdc"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  req->setUri("http://localhost/download/beta/v2.7/Changelog");
  CPPUNIT_ASSERT_EQUAL(std::string("soap:planB"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  req->setUri("http://localhost/documents/reference.html");
  CPPUNIT_ASSERT_EQUAL(std::string("alice:ium8"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  req->setUri("http://localhost/documents2/manual.html");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypass"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  req->setUri("http://localhost/doc/readme.txt");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypass"),
                       factory.createAuthConfig(req, &option)->getAuthText());

  req->setUri("http://localhost:8080/doc/readme.txt");
  CPPUNIT_ASSERT(!factory.createAuthConfig(req, &option));

  req->setUri("http://local/");
  CPPUNIT_ASSERT(!factory.createAuthConfig(req, &option));

  req->setUri("http://mirror/");
  CPPUNIT_ASSERT(!factory.createAuthConfig(req, &option));
}

} // namespace aria2
