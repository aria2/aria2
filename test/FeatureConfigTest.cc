#include "FeatureConfig.h"

#include <cppunit/extensions/HelperMacros.h>

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
  CPPUNIT_ASSERT_EQUAL(80,
		       FeatureConfig::getInstance()->getDefaultPort("http"));
  CPPUNIT_ASSERT_EQUAL(443,
		       FeatureConfig::getInstance()->getDefaultPort("https"));
  CPPUNIT_ASSERT_EQUAL(21,
		       FeatureConfig::getInstance()->getDefaultPort("ftp"));
}

void FeatureConfigTest::testIsSupported() {
  CPPUNIT_ASSERT_EQUAL(true,
		       FeatureConfig::getInstance()->isSupported("http"));
  CPPUNIT_ASSERT_EQUAL(true,
		       FeatureConfig::getInstance()->isSupported("https"));
  CPPUNIT_ASSERT_EQUAL(true,
		       FeatureConfig::getInstance()->isSupported("ftp"));
  CPPUNIT_ASSERT_EQUAL(false,
		       FeatureConfig::getInstance()->isSupported("ftps"));
}

void FeatureConfigTest::testGetConfigurationSummary() {
  CPPUNIT_ASSERT_EQUAL(string("http: yes\n")
		       +"https: yes\n"
		       +"ftp: yes\n"
		       +"bittorrent: yes\n"
		       +"metalink: yes\n"
		       +"message digest: yes\n",
		       FeatureConfig::getInstance()->getConfigurationSummary());
}
