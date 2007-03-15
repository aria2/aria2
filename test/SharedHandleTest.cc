#include "SharedHandle.h"
#include "common.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

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
  cerr << "xh:" << endl;
  SharedHandle<int> xh = new int(1);

  CPPUNIT_ASSERT_EQUAL(1, xh.getRefCount()->totalRefCount);
  CPPUNIT_ASSERT_EQUAL(1, xh.getRefCount()->strongRefCount);

  cerr << "nullHandle:" << endl;
  SharedHandle<int> nullHandle = 0;

  CPPUNIT_ASSERT_EQUAL(1, nullHandle.getRefCount()->totalRefCount);
  CPPUNIT_ASSERT_EQUAL(1, nullHandle.getRefCount()->strongRefCount);

  cerr << "staticHandle:" << endl;
  CPPUNIT_ASSERT_EQUAL(1, staticHandle.getRefCount()->totalRefCount);
  CPPUNIT_ASSERT_EQUAL(1, staticHandle.getRefCount()->strongRefCount);

  SharedHandle<int> localStaticHandle = getInstance();

  CPPUNIT_ASSERT_EQUAL(2, localStaticHandle.getRefCount()->totalRefCount);
  CPPUNIT_ASSERT_EQUAL(2, localStaticHandle.getRefCount()->strongRefCount);
}
