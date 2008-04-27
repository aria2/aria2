#include "FeatureConfig.h"
#include <cppunit/extensions/HelperMacros.h>

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
  CPPUNIT_ASSERT_EQUAL(
#ifdef ENABLE_ASYNC_DNS
		       std::string("Async DNS, ")
#else
		       std::string()
#endif // ENABLE_ASYNC_DNS
#ifdef ENABLE_BITTORRENT
		       +std::string("BitTorrent, ")
#else
		       +std::string()
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_SSL
		       +std::string("HTTPS, ")
#else
		       +std::string()
#endif // ENABLE_SSL
#ifdef ENABLE_MESSAGE_DIGEST
		       +std::string("Message Digest, ")
#else
		       +std::string()
#endif // ENABLE_MESSAGE_DIGEST
#ifdef ENABLE_METALINK
		       +std::string("Metalink")
#else
		       +std::string()
#endif // ENABLE_METALINK
		       ,
		       FeatureConfig::getInstance()->featureSummary());
}

} // namespace aria2
