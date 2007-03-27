#include "NetrcAuthResolver.h"
#include "prefs.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class NetrcAuthResolverTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(NetrcAuthResolverTest);
  CPPUNIT_TEST(testResolveAuthConfig_without_userDefined);
  CPPUNIT_TEST(testResolveAuthConfig_with_userDefined);
  CPPUNIT_TEST_SUITE_END();
private:
  NetrcHandle _netrc;
  //SharedHandle<Option> _option;
  NetrcAuthResolverHandle _resolver;
public:
  void setUp()
  {
    _netrc = new Netrc();
    _netrc->addAuthenticator(new Authenticator("localhost", "name", "passwd", "account"));
    _netrc->addAuthenticator(new DefaultAuthenticator("default", "defaultpasswd", "defaultaccount"));

    //_option = new Option();
    _resolver = new NetrcAuthResolver();
    _resolver->setNetrc(_netrc);
    _resolver->setDefaultAuthConfig(new AuthConfig("foo", "bar"));
  }

  void testResolveAuthConfig_without_userDefined();
  void testResolveAuthConfig_with_userDefined();
};


CPPUNIT_TEST_SUITE_REGISTRATION( NetrcAuthResolverTest );

void NetrcAuthResolverTest::testResolveAuthConfig_without_userDefined()
{
  AuthConfigHandle authConfig = _resolver->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(string("name:passwd"), authConfig->getAuthText());

  authConfig = _resolver->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(string("default:defaultpasswd"), authConfig->getAuthText());

  _resolver->setNetrc(0);
  authConfig = _resolver->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(string("foo:bar"), authConfig->getAuthText());

}

void NetrcAuthResolverTest::testResolveAuthConfig_with_userDefined()
{
  _resolver->setUserDefinedAuthConfig(new AuthConfig("myname", "mypasswd"));
  AuthConfigHandle authConfig = _resolver->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(string("myname:mypasswd"), authConfig->getAuthText());

  authConfig = _resolver->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(string("myname:mypasswd"), authConfig->getAuthText());

  _resolver->setNetrc(0);
  authConfig = _resolver->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(string("myname:mypasswd"), authConfig->getAuthText());
}
