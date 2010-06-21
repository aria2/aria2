#include "NetrcAuthResolver.h"
#include "prefs.h"
#include "Netrc.h"
#include "AuthConfig.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class NetrcAuthResolverTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(NetrcAuthResolverTest);
  CPPUNIT_TEST(testResolveAuthConfig_without_userDefined);
  CPPUNIT_TEST(testResolveAuthConfig_with_userDefined);
  CPPUNIT_TEST(testResolveAuthConfig_ignoreDefault);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Netrc> netrc_;
  //SharedHandle<Option> _option;
  SharedHandle<NetrcAuthResolver> resolver_;
public:
  void setUp()
  {
    netrc_.reset(new Netrc());
    netrc_->addAuthenticator
      (SharedHandle<Authenticator>(new Authenticator("localhost", "name", "passwd", "account")));
    netrc_->addAuthenticator
      (SharedHandle<Authenticator>(new DefaultAuthenticator("default", "defaultpasswd", "defaultaccount")));

    //_option = new Option();
    resolver_.reset(new NetrcAuthResolver());
    resolver_->setNetrc(netrc_);
    resolver_->setDefaultAuthConfig
      (SharedHandle<AuthConfig>(new AuthConfig("foo", "bar")));
  }

  void testResolveAuthConfig_without_userDefined();
  void testResolveAuthConfig_with_userDefined();
  void testResolveAuthConfig_ignoreDefault();
};


CPPUNIT_TEST_SUITE_REGISTRATION( NetrcAuthResolverTest );

void NetrcAuthResolverTest::testResolveAuthConfig_without_userDefined()
{
  SharedHandle<AuthConfig> authConfig = resolver_->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("name:passwd"), authConfig->getAuthText());

  authConfig = resolver_->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(std::string("default:defaultpasswd"), authConfig->getAuthText());

  resolver_->setNetrc(SharedHandle<Netrc>());
  authConfig = resolver_->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("foo:bar"), authConfig->getAuthText());

}

void NetrcAuthResolverTest::testResolveAuthConfig_with_userDefined()
{
  resolver_->setUserDefinedAuthConfig
    (SharedHandle<AuthConfig>(new AuthConfig("myname", "mypasswd")));
  SharedHandle<AuthConfig> authConfig = resolver_->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"), authConfig->getAuthText());

  authConfig = resolver_->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"), authConfig->getAuthText());

  resolver_->setNetrc(SharedHandle<Netrc>());
  authConfig = resolver_->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"), authConfig->getAuthText());
}

void NetrcAuthResolverTest::testResolveAuthConfig_ignoreDefault()
{
  resolver_->ignoreDefault();
  SharedHandle<AuthConfig> authConfig = resolver_->resolveAuthConfig("mirror");
  CPPUNIT_ASSERT_EQUAL(std::string("foo:bar"), authConfig->getAuthText());

  resolver_->useDefault();
  SharedHandle<AuthConfig> defAuthConfig =
    resolver_->resolveAuthConfig("mirror");
  CPPUNIT_ASSERT_EQUAL(std::string("default:defaultpasswd"),
                       defAuthConfig->getAuthText());  
}

} // namespace aria2
