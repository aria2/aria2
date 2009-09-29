#include "DHTRoutingTableSerializer.h"
#include "Exception.h"
#include "Util.h"
#include "DHTNode.h"
#include "array_fun.h"
#include "DHTConstants.h"
#include "bittorrent_helper.h"
#include "a2netcompat.h"
#include <cstring>
#include <sstream>
#include <iostream>
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
  SharedHandle<DHTNode> localNode(new DHTNode());

  SharedHandle<DHTNode> nodesSrc[3];
  for(size_t i = 0; i < arrayLength(nodesSrc); ++i) {
    nodesSrc[i].reset(new DHTNode());
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

  char zero[16];
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
  CPPUNIT_ASSERT((char)0x03 == buf[7]);

  // time
  ss.read(buf, 8);
  time_t time;
  uint64_t timebuf;
  memcpy(&timebuf, buf, sizeof(timebuf));
  time = ntoh64(timebuf);
  std::cerr << time << std::endl;

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
  uint32_t numNodes;
  memcpy(&numNodes, buf, sizeof(numNodes));
  numNodes = ntohl(numNodes);

  CPPUNIT_ASSERT_EQUAL((uint32_t)3, numNodes);
  // 4bytes reserved
  ss.read(buf, 4);
  CPPUNIT_ASSERT(memcmp(zero, buf, 4) == 0);

  // node[0]
  // 1byte compatc peer format length
  {
    uint8_t len;
    ss >> len;
    CPPUNIT_ASSERT_EQUAL((uint8_t)6, len);
  }
  // 7bytes reserved
  ss.read(buf, 7);
  CPPUNIT_ASSERT(memcmp(zero, buf, 7) == 0);
  // 6bytes compact peer info
  ss.read(buf, 6);
  {
    std::pair<std::string, uint16_t> peer =
      bittorrent::unpackcompact(reinterpret_cast<const unsigned char*>(buf));
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer.first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, peer.second);
  }
  // 2bytes reserved
  ss.read(buf, 2);
  CPPUNIT_ASSERT(memcmp(zero, buf, 2) == 0);
  // 16bytes reserved
  ss.read(buf, 16);
  CPPUNIT_ASSERT(memcmp(zero, buf, 16) == 0);
  // localnode ID
  ss.read(buf, DHT_ID_LENGTH);
  CPPUNIT_ASSERT(memcmp(nodes[0]->getID(), buf, DHT_ID_LENGTH) == 0);
  // 4bytes reserved
  ss.read(buf, 4);
  CPPUNIT_ASSERT(memcmp(zero, buf, 4) == 0);

  // node[1]
  // 1byte compatc peer format length
  {
    uint8_t len;
    ss >> len;
    CPPUNIT_ASSERT_EQUAL((uint8_t)6, len);
  }
  // 7bytes reserved
  ss.read(buf, 7);
  CPPUNIT_ASSERT(memcmp(zero, buf, 7) == 0);
  // 6bytes compact peer info
  ss.read(buf, 6);
  // zero filled because node[1]'s hostname is not numerical form
  // deserializer should skip this entry
  CPPUNIT_ASSERT(memcmp(zero, buf, 6) == 0);
  // 2bytes reserved
  ss.read(buf, 2);
  CPPUNIT_ASSERT(memcmp(zero, buf, 2) == 0);
  // 16bytes reserved
  ss.read(buf, 16);
  CPPUNIT_ASSERT(memcmp(zero, buf, 16) == 0);
  // localnode ID
  ss.read(buf, DHT_ID_LENGTH);
  CPPUNIT_ASSERT(memcmp(nodes[1]->getID(), buf, DHT_ID_LENGTH) == 0);
  // 4bytes reserved
  ss.read(buf, 4);
  CPPUNIT_ASSERT(memcmp(zero, buf, 4) == 0);

  // node[2]
  // 1byte compatc peer format length
  {
    uint8_t len;
    ss >> len;
    CPPUNIT_ASSERT_EQUAL((uint8_t)6, len);
  }
  // 7bytes reserved
  ss.read(buf, 7);
  CPPUNIT_ASSERT(memcmp(zero, buf, 7) == 0);
  // 6bytes compact peer info
  ss.read(buf, 6);
  {
    std::pair<std::string, uint16_t> peer =
      bittorrent::unpackcompact(reinterpret_cast<const unsigned char*>(buf));
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.3"), peer.first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6883, peer.second);
  }
  // 2bytes reserved
  ss.read(buf, 2);
  CPPUNIT_ASSERT(memcmp(zero, buf, 2) == 0);
  // 16bytes reserved
  ss.read(buf, 16);
  CPPUNIT_ASSERT(memcmp(zero, buf, 16) == 0);
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
