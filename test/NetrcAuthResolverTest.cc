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
  SharedHandle<Netrc> _netrc;
  //SharedHandle<Option> _option;
  SharedHandle<NetrcAuthResolver> _resolver;
public:
  void setUp()
  {
    _netrc.reset(new Netrc());
    _netrc->addAuthenticator
      (SharedHandle<Authenticator>(new Authenticator("localhost", "name", "passwd", "account")));
    _netrc->addAuthenticator
      (SharedHandle<Authenticator>(new DefaultAuthenticator("default", "defaultpasswd", "defaultaccount")));

    //_option = new Option();
    _resolver.reset(new NetrcAuthResolver());
    _resolver->setNetrc(_netrc);
    _resolver->setDefaultAuthConfig
      (SharedHandle<AuthConfig>(new AuthConfig("foo", "bar")));
  }

  void testResolveAuthConfig_without_userDefined();
  void testResolveAuthConfig_with_userDefined();
  void testResolveAuthConfig_ignoreDefault();
};


CPPUNIT_TEST_SUITE_REGISTRATION( NetrcAuthResolverTest );

void NetrcAuthResolverTest::testResolveAuthConfig_without_userDefined()
{
  SharedHandle<AuthConfig> authConfig = _resolver->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("name:passwd"), authConfig->getAuthText());

  authConfig = _resolver->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(std::string("default:defaultpasswd"), authConfig->getAuthText());

  _resolver->setNetrc(SharedHandle<Netrc>());
  authConfig = _resolver->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("foo:bar"), authConfig->getAuthText());

}

void NetrcAuthResolverTest::testResolveAuthConfig_with_userDefined()
{
  _resolver->setUserDefinedAuthConfig
    (SharedHandle<AuthConfig>(new AuthConfig("myname", "mypasswd")));
  SharedHandle<AuthConfig> authConfig = _resolver->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"), authConfig->getAuthText());

  authConfig = _resolver->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"), authConfig->getAuthText());

  _resolver->setNetrc(SharedHandle<Netrc>());
  authConfig = _resolver->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"), authConfig->getAuthText());
}

void NetrcAuthResolverTest::testResolveAuthConfig_ignoreDefault()
{
  _resolver->ignoreDefault();
  SharedHandle<AuthConfig> authConfig = _resolver->resolveAuthConfig("mirror");
  CPPUNIT_ASSERT_EQUAL(std::string("foo:bar"), authConfig->getAuthText());

  _resolver->useDefault();
  SharedHandle<AuthConfig> defAuthConfig =
    _resolver->resolveAuthConfig("mirror");
  CPPUNIT_ASSERT_EQUAL(std::string("default:defaultpasswd"),
		       defAuthConfig->getAuthText());  
}

} // namespace aria2
