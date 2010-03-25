#include "DHTRoutingTableDeserializer.h"

#include <cstring>
#include <sstream>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "DHTRoutingTableSerializer.h"
#include "Exception.h"
#include "util.h"
#include "DHTNode.h"
#include "array_fun.h"
#include "DHTConstants.h"
#include "a2netcompat.h"

namespace aria2 {

class DHTRoutingTableDeserializerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTRoutingTableDeserializerTest);
  CPPUNIT_TEST(testDeserialize);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testDeserialize();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTRoutingTableDeserializerTest);

void DHTRoutingTableDeserializerTest::testDeserialize()
{
  SharedHandle<DHTNode> localNode(new DHTNode());

  SharedHandle<DHTNode> nodesSrc[3];
  for(size_t i = 0; i < arrayLength(nodesSrc); ++i) {
    nodesSrc[i].reset(new DHTNode());
    nodesSrc[i]->setIPAddress("192.168.0."+util::uitos(i+1));
    nodesSrc[i]->setPort(6881+i);
  }
  nodesSrc[1]->setIPAddress("non-numerical-name");
  std::vector<SharedHandle<DHTNode> > nodes(vbegin(nodesSrc), vend(nodesSrc));
  
  DHTRoutingTableSerializer s;
  s.setLocalNode(localNode);
  s.setNodes(nodes);

  std::stringstream ss;
  s.serialize(ss);

  DHTRoutingTableDeserializer d;
  d.deserialize(ss);

  CPPUNIT_ASSERT(memcmp(localNode->getID(), d.getLocalNode()->getID(),
                        DHT_ID_LENGTH) == 0);

  std::cout << d.getSerializedTime().getTime() << std::endl;

  CPPUNIT_ASSERT_EQUAL((size_t)2, d.getNodes().size());
  const std::vector<SharedHandle<DHTNode> >& dsnodes = d.getNodes();
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), dsnodes[0]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, dsnodes[0]->getPort());
  CPPUNIT_ASSERT(memcmp(nodes[0]->getID(), dsnodes[0]->getID(), DHT_ID_LENGTH) == 0);
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.3"), dsnodes[1]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6883, dsnodes[1]->getPort());
  CPPUNIT_ASSERT(memcmp(nodes[2]->getID(), dsnodes[1]->getID(), DHT_ID_LENGTH) == 0);
}

} // namespace aria2
