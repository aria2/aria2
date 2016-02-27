#include "NetrcAuthResolver.h"

#include <cppunit/extensions/HelperMacros.h>

#include "prefs.h"
#include "Netrc.h"
#include "AuthConfig.h"
#include "a2functional.h"

namespace aria2 {

class NetrcAuthResolverTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(NetrcAuthResolverTest);
  CPPUNIT_TEST(testResolveAuthConfig_without_userDefined);
  CPPUNIT_TEST(testResolveAuthConfig_with_userDefined);
  CPPUNIT_TEST(testResolveAuthConfig_ignoreDefault);
  CPPUNIT_TEST_SUITE_END();

private:
  std::unique_ptr<Netrc> netrc_;
  std::unique_ptr<NetrcAuthResolver> resolver_;

public:
  void setUp()
  {
    netrc_.reset(new Netrc());
    netrc_->addAuthenticator(
        make_unique<Authenticator>("localhost", "name", "passwd", "account"));
    netrc_->addAuthenticator(make_unique<DefaultAuthenticator>(
        "default", "defaultpasswd", "defaultaccount"));

    resolver_.reset(new NetrcAuthResolver());
    resolver_->setNetrc(netrc_.get());
    resolver_->setDefaultCred("foo", "bar");
  }

  void testResolveAuthConfig_without_userDefined();
  void testResolveAuthConfig_with_userDefined();
  void testResolveAuthConfig_ignoreDefault();
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetrcAuthResolverTest);

void NetrcAuthResolverTest::testResolveAuthConfig_without_userDefined()
{
  auto authConfig = resolver_->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("name:passwd"), authConfig->getAuthText());

  authConfig = resolver_->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(std::string("default:defaultpasswd"),
                       authConfig->getAuthText());

  resolver_->setNetrc(nullptr);
  authConfig = resolver_->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("foo:bar"), authConfig->getAuthText());
}

void NetrcAuthResolverTest::testResolveAuthConfig_with_userDefined()
{
  resolver_->setUserDefinedCred("myname", "mypasswd");
  auto authConfig = resolver_->resolveAuthConfig("localhost");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"),
                       authConfig->getAuthText());

  authConfig = resolver_->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"),
                       authConfig->getAuthText());

  resolver_->setNetrc(nullptr);
  authConfig = resolver_->resolveAuthConfig("mymachine");
  CPPUNIT_ASSERT_EQUAL(std::string("myname:mypasswd"),
                       authConfig->getAuthText());
}

void NetrcAuthResolverTest::testResolveAuthConfig_ignoreDefault()
{
  resolver_->ignoreDefault();
  auto authConfig = resolver_->resolveAuthConfig("mirror");
  CPPUNIT_ASSERT_EQUAL(std::string("foo:bar"), authConfig->getAuthText());

  resolver_->useDefault();
  auto defAuthConfig = resolver_->resolveAuthConfig("mirror");
  CPPUNIT_ASSERT_EQUAL(std::string("default:defaultpasswd"),
                       defAuthConfig->getAuthText());
}

} // namespace aria2
