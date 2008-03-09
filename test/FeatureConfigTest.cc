#include "FeatureConfig.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class FeatureConfigTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(FeatureConfigTest);
  CPPUNIT_TEST(testGetDefaultPort);
  CPPUNIT_TEST(testIsSupported);
  CPPUNIT_TEST(testGetConfigurationSummary);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testGetDefaultPort();
  void testIsSupported();
  void testGetConfigurationSummary();
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
  CPPUNIT_ASSERT_EQUAL(true,
		       FeatureConfig::getInstance()->isSupported("http"));
#ifdef ENABLE_SSL
  CPPUNIT_ASSERT_EQUAL(true,
		       FeatureConfig::getInstance()->isSupported("https"));
#else
  CPPUNIT_ASSERT_EQUAL(false,
		       FeatureConfig::getInstance()->isSupported("https"));
#endif // ENABLE_SSL
  CPPUNIT_ASSERT_EQUAL(true,
		       FeatureConfig::getInstance()->isSupported("ftp"));
  CPPUNIT_ASSERT_EQUAL(false,
		       FeatureConfig::getInstance()->isSupported("ftps"));
}

void FeatureConfigTest::testGetConfigurationSummary() {
  CPPUNIT_ASSERT_EQUAL(std::string("http: yes\n")
#ifdef ENABLE_SSL
		       +"https: yes\n"
#else
		       +"https: no\n"
#endif // ENABLE_SSL
		       +"ftp: yes\n"
#ifdef ENABLE_BITTORRENT
		       +"bittorrent: yes\n"
#else
		       +"bittorrent: no\n"
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
		       +"metalink: yes\n"
#else
		       +"metalink: no\n"
#endif // ENABLE_METALINK
#ifdef ENABLE_MESSAGE_DIGEST
		       +"message digest: yes\n"
#else
		       +"message digest: no\n"
#endif // ENABLE_MESSAGE_DIGEST
#ifdef ENABLE_ASYNC_DNS
		       +"async dns: yes\n"
#else
		       +"async dns: no\n"
#endif // ENABLE_ASYNC_DNS
		       ,
		       FeatureConfig::getInstance()->getConfigurationSummary());
}

} // namespace aria2
