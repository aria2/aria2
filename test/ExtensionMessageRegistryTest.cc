#include "ExtensionMessageRegistry.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class ExtensionMessageRegistryTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ExtensionMessageRegistryTest);
  CPPUNIT_TEST(testStrBtExtension);
  CPPUNIT_TEST(testKeyBtExtension);
  CPPUNIT_TEST(testGetExtensionMessageID);
  CPPUNIT_TEST_SUITE_END();

public:
  void testStrBtExtension();
  void testKeyBtExtension();
  void testGetExtensionMessageID();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ExtensionMessageRegistryTest);

void ExtensionMessageRegistryTest::testStrBtExtension()
{
  CPPUNIT_ASSERT_EQUAL(
      std::string("ut_pex"),
      std::string(strBtExtension(ExtensionMessageRegistry::UT_PEX)));
  CPPUNIT_ASSERT_EQUAL(
      std::string("ut_metadata"),
      std::string(strBtExtension(ExtensionMessageRegistry::UT_METADATA)));
  CPPUNIT_ASSERT(!strBtExtension(100));
}

void ExtensionMessageRegistryTest::testKeyBtExtension()
{
  CPPUNIT_ASSERT_EQUAL((int)ExtensionMessageRegistry::UT_PEX,
                       keyBtExtension("ut_pex"));
  CPPUNIT_ASSERT_EQUAL((int)ExtensionMessageRegistry::UT_METADATA,
                       keyBtExtension("ut_metadata"));
  CPPUNIT_ASSERT_EQUAL((int)ExtensionMessageRegistry::MAX_EXTENSION,
                       keyBtExtension("unknown"));
}

void ExtensionMessageRegistryTest::testGetExtensionMessageID()
{
  ExtensionMessageRegistry reg;
  CPPUNIT_ASSERT_EQUAL(
      (uint8_t)0, reg.getExtensionMessageID(ExtensionMessageRegistry::UT_PEX));
  CPPUNIT_ASSERT(!reg.getExtensionName(0));
  CPPUNIT_ASSERT(!reg.getExtensionName(1));
  CPPUNIT_ASSERT(!reg.getExtensionName(100));

  reg.setExtensionMessageID(ExtensionMessageRegistry::UT_PEX, 1);

  CPPUNIT_ASSERT_EQUAL(std::string("ut_pex"),
                       std::string(reg.getExtensionName(1)));
  CPPUNIT_ASSERT_EQUAL(
      (uint8_t)1, reg.getExtensionMessageID(ExtensionMessageRegistry::UT_PEX));

  reg.setExtensionMessageID(ExtensionMessageRegistry::UT_METADATA, 127);

  CPPUNIT_ASSERT_EQUAL(
      (uint8_t)127,
      reg.getExtensionMessageID(ExtensionMessageRegistry::UT_METADATA));
  CPPUNIT_ASSERT_EQUAL(
      (uint8_t)1, reg.getExtensionMessageID(ExtensionMessageRegistry::UT_PEX));

  reg.removeExtension(ExtensionMessageRegistry::UT_PEX);

  CPPUNIT_ASSERT_EQUAL(
      (uint8_t)127,
      reg.getExtensionMessageID(ExtensionMessageRegistry::UT_METADATA));
  CPPUNIT_ASSERT_EQUAL(
      (uint8_t)0, reg.getExtensionMessageID(ExtensionMessageRegistry::UT_PEX));
  CPPUNIT_ASSERT(!reg.getExtensionName(1));
}

} // namespace aria2
