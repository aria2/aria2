#include "DHTNode.h"
#include "DHTNodeLookupEntry.h"
#include "DHTIDCloser.h"
#include "Exception.h"
#include "util.h"
#include <cstring>
#include <algorithm>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHTIDCloserTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTIDCloserTest);
  CPPUNIT_TEST(testOperator);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testOperator();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTIDCloserTest);

void DHTIDCloserTest::testOperator()
{
  unsigned char id[DHT_ID_LENGTH];
  memset(id, 0xf0, DHT_ID_LENGTH);

  SharedHandle<DHTNodeLookupEntry> e1
    (new DHTNodeLookupEntry(SharedHandle<DHTNode>(new DHTNode(id))));

  id[0] = 0xb0;
  SharedHandle<DHTNodeLookupEntry> e2
    (new DHTNodeLookupEntry(SharedHandle<DHTNode>(new DHTNode(id))));

  id[0] = 0xa0;
  SharedHandle<DHTNodeLookupEntry> e3
    (new DHTNodeLookupEntry(SharedHandle<DHTNode>(new DHTNode(id))));

  id[0] = 0x80;
  SharedHandle<DHTNodeLookupEntry> e4
    (new DHTNodeLookupEntry(SharedHandle<DHTNode>(new DHTNode(id))));

  id[0] = 0x00;
  SharedHandle<DHTNodeLookupEntry> e5
    (new DHTNodeLookupEntry(SharedHandle<DHTNode>(new DHTNode(id))));

  std::deque<SharedHandle<DHTNodeLookupEntry> > entries;
  entries.push_back(e1);
  entries.push_back(e2);
  entries.push_back(e3);
  entries.push_back(e4);
  entries.push_back(e5);

  std::sort(entries.begin(), entries.end(), DHTIDCloser(e3->node->getID()));

  CPPUNIT_ASSERT(*e3 == *entries[0]);
  CPPUNIT_ASSERT(*e2 == *entries[1]);
  CPPUNIT_ASSERT(*e4 == *entries[2]);
  CPPUNIT_ASSERT(*e1 == *entries[3]);
  CPPUNIT_ASSERT(*e5 == *entries[4]);
}

} // namespace aria2
