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
  CPPUNIT_TEST(testIsSupported);
  CPPUNIT_TEST(testFeatureSummary);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testGetDefaultPort();
  void testIsSupported();
  void testFeatureSummary();
};


CPPUNIT_TEST_SUITE_REGISTRATION(FeatureConfigTest);

void FeatureConfigTest::testGetDefaultPort() {
  CPPUNIT_ASSERT_EQUAL((uint16_t)80,
                       FeatureConfig::getInstance()->getDefaultPort("http"));
  CPPUNIT_ASSERT_EQUAL((uint16_t)443,
                       FeatureConfig::getInstance()->getDefaultPort("https"));
  CPPUNIT_ASSERT_EQUAL((uint16_t)21,
                       FeatureConfig::getInstance()->getDefaultPort("ftp"));
}

void FeatureConfigTest::testIsSupported() {
#ifdef ENABLE_SSL
  CPPUNIT_ASSERT_EQUAL(true,
                       FeatureConfig::getInstance()->isSupported
                       (FeatureConfig::FEATURE_HTTPS));
#else
  CPPUNIT_ASSERT_EQUAL(false,
                       FeatureConfig::getInstance()->isSupported
                       (FeatureConfig::FEATURE_HTTPS));
#endif // ENABLE_SSL
  CPPUNIT_ASSERT_EQUAL(false,
                       FeatureConfig::getInstance()->isSupported("FTPS"));
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

  std::string featuresString;
  const std::string delim(", ");
  std::for_each(vbegin(features), vend(features),
                StringAppend(featuresString, delim));
  featuresString = util::strip(featuresString, delim);
  
  CPPUNIT_ASSERT_EQUAL(featuresString,
                       FeatureConfig::getInstance()->featureSummary());
}

} // namespace aria2
