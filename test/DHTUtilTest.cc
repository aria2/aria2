#include "DHTUtil.h"
#include "Exception.h"
#include "Util.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHTUtilTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTUtilTest);
  CPPUNIT_TEST(testGenerateRandomData);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGenerateRandomData();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTUtilTest);

void DHTUtilTest::testGenerateRandomData()
{
  unsigned char data1[20];
  DHTUtil::generateRandomData(data1, sizeof(data1));
  unsigned char data2[20];
  DHTUtil::generateRandomData(data2, sizeof(data2));
  CPPUNIT_ASSERT(memcmp(data1, data2, sizeof(data1)) != 0);
}

} // namespace aria2
