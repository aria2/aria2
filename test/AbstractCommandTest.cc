#include "AbstractCommand.h"

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

#include "Option.h"
#include "prefs.h"
#include "SocketCore.h"
#include "SocketRecvBuffer.h"

namespace aria2 {

class AbstractCommandTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(AbstractCommandTest);
  CPPUNIT_TEST(testGetProxyUri);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testGetProxyUri();
};

CPPUNIT_TEST_SUITE_REGISTRATION(AbstractCommandTest);

void AbstractCommandTest::testGetProxyUri()
{
  Option op;
  CPPUNIT_ASSERT_EQUAL(std::string(), getProxyUri("http", &op));

  op.put(PREF_HTTP_PROXY, "http://hu:hp@httpproxy/");
  op.put(PREF_FTP_PROXY, "ftp://fu:fp@ftpproxy/");
  CPPUNIT_ASSERT_EQUAL(std::string("http://hu:hp@httpproxy/"),
                       getProxyUri("http", &op));
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://fu:fp@ftpproxy/"),
                       getProxyUri("ftp", &op));

  op.put(PREF_ALL_PROXY, "http://au:ap@allproxy/");
  CPPUNIT_ASSERT_EQUAL(std::string("http://au:ap@allproxy/"),
                       getProxyUri("https", &op));

  op.put(PREF_ALL_PROXY_USER, "aunew");
  op.put(PREF_ALL_PROXY_PASSWD, "apnew");
  CPPUNIT_ASSERT_EQUAL(std::string("http://aunew:apnew@allproxy/"),
                       getProxyUri("https", &op));

  op.put(PREF_HTTPS_PROXY, "http://hsu:hsp@httpsproxy/");
  CPPUNIT_ASSERT_EQUAL(std::string("http://hsu:hsp@httpsproxy/"),
                       getProxyUri("https", &op));

  CPPUNIT_ASSERT_EQUAL(std::string(), getProxyUri("unknown", &op));

  op.put(PREF_HTTP_PROXY_USER, "hunew");
  CPPUNIT_ASSERT_EQUAL(std::string("http://hunew:hp@httpproxy/"),
                       getProxyUri("http", &op));

  op.put(PREF_HTTP_PROXY_PASSWD, "hpnew");
  CPPUNIT_ASSERT_EQUAL(std::string("http://hunew:hpnew@httpproxy/"),
                       getProxyUri("http", &op));

  op.put(PREF_HTTP_PROXY_USER, "");
  CPPUNIT_ASSERT_EQUAL(std::string("http://httpproxy/"),
                       getProxyUri("http", &op));
}

} // namespace aria2
