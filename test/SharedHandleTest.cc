#include "common.h"
#include "SharedHandle.h"
#include <string>
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class SharedHandleTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SharedHandleTest);
  CPPUNIT_TEST(testSharedHandle);
  CPPUNIT_TEST_SUITE_END();

  static SharedHandle<int> staticHandle;
public:
  void setUp() {
  }

  static SharedHandle<int> getInstance() {
    if(staticHandle.isNull()) {
      staticHandle = new int(1);
    }
    return staticHandle;
  }

  void testSharedHandle();
};

SharedHandle<int> SharedHandleTest::staticHandle = 0;

CPPUNIT_TEST_SUITE_REGISTRATION( SharedHandleTest );

void SharedHandleTest::testSharedHandle() {
  std::cerr << "xh:" << std::endl;
  SharedHandle<int> xh = new int(1);

  CPPUNIT_ASSERT_EQUAL((int32_t)1, xh.getRefCount()->totalRefCount);
  CPPUNIT_ASSERT_EQUAL((int32_t)1, xh.getRefCount()->strongRefCount);

  std::cerr << "nullHandle:" << std::endl;
  SharedHandle<int> nullHandle = 0;

  CPPUNIT_ASSERT_EQUAL((int32_t)1, nullHandle.getRefCount()->totalRefCount);
  CPPUNIT_ASSERT_EQUAL((int32_t)1, nullHandle.getRefCount()->strongRefCount);

  std::cerr << "staticHandle:" << std::endl;
  CPPUNIT_ASSERT_EQUAL((int32_t)1, staticHandle.getRefCount()->totalRefCount);
  CPPUNIT_ASSERT_EQUAL((int32_t)1, staticHandle.getRefCount()->strongRefCount);

  SharedHandle<int> localStaticHandle = getInstance();

  CPPUNIT_ASSERT_EQUAL((int32_t)2, localStaticHandle.getRefCount()->totalRefCount);
  CPPUNIT_ASSERT_EQUAL((int32_t)2, localStaticHandle.getRefCount()->strongRefCount);
}

} // namespace aria2
