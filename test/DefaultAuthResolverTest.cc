#include "DefaultAuthResolver.h"
#include "prefs.h"
#include "AuthConfig.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DefaultAuthResolverTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultAuthResolverTest);
  CPPUNIT_TEST(testResolveAuthConfig_without_userDefined);
  CPPUNIT_TEST(testResolveAuthConfig_with_userDefined);
  CPPUNIT_TEST_SUITE_END();
private:
  //NetrcHandle _netrc;
  //SharedHandle<Option> _option;
  SharedHandle<DefaultAuthResolver> _resolver;
public:
  void setUp()
  {
    //_netrc = new Netrc();
    //_option = new Option();
    _resolver = new DefaultAuthResolver();
    //_factory->setOption(_option.get());
    _resolver->setDefaultAuthConfig(new AuthConfig("foo", "bar"));
  }

  void testResolveAuthConfig_without_userDefined();
  void testResolveAuthConfig_with_userDefined();
};


CPPUNIT_TEST_SUITE_REGISTRATION( DefaultAuthResolverTest );

void DefaultAuthResolverTest::testResolveAuthConfig_without_userDefined()
{
  SharedHandle<AuthConfig> authConfig = _resolver->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("foo:bar"), authConfig->getAuthText());
}

void DefaultAuthResolverTest::testResolveAuthConfig_with_userDefined()
{
  _resolver->setUserDefinedAuthConfig(new AuthConfig("myname", "mypasswd"));
  SharedHandle<AuthConfig> authConfig = _resolver->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"), authConfig->getAuthText());
}

} // namespace aria2
