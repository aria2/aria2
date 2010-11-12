#include "common.h"
#include "SharedHandle.h"
#include <string>
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class SharedHandleTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SharedHandleTest);
  CPPUNIT_TEST(testSharedHandle);
  CPPUNIT_TEST(testWeakHandle);
  CPPUNIT_TEST_SUITE_END();

  static SharedHandle<int> staticHandle;
public:
  void setUp() {
  }

  static SharedHandle<int> getInstance() {
    if(!staticHandle) {
      staticHandle.reset(new int(1));
    }
    return staticHandle;
  }

  void testSharedHandle();
  void testWeakHandle();
};

SharedHandle<int> SharedHandleTest::staticHandle;

CPPUNIT_TEST_SUITE_REGISTRATION( SharedHandleTest );

void SharedHandleTest::testSharedHandle() {
  std::cout << "xh:" << std::endl;
  SharedHandle<int> xh(new int(1));

  CPPUNIT_ASSERT_EQUAL((size_t)1, xh.getRefCount());

  std::cout << "nullHandle:" << std::endl;
  SharedHandle<int> nullHandle;

  CPPUNIT_ASSERT_EQUAL((size_t)1, nullHandle.getRefCount());

  std::cout << "staticHandle:" << std::endl;
  CPPUNIT_ASSERT_EQUAL((size_t)1, staticHandle.getRefCount());

  SharedHandle<int> localStaticHandle = getInstance();

  CPPUNIT_ASSERT_EQUAL((size_t)2, localStaticHandle.getRefCount());
}

void SharedHandleTest::testWeakHandle()
{
  SharedHandle<int> x;
  x.reset(new int(1));

  WeakHandle<int> y = x;

  WeakHandle<int> z;
  z = y;

  std::cout << "z.getRefCount() = " << z.getRefCount() << std::endl;
  y.reset();
  z.reset();

  std::cout << "z.getRefCount() = " << z.getRefCount() << std::endl;

  std::cout << "x.getRefCount() = " << x.getRefCount() << std::endl;

  SharedHandle<int> w;

  x = w;

  std::cout << "w.getRefCount() = " << w.getRefCount() << std::endl;
  std::cout << "x.getRefCount() = " << x.getRefCount() << std::endl;
}

} // namespace aria2
