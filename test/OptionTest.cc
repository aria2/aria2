#include "Option.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class OptionTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(OptionTest);
  CPPUNIT_TEST(testPutAndGet);
  CPPUNIT_TEST(testPutAndGetAsInt);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testPutAndGet();
  void testPutAndGetAsInt();
};


CPPUNIT_TEST_SUITE_REGISTRATION( OptionTest );

void OptionTest::testPutAndGet() {
  Option op;
  op.put("key", "value");
  
  CPPUNIT_ASSERT(op.defined("key"));
  CPPUNIT_ASSERT_EQUAL(string("value"), op.get("key"));
}

void OptionTest::testPutAndGetAsInt() {
  Option op;
  op.put("key", "1000");

  CPPUNIT_ASSERT(op.defined("key"));
  CPPUNIT_ASSERT_EQUAL(1000, op.getAsInt("key"));
}
