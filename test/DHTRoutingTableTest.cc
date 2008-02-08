#include "DHTRoutingTable.h"
#include "Exception.h"
#include "Util.h"
#include "DHTNode.h"
#include "DHTBucket.h"
#include "MockDHTTaskQueue.h"
#include "MockDHTTaskFactory.h"
#include "DHTTask.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHTRoutingTableTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTRoutingTableTest);
  CPPUNIT_TEST(testAddNode);
  CPPUNIT_TEST(testGetClosestKNodes);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testAddNode();
  void testGetClosestKNodes();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTRoutingTableTest);

void DHTRoutingTableTest::testAddNode()
{
  DHTRoutingTable table(new DHTNode());
  table.setTaskFactory(new MockDHTTaskFactory());
  table.setTaskQueue(new MockDHTTaskQueue());
  uint32_t count = 0;
  for(int i = 0; i < 100; ++i) {
    if(table.addNode(new DHTNode())) {
      ++count;
    }
  }
  table.showBuckets();
}

static void createID(unsigned char* id, unsigned char firstChar, unsigned char lastChar)
{
  memset(id, 0, DHT_ID_LENGTH);
  id[0] = firstChar;
  id[DHT_ID_LENGTH-1] = lastChar;
}

void DHTRoutingTableTest::testGetClosestKNodes()
{
  unsigned char id[DHT_ID_LENGTH];
  createID(id, 0x81, 0);
  SharedHandle<DHTNode> localNode = new DHTNode(id);

  DHTRoutingTable table(localNode);

  SharedHandle<DHTNode> nodes1[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  SharedHandle<DHTNode> nodes2[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  SharedHandle<DHTNode> nodes3[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  for(size_t i = 0; i < DHTBucket::K; ++i) {
    createID(id, 0xf0, i);
    nodes1[i] = new DHTNode(id);
    CPPUNIT_ASSERT(table.addNode(nodes1[i]));
  }
  for(size_t i = 0; i < DHTBucket::K; ++i) {
    createID(id, 0x80, i);
    nodes2[i] = new DHTNode(id);
    CPPUNIT_ASSERT(table.addNode(nodes2[i]));
  }
  for(size_t i = 0; i < DHTBucket::K; ++i) {
    createID(id, 0x70, i);
    nodes3[i] = new DHTNode(id);
    CPPUNIT_ASSERT(table.addNode(nodes3[i]));
  }
  {
    createID(id, 0x80, 0x10);
    std::deque<SharedHandle<DHTNode> > nodes = table.getClosestKNodes(id);
    CPPUNIT_ASSERT_EQUAL((size_t)8, nodes.size());
    for(size_t i = 0; i < nodes.size(); ++i) {
      CPPUNIT_ASSERT(memcmp(nodes2[0]->getID(), nodes[0]->getID(), DHT_ID_LENGTH) == 0);
    }
  }
}

} // namespace aria2
