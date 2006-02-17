#include "Base64.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class Base64Test:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Base64Test);
  CPPUNIT_TEST(testEncode);
  CPPUNIT_TEST(testDecode);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testEncode();
  void testDecode();
};


CPPUNIT_TEST_SUITE_REGISTRATION( Base64Test );

void Base64Test::testEncode() {
  CPPUNIT_ASSERT_EQUAL(string("SGVsbG8gV29ybGQh"),
		       Base64::encode("Hello World!"));
}

void Base64Test::testDecode() {
  CPPUNIT_ASSERT_EQUAL(string("Hello World!"),
		       Base64::decode("SGVsbG8gV29ybGQh"));
}
