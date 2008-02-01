#include "DHTNode.h"
#include "DHTNodeLookupEntry.h"
#include "DHTIDCloser.h"
#include "Exception.h"
#include "Util.h"
#include <cppunit/extensions/HelperMacros.h>

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

  DHTNodeLookupEntryHandle e1 = new DHTNodeLookupEntry(new DHTNode(id));

  id[0] = 0xb0;
  DHTNodeLookupEntryHandle e2 = new DHTNodeLookupEntry(new DHTNode(id));

  id[0] = 0xa0;
  DHTNodeLookupEntryHandle e3 = new DHTNodeLookupEntry(new DHTNode(id));

  id[0] = 0x80;
  DHTNodeLookupEntryHandle e4 = new DHTNodeLookupEntry(new DHTNode(id));

  id[0] = 0x00;
  DHTNodeLookupEntryHandle e5 = new DHTNodeLookupEntry(new DHTNode(id));

  DHTNodeLookupEntries entries;
  entries.push_back(e1);
  entries.push_back(e2);
  entries.push_back(e3);
  entries.push_back(e4);
  entries.push_back(e5);

  std::sort(entries.begin(), entries.end(), DHTIDCloser(e3->_node->getID()));

  CPPUNIT_ASSERT(e3 == entries[0]);
  CPPUNIT_ASSERT(e2 == entries[1]);
  CPPUNIT_ASSERT(e4 == entries[2]);
  CPPUNIT_ASSERT(e1 == entries[3]);
  CPPUNIT_ASSERT(e5 == entries[4]);
}
