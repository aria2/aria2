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
  std::unique_ptr<DefaultAuthResolver> resolver_;

public:
  void setUp()
  {
    //_netrc = new Netrc();
    //_option = new Option();
    resolver_.reset(new DefaultAuthResolver());
    //_factory->setOption(_option.get());
    resolver_->setDefaultCred("foo", "bar");
  }

  void testResolveAuthConfig_without_userDefined();
  void testResolveAuthConfig_with_userDefined();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DefaultAuthResolverTest);

void DefaultAuthResolverTest::testResolveAuthConfig_without_userDefined()
{
  auto authConfig = resolver_->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("foo:bar"), authConfig->getAuthText());
}

void DefaultAuthResolverTest::testResolveAuthConfig_with_userDefined()
{
  resolver_->setUserDefinedCred("myname", "mypasswd");
  auto authConfig = resolver_->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"),
                       authConfig->getAuthText());
}

} // namespace aria2
