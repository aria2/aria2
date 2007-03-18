#include "AuthConfig.h"
#include "Option.h"
#include "prefs.h"
#include <cppunit/extensions/HelperMacros.h>

class AuthConfigTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(AuthConfigTest);
  CPPUNIT_TEST(testGet);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testGet();
};


CPPUNIT_TEST_SUITE_REGISTRATION( AuthConfigTest );

void AuthConfigTest::testGet()
{
  Option option;
  option.put(PREF_HTTP_USER, "httpUser");
  option.put(PREF_HTTP_PASSWD, "httpPassword");
  option.put(PREF_FTP_USER, "ftpUser");
  option.put(PREF_FTP_PASSWD, "ftpPassword");
  option.put(PREF_HTTP_PROXY_USER, "httpProxyUser");
  option.put(PREF_HTTP_PROXY_PASSWD, "httpProxyPassword");

  AuthConfig authConfig;
  authConfig.configure(&option);

  AuthConfigItemHandle httpAuth = authConfig.getHttpAuthConfigItem();
  CPPUNIT_ASSERT_EQUAL(string("httpUser"), httpAuth->getUser());
  CPPUNIT_ASSERT_EQUAL(string("httpPassword"), httpAuth->getPassword());

  AuthConfigItemHandle ftpAuth = authConfig.getFtpAuthConfigItem();
  CPPUNIT_ASSERT_EQUAL(string("ftpUser"), ftpAuth->getUser());
  CPPUNIT_ASSERT_EQUAL(string("ftpPassword"), ftpAuth->getPassword());

  AuthConfigItemHandle httpProxyAuth = authConfig.getHttpProxyAuthConfigItem();
  CPPUNIT_ASSERT_EQUAL(string("httpProxyUser"), httpProxyAuth->getUser());
  CPPUNIT_ASSERT_EQUAL(string("httpProxyPassword"), httpProxyAuth->getPassword());
}
