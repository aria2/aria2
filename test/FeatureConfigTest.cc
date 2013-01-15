#include "FeatureConfig.h"

#include <algorithm>

#include <cppunit/extensions/HelperMacros.h>

#include "a2functional.h"
#include "array_fun.h"
#include "util.h"

namespace aria2 {

class FeatureConfigTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(FeatureConfigTest);
  CPPUNIT_TEST(testGetDefaultPort);
  CPPUNIT_TEST(testStrSupportedFeature);
  CPPUNIT_TEST(testFeatureSummary);
  CPPUNIT_TEST_SUITE_END();
public:
  void testGetDefaultPort();
  void testStrSupportedFeature();
  void testFeatureSummary();
};


CPPUNIT_TEST_SUITE_REGISTRATION(FeatureConfigTest);

void FeatureConfigTest::testGetDefaultPort() {
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, getDefaultPort("http"));
  CPPUNIT_ASSERT_EQUAL((uint16_t)443, getDefaultPort("https"));
  CPPUNIT_ASSERT_EQUAL((uint16_t)21, getDefaultPort("ftp"));
}

void FeatureConfigTest::testStrSupportedFeature() {
  const char* https = strSupportedFeature(FEATURE_HTTPS);
#ifdef ENABLE_SSL
  CPPUNIT_ASSERT(https);
#else
  CPPUNIT_ASSERT(!https);
#endif // ENABLE_SSL
  CPPUNIT_ASSERT(!strSupportedFeature(MAX_FEATURE));
}

void FeatureConfigTest::testFeatureSummary() {
  const std::string features[] = {

#ifdef ENABLE_ASYNC_DNS
    "Async DNS",
#endif // ENABLE_ASYNC_DNS

#ifdef ENABLE_BITTORRENT
    "BitTorrent",
#endif // ENABLE_BITTORRENT

#ifdef HAVE_SQLITE3
    "Firefox3 Cookie",
#endif // HAVE_SQLITE3

#ifdef HAVE_ZLIB
    "GZip",
#endif // HAVE_ZLIB

#ifdef ENABLE_SSL
    "HTTPS",
#endif // ENABLE_SSL

#ifdef ENABLE_MESSAGE_DIGEST
    "Message Digest",
#endif // ENABLE_MESSAGE_DIGEST

#ifdef ENABLE_METALINK
    "Metalink",
#endif // ENABLE_METALINK

#ifdef ENABLE_XML_RPC
    "XML-RPC",
#endif // ENABLE_XML_RPC

  };

  std::string featuresString = strjoin(vbegin(features), vend(features), ", ");
  CPPUNIT_ASSERT_EQUAL(featuresString, featureSummary());
}

} // namespace aria2
