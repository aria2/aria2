#include "XORCloser.h"
#include "Exception.h"
#include "util.h"
#include "DHTNodeLookupEntry.h"
#include "DHTNode.h"
#include <cstring>
#include <algorithm>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class XORCloserTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(XORCloserTest);
  CPPUNIT_TEST(testOperator);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testOperator();
};


CPPUNIT_TEST_SUITE_REGISTRATION(XORCloserTest);

void XORCloserTest::testOperator()
{
  const size_t NUM_KEY = 6;
  unsigned char keys[NUM_KEY][DHT_ID_LENGTH];
  memset(keys, 0, 6*DHT_ID_LENGTH);

  keys[0][0] = 0xf0;
  keys[1][0] = 0xb0;
  keys[2][0] = 0xa0;
  keys[3][0] = 0x80;
  keys[4][0] = 0x00;
  keys[4][DHT_ID_LENGTH-1] = 0x01;
  keys[5][0] = 0x00;

  std::deque<unsigned char*> l(&keys[0], &keys[NUM_KEY]);

  std::sort(l.begin(), l.end(), XORCloser(keys[2], DHT_ID_LENGTH));

  CPPUNIT_ASSERT(memcmp(keys[2], l[0], DHT_ID_LENGTH) == 0);
  CPPUNIT_ASSERT(memcmp(keys[1], l[1], DHT_ID_LENGTH) == 0);
  CPPUNIT_ASSERT(memcmp(keys[3], l[2], DHT_ID_LENGTH) == 0);
  CPPUNIT_ASSERT(memcmp(keys[0], l[3], DHT_ID_LENGTH) == 0);
  CPPUNIT_ASSERT(memcmp(keys[5], l[4], DHT_ID_LENGTH) == 0);
  CPPUNIT_ASSERT(memcmp(keys[4], l[5], DHT_ID_LENGTH) == 0);
}

} // namespace aria2
