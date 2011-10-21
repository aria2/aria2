#include "Option.h"

#include <string>

#include <cppunit/extensions/HelperMacros.h>

#include "prefs.h"

namespace aria2 {

class OptionTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(OptionTest);
  CPPUNIT_TEST(testPutAndGet);
  CPPUNIT_TEST(testPutAndGetAsInt);
  CPPUNIT_TEST(testPutAndGetAsDouble);
  CPPUNIT_TEST(testDefined);
  CPPUNIT_TEST(testBlank);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testPutAndGet();
  void testPutAndGetAsInt();
  void testPutAndGetAsDouble();
  void testDefined();
  void testBlank();
};


CPPUNIT_TEST_SUITE_REGISTRATION( OptionTest );

void OptionTest::testPutAndGet() {
  Option op;
  op.put(PREF_TIMEOUT, "value");
  
  CPPUNIT_ASSERT(op.defined(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL(std::string("value"), op.get(PREF_TIMEOUT));
}

void OptionTest::testPutAndGetAsInt() {
  Option op;
  op.put(PREF_TIMEOUT, "1000");

  CPPUNIT_ASSERT(op.defined(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL((int32_t)1000, op.getAsInt(PREF_TIMEOUT));
}

void OptionTest::testPutAndGetAsDouble() {
  Option op;
  op.put(PREF_TIMEOUT, "10.0");
  
  CPPUNIT_ASSERT_EQUAL(10.0, op.getAsDouble(PREF_TIMEOUT));
}

void OptionTest::testDefined()
{
  Option op;
  op.put(PREF_TIMEOUT, "v");
  op.put(PREF_DIR, "");
  CPPUNIT_ASSERT(op.defined(PREF_TIMEOUT));
  CPPUNIT_ASSERT(op.defined(PREF_DIR));
  CPPUNIT_ASSERT(!op.defined(PREF_DAEMON));
}

void OptionTest::testBlank()
{
  Option op;
  op.put(PREF_TIMEOUT, "v");
  op.put(PREF_DIR, "");
  CPPUNIT_ASSERT(!op.blank(PREF_TIMEOUT));
  CPPUNIT_ASSERT(op.blank(PREF_DIR));
  CPPUNIT_ASSERT(op.blank(PREF_DAEMON));
}

} // namespace aria2
