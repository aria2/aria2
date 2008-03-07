#include "Data.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DataTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DataTest);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST(testGetData);
  CPPUNIT_TEST(testToInt);
  CPPUNIT_TEST(testToLLInt);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testToString();
  void testGetData();
  void testToInt();
  void testToLLInt();
};


CPPUNIT_TEST_SUITE_REGISTRATION( DataTest );

void DataTest::testToString() {
  Data data("aria2", 5);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), data.toString());

  Data null(reinterpret_cast<const char*>(0), 0);
  CPPUNIT_ASSERT_EQUAL(std::string(""), null.toString());
}

void DataTest::testGetData() {
  Data data("aria2", 5);
  CPPUNIT_ASSERT_EQUAL(0, memcmp("aria2", data.getData(), 5));
  CPPUNIT_ASSERT_EQUAL((int32_t)5, data.getLen());  

  Data null(reinterpret_cast<const char*>(0), 0);
  CPPUNIT_ASSERT(null.getData() == 0);
  CPPUNIT_ASSERT_EQUAL((int32_t)0, null.getLen());

}

void DataTest::testToInt() {
  Data data("1000", 4);
  CPPUNIT_ASSERT_EQUAL((int32_t)1000, data.toInt());

  Data null(reinterpret_cast<const char*>(0), 0);
  CPPUNIT_ASSERT_EQUAL((int32_t)0, null.toInt());

  Data alpha("abc", 3);
  CPPUNIT_ASSERT_EQUAL((int32_t)0, alpha.toInt());
}

void DataTest::testToLLInt() {
  Data data("1000", 4);
  CPPUNIT_ASSERT_EQUAL(1000, (int)data.toLLInt());

  Data null(reinterpret_cast<const char*>(0), 0);
  CPPUNIT_ASSERT_EQUAL(0, (int)null.toLLInt());

  Data alpha("abc", 3);
  CPPUNIT_ASSERT_EQUAL(0, (int)alpha.toLLInt());
}

} // namespace aria2
