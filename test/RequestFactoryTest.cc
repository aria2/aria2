#include "RequestFactory.h"
#include "prefs.h"
#include "NetrcAuthResolver.h"
#include "DefaultAuthResolver.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class RequestFactoryTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RequestFactoryTest);
  CPPUNIT_TEST(testCreateHttpAuthResolver_netrc);
  CPPUNIT_TEST(testCreateHttpAuthResolver_def);
  CPPUNIT_TEST(testCreateFtpAuthResolver_netrc);
  CPPUNIT_TEST(testCreateFtpAuthResolver_def);
  CPPUNIT_TEST(testCreateHttpProxyAuthResolver_netrc);
  CPPUNIT_TEST(testCreateHttpProxyAuthResolver_def);
  CPPUNIT_TEST_SUITE_END();
private:
  NetrcHandle _netrc;
  SharedHandle<Option> _option;
  RequestFactoryHandle _factory;
public:
  void setUp()
  {
    _netrc = new Netrc();
    _option = new Option();
    _factory = new RequestFactory();
    _factory->setNetrc(_netrc);
    _factory->setOption(_option.get());
  }

  void testCreateHttpAuthResolver_netrc();
  void testCreateHttpAuthResolver_def();
  void testCreateFtpAuthResolver_netrc();
  void testCreateFtpAuthResolver_def();
  void testCreateHttpProxyAuthResolver_netrc();
  void testCreateHttpProxyAuthResolver_def();
};


CPPUNIT_TEST_SUITE_REGISTRATION( RequestFactoryTest );

void RequestFactoryTest::testCreateHttpAuthResolver_netrc()
{
  _option->put(PREF_NO_NETRC, V_FALSE);
  _option->put(PREF_HTTP_USER, "foo");
  _option->put(PREF_HTTP_PASSWD, "bar");
  DefaultAuthResolverHandle defResolver = _factory->createHttpAuthResolver();
  CPPUNIT_ASSERT(!defResolver.isNull());  
  CPPUNIT_ASSERT(!defResolver->getUserDefinedAuthConfig().isNull());
  CPPUNIT_ASSERT_EQUAL(string("foo:bar"),
		       defResolver->getUserDefinedAuthConfig()->getAuthText());
  /*
  NetrcAuthResolverHandle netrcResolver = _factory->createHttpAuthResolver();
  CPPUNIT_ASSERT(!netrcResolver.isNull());
  CPPUNIT_ASSERT(!netrcResolver->getNetrc().isNull());
  CPPUNIT_ASSERT(netrcResolver->getUserDefinedAuthConfig().isNull());
  */
}

void RequestFactoryTest::testCreateHttpAuthResolver_def()
{
  _option->put(PREF_NO_NETRC, V_TRUE);
  _option->put(PREF_HTTP_USER, "foo");
  _option->put(PREF_HTTP_PASSWD, "bar");
  DefaultAuthResolverHandle defResolver = _factory->createHttpAuthResolver();
  CPPUNIT_ASSERT(!defResolver.isNull());  
  CPPUNIT_ASSERT(!defResolver->getUserDefinedAuthConfig().isNull());
  CPPUNIT_ASSERT_EQUAL(string("foo:bar"),
		       defResolver->getUserDefinedAuthConfig()->getAuthText());
}

void RequestFactoryTest::testCreateFtpAuthResolver_netrc()
{
  _option->put(PREF_NO_NETRC, V_FALSE);
  NetrcAuthResolverHandle netrcResolver = _factory->createFtpAuthResolver();
  CPPUNIT_ASSERT(!netrcResolver.isNull());
  CPPUNIT_ASSERT(!netrcResolver->getNetrc().isNull());
  CPPUNIT_ASSERT(netrcResolver->getUserDefinedAuthConfig().isNull());
  CPPUNIT_ASSERT_EQUAL(string("anonymous:ARIA2USER@"),
		       netrcResolver->getDefaultAuthConfig()->getAuthText());
}

void RequestFactoryTest::testCreateFtpAuthResolver_def()
{
  _option->put(PREF_NO_NETRC, V_TRUE);
  _option->put(PREF_FTP_USER, "foo");
  _option->put(PREF_FTP_PASSWD, "bar");
  DefaultAuthResolverHandle defResolver = _factory->createFtpAuthResolver();
  CPPUNIT_ASSERT(!defResolver.isNull());  
  CPPUNIT_ASSERT(!defResolver->getUserDefinedAuthConfig().isNull());
  CPPUNIT_ASSERT_EQUAL(string("foo:bar"),
		       defResolver->getUserDefinedAuthConfig()->getAuthText());
  CPPUNIT_ASSERT_EQUAL(string("anonymous:ARIA2USER@"),
		       defResolver->getDefaultAuthConfig()->getAuthText());
}

void RequestFactoryTest::testCreateHttpProxyAuthResolver_netrc()
{
  _option->put(PREF_NO_NETRC, V_FALSE);
  _option->put(PREF_HTTP_PROXY_USER, "foo");
  _option->put(PREF_HTTP_PROXY_PASSWD, "bar");
  DefaultAuthResolverHandle defResolver = _factory->createHttpProxyAuthResolver();
  CPPUNIT_ASSERT(!defResolver.isNull());  
  CPPUNIT_ASSERT(!defResolver->getUserDefinedAuthConfig().isNull());
  CPPUNIT_ASSERT_EQUAL(string("foo:bar"),
		       defResolver->getUserDefinedAuthConfig()->getAuthText());
  /*
  NetrcAuthResolverHandle netrcResolver = _factory->createHttpProxyAuthResolver();
  CPPUNIT_ASSERT(!netrcResolver.isNull());
  CPPUNIT_ASSERT(!netrcResolver->getNetrc().isNull());
  CPPUNIT_ASSERT(netrcResolver->getUserDefinedAuthConfig().isNull());
  */
}

void RequestFactoryTest::testCreateHttpProxyAuthResolver_def()
{
  _option->put(PREF_NO_NETRC, V_TRUE);
  _option->put(PREF_HTTP_PROXY_USER, "foo");
  _option->put(PREF_HTTP_PROXY_PASSWD, "bar");
  DefaultAuthResolverHandle defResolver = _factory->createHttpProxyAuthResolver();
  CPPUNIT_ASSERT(!defResolver.isNull());  
  CPPUNIT_ASSERT(!defResolver->getUserDefinedAuthConfig().isNull());
  CPPUNIT_ASSERT_EQUAL(string("foo:bar"),
		       defResolver->getUserDefinedAuthConfig()->getAuthText());
}

