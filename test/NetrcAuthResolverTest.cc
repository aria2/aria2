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
  std::shared_ptr<Netrc> netrc_;
  //std::shared_ptr<Option> _option;
  std::shared_ptr<NetrcAuthResolver> resolver_;
public:
  void setUp()
  {
    netrc_.reset(new Netrc());
    netrc_->addAuthenticator
      (std::shared_ptr<Authenticator>(new Authenticator("localhost", "name", "passwd", "account")));
    netrc_->addAuthenticator
      (std::shared_ptr<Authenticator>(new DefaultAuthenticator("default", "defaultpasswd", "defaultaccount")));

    //_option = new Option();
    resolver_.reset(new NetrcAuthResolver());
    resolver_->setNetrc(netrc_);
    resolver_->setDefaultAuthConfig
      (std::shared_ptr<AuthConfig>(new AuthConfig("foo", "bar")));
  }

  void testResolveAuthConfig_without_userDefined();
  void testResolveAuthConfig_with_userDefined();
  void testResolveAuthConfig_ignoreDefault();
};


CPPUNIT_TEST_SUITE_REGISTRATION( NetrcAuthResolverTest );

void NetrcAuthResolverTest::testResolveAuthConfig_without_userDefined()
{
  std::shared_ptr<AuthConfig> authConfig = resolver_->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("name:passwd"), authConfig->getAuthText());

  authConfig = resolver_->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(std::string("default:defaultpasswd"), authConfig->getAuthText());

  resolver_->setNetrc(std::shared_ptr<Netrc>());
  authConfig = resolver_->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("foo:bar"), authConfig->getAuthText());

}

void NetrcAuthResolverTest::testResolveAuthConfig_with_userDefined()
{
  resolver_->setUserDefinedAuthConfig
    (std::shared_ptr<AuthConfig>(new AuthConfig("myname", "mypasswd")));
  std::shared_ptr<AuthConfig> authConfig = resolver_->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"), authConfig->getAuthText());

  authConfig = resolver_->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"), authConfig->getAuthText());

  resolver_->setNetrc(std::shared_ptr<Netrc>());
  authConfig = resolver_->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"), authConfig->getAuthText());
}

void NetrcAuthResolverTest::testResolveAuthConfig_ignoreDefault()
{
  resolver_->ignoreDefault();
  std::shared_ptr<AuthConfig> authConfig = resolver_->resolveAuthConfig("mirror");
  CPPUNIT_ASSERT_EQUAL(std::string("foo:bar"), authConfig->getAuthText());

  resolver_->useDefault();
  std::shared_ptr<AuthConfig> defAuthConfig =
    resolver_->resolveAuthConfig("mirror");
  CPPUNIT_ASSERT_EQUAL(std::string("default:defaultpasswd"),
                       defAuthConfig->getAuthText());
}

} // namespace aria2
