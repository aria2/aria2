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

class DHTRoutingTableDeserializerTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTRoutingTableDeserializerTest);
  CPPUNIT_TEST(testDeserialize);
  CPPUNIT_TEST(testDeserialize6);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testDeserialize();

  void testDeserialize6();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DHTRoutingTableDeserializerTest);

void DHTRoutingTableDeserializerTest::testDeserialize()
{
  std::shared_ptr<DHTNode> localNode(new DHTNode());

  std::vector<std::shared_ptr<DHTNode>> nodes(3);
  for (size_t i = 0; i < nodes.size(); ++i) {
    nodes[i].reset(new DHTNode());
    nodes[i]->setIPAddress("192.168.0." + util::uitos(i + 1));
    nodes[i]->setPort(6881 + i);
  }
  nodes[1]->setIPAddress("non-numerical-name");

  DHTRoutingTableSerializer s(AF_INET);
  s.setLocalNode(localNode);
  s.setNodes(nodes);

  std::string filename =
      A2_TEST_OUT_DIR "/aria2_DHTRoutingTableDeserializerTest_testDeserialize";
  s.serialize(filename);

  DHTRoutingTableDeserializer d(AF_INET);
  d.deserialize(filename);

  CPPUNIT_ASSERT(memcmp(localNode->getID(), d.getLocalNode()->getID(),
                        DHT_ID_LENGTH) == 0);

  CPPUNIT_ASSERT_EQUAL((size_t)2, d.getNodes().size());
  const std::vector<std::shared_ptr<DHTNode>>& dsnodes = d.getNodes();
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), dsnodes[0]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, dsnodes[0]->getPort());
  CPPUNIT_ASSERT(
      memcmp(nodes[0]->getID(), dsnodes[0]->getID(), DHT_ID_LENGTH) == 0);
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.3"), dsnodes[1]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6883, dsnodes[1]->getPort());
  CPPUNIT_ASSERT(
      memcmp(nodes[2]->getID(), dsnodes[1]->getID(), DHT_ID_LENGTH) == 0);
}

void DHTRoutingTableDeserializerTest::testDeserialize6()
{
  std::shared_ptr<DHTNode> localNode(new DHTNode());

  std::vector<std::shared_ptr<DHTNode>> nodes(3);
  for (size_t i = 0; i < nodes.size(); ++i) {
    nodes[i].reset(new DHTNode());
    nodes[i]->setIPAddress("2001::100" + util::uitos(i + 1));
    nodes[i]->setPort(6881 + i);
  }
  nodes[1]->setIPAddress("non-numerical-name");

  DHTRoutingTableSerializer s(AF_INET6);
  s.setLocalNode(localNode);
  s.setNodes(nodes);

  std::string filename =
      A2_TEST_OUT_DIR "/aria2_DHTRoutingTableDeserializerTest_testDeserialize6";
  s.serialize(filename);

  DHTRoutingTableDeserializer d(AF_INET6);
  d.deserialize(filename);

  CPPUNIT_ASSERT(memcmp(localNode->getID(), d.getLocalNode()->getID(),
                        DHT_ID_LENGTH) == 0);

  CPPUNIT_ASSERT_EQUAL((size_t)2, d.getNodes().size());
  const std::vector<std::shared_ptr<DHTNode>>& dsnodes = d.getNodes();
  CPPUNIT_ASSERT_EQUAL(std::string("2001::1001"), dsnodes[0]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, dsnodes[0]->getPort());
  CPPUNIT_ASSERT(
      memcmp(nodes[0]->getID(), dsnodes[0]->getID(), DHT_ID_LENGTH) == 0);
  CPPUNIT_ASSERT_EQUAL(std::string("2001::1003"), dsnodes[1]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6883, dsnodes[1]->getPort());
  CPPUNIT_ASSERT(
      memcmp(nodes[2]->getID(), dsnodes[1]->getID(), DHT_ID_LENGTH) == 0);
}

} // namespace aria2
