#include "DHTRoutingTableSerializer.h"
#include "Exception.h"
#include "Util.h"
#include "DHTNode.h"
#include "array_fun.h"
#include "DHTConstants.h"
#include "PeerMessageUtil.h"
#include <arpa/inet.h>
#include <cstring>
#include <sstream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHTRoutingTableSerializerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTRoutingTableSerializerTest);
  CPPUNIT_TEST(testSerialize);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testSerialize();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTRoutingTableSerializerTest);

void DHTRoutingTableSerializerTest::testSerialize()
{
  SharedHandle<DHTNode> localNode = new DHTNode();

  SharedHandle<DHTNode> nodesSrc[] = { 0, 0, 0 };
  for(size_t i = 0; i < arrayLength(nodesSrc); ++i) {
    nodesSrc[i] = new DHTNode();
    nodesSrc[i]->setIPAddress("192.168.0."+Util::uitos(i+1));
    nodesSrc[i]->setPort(6881+i);
  }
  nodesSrc[1]->setIPAddress("non-numerical-name");
  std::deque<SharedHandle<DHTNode> > nodes(&nodesSrc[0], &nodesSrc[arrayLength(nodesSrc)]);
  
  DHTRoutingTableSerializer s;
  s.setLocalNode(localNode);
  s.setNodes(nodes);

  std::stringstream ss;
  s.serialize(ss);

  char zero[8];
  memset(zero, 0, sizeof(zero));

  char buf[20];
  // header
  ss.read(buf, 8);
  // magic
  CPPUNIT_ASSERT((char)0xa1 == buf[0]);
  CPPUNIT_ASSERT((char)0xa2 == buf[1]);
  // format ID
  CPPUNIT_ASSERT((char)0x02 == buf[2]);
  // reserved
  CPPUNIT_ASSERT((char)0x00 == buf[3]);
  CPPUNIT_ASSERT((char)0x00 == buf[4]);
  CPPUNIT_ASSERT((char)0x00 == buf[5]);
  // version
  CPPUNIT_ASSERT((char)0x00 == buf[6]);
  CPPUNIT_ASSERT((char)0x01 == buf[7]);

  // time
  ss.read(buf, 4);
  time_t time = ntohl(*reinterpret_cast<uint32_t*>(buf));
  std::cerr << time << std::endl;
  // 4bytes reserved
  ss.read(buf, 4);
  CPPUNIT_ASSERT(memcmp(zero, buf, 4) == 0);

  // localnode
  // 8bytes reserved
  ss.read(buf, 8);
  CPPUNIT_ASSERT(memcmp(zero, buf, 8) == 0);
  // localnode ID
  ss.read(buf, DHT_ID_LENGTH);
  CPPUNIT_ASSERT(memcmp(localNode->getID(), buf, DHT_ID_LENGTH) == 0);
  // 4bytes reserved
  ss.read(buf, 4);
  CPPUNIT_ASSERT(memcmp(zero, buf, 4) == 0);

  // number of nodes saved
  ss.read(buf, 4);
  uint32_t numNodes = ntohl(*reinterpret_cast<uint32_t*>(buf));
  CPPUNIT_ASSERT_EQUAL((uint32_t)3, numNodes);
  // 4bytes reserved
  ss.read(buf, 4);
  CPPUNIT_ASSERT(memcmp(zero, buf, 4) == 0);

  // node[0]
  // 6bytes compact peer info
  ss.read(buf, 6);
  {
    std::pair<std::string, uint16_t> peer = PeerMessageUtil::unpackcompact(buf);
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer.first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, peer.second);
  }
  // 2bytes reserved
  ss.read(buf, 2);
  CPPUNIT_ASSERT(memcmp(zero, buf, 2) == 0);
  // localnode ID
  ss.read(buf, DHT_ID_LENGTH);
  CPPUNIT_ASSERT(memcmp(nodes[0]->getID(), buf, DHT_ID_LENGTH) == 0);
  // 4bytes reserved
  ss.read(buf, 4);
  CPPUNIT_ASSERT(memcmp(zero, buf, 4) == 0);

  // node[1]
  // 6bytes compact peer info
  ss.read(buf, 6);
  // zero filled because node[1]'s hostname is not numerical form
  // deserializer should skip this entry
  CPPUNIT_ASSERT(memcmp(zero, buf, 6) == 0);
  // 2bytes reserved
  ss.read(buf, 2);
  CPPUNIT_ASSERT(memcmp(zero, buf, 2) == 0);
  // localnode ID
  ss.read(buf, DHT_ID_LENGTH);
  CPPUNIT_ASSERT(memcmp(nodes[1]->getID(), buf, DHT_ID_LENGTH) == 0);
  // 4bytes reserved
  ss.read(buf, 4);
  CPPUNIT_ASSERT(memcmp(zero, buf, 4) == 0);

  // node[2]
  // 6bytes compact peer info
  ss.read(buf, 6);
  {
    std::pair<std::string, uint16_t> peer = PeerMessageUtil::unpackcompact(buf);
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.3"), peer.first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6883, peer.second);
  }
  // 2bytes reserved
  ss.read(buf, 2);
  CPPUNIT_ASSERT(memcmp(zero, buf, 2) == 0);
  // localnode ID
  ss.read(buf, DHT_ID_LENGTH);
  CPPUNIT_ASSERT(memcmp(nodes[2]->getID(), buf, DHT_ID_LENGTH) == 0);
  // 4bytes reserved
  ss.read(buf, 4);
  CPPUNIT_ASSERT(memcmp(zero, buf, 4) == 0);

  // check to see stream ends
  ss.read(buf, 1);
  CPPUNIT_ASSERT_EQUAL((std::streamsize)0, ss.gcount());
  CPPUNIT_ASSERT(ss.eof());
}

} // namespace aria2
