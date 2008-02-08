#include "DHTBucket.h"
#include "DHTNode.h"
#include "Exception.h"
#include "Util.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHTBucketTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTBucketTest);
  CPPUNIT_TEST(testGetRandomNodeID);
  CPPUNIT_TEST(testIsInRange);
  CPPUNIT_TEST(testSplitAllowed);
  CPPUNIT_TEST(testSplit);
  CPPUNIT_TEST(testAddNode);
  CPPUNIT_TEST(testMoveToHead);
  CPPUNIT_TEST(testMoveToTail);
  CPPUNIT_TEST(testGetGoodNodes);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGetRandomNodeID();
  void testIsInRange();
  void testSplitAllowed();
  void testSplit();
  void testAddNode();
  void testMoveToHead();
  void testMoveToTail();
  void testGetGoodNodes();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTBucketTest);

void DHTBucketTest::testGetRandomNodeID()
{
  unsigned char localNodeID[] = { 0x00, 0x00, 0x00, 0x00, 0x00,
				  0x00, 0x00, 0x00, 0x00, 0x00,
				  0x00, 0x00, 0x00, 0x00, 0x00,
				  0x00, 0x00, 0x00, 0x00, 0x00 };
  SharedHandle<DHTNode> localNode = new DHTNode(localNodeID);
  {
    DHTBucket bucket(localNode);
    unsigned char nodeID[DHT_ID_LENGTH];
    bucket.getRandomNodeID(nodeID);
  }
  {
    unsigned char max[] = { 0x01, 0x01, 0xff, 0xff, 0xff,
			    0xff, 0xff, 0xff, 0xff, 0xff,
			    0xff, 0xff, 0xff, 0xff, 0xff,
			    0xff, 0xff, 0xff, 0xff, 0xff };
    unsigned char min[] = { 0x01, 0x01, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00 };
    DHTBucket bucket(16, max, min, localNode);
    unsigned char nodeID[DHT_ID_LENGTH];
    bucket.getRandomNodeID(nodeID);
    CPPUNIT_ASSERT_EQUAL(std::string("0101"),
			 Util::toHex(nodeID, sizeof(nodeID)).substr(0, 4));
  }
}

void DHTBucketTest::testIsInRange()
{
  unsigned char localNodeID[] = { 0x00, 0x00, 0x00, 0x00, 0x00,
				  0x00, 0x00, 0x00, 0x00, 0x00,
				  0x00, 0x00, 0x00, 0x00, 0x00,
				  0x00, 0x00, 0x00, 0x00, 0x00 };
  SharedHandle<DHTNode> localNode = new DHTNode(localNodeID);
  {
    unsigned char nodeID[] = { 0x00, 0x00, 0x00, 0x00, 0x00,
			       0x00, 0x00, 0x00, 0x00, 0x00,
			       0x00, 0x00, 0x00, 0x00, 0x00,
			       0x00, 0x00, 0x00, 0x00, 0x00 };
    SharedHandle<DHTNode> node = new DHTNode(nodeID);
    DHTBucket bucket(localNode);
    CPPUNIT_ASSERT(bucket.isInRange(node));
    memset(nodeID, 0xff, sizeof(nodeID));
    CPPUNIT_ASSERT(bucket.isInRange(node));
  }
  {
    unsigned char max[] = { 0x01, 0x01, 0xff, 0xff, 0xff,
			    0xff, 0xff, 0xff, 0xff, 0xff,
			    0xff, 0xff, 0xff, 0xff, 0xff,
			    0xff, 0xff, 0xff, 0xff, 0xff };
    unsigned char min[] = { 0x01, 0x01, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00 };
    {
      //min
      unsigned char nodeID[] = { 0x01, 0x01, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00 };
      SharedHandle<DHTNode> node = new DHTNode(nodeID);
      DHTBucket bucket(16, max, min, localNode);
      CPPUNIT_ASSERT(bucket.isInRange(node));
    }
    {
      //max
      unsigned char nodeID[] = { 0x01, 0x01, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff };
      SharedHandle<DHTNode> node = new DHTNode(nodeID);
      DHTBucket bucket(16, max, min, localNode);
      CPPUNIT_ASSERT(bucket.isInRange(node));
    }
    {
      unsigned char nodeID[] = { 0x01, 0x01, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00,
				 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff };
      SharedHandle<DHTNode> node = new DHTNode(nodeID);
      DHTBucket bucket(16, max, min, localNode);
      CPPUNIT_ASSERT(bucket.isInRange(node));
    }
    {
      // nodeID is out of range: smaller than this range
      unsigned char nodeID[] = { 0x00, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00 };
      SharedHandle<DHTNode> node = new DHTNode(nodeID);
      DHTBucket bucket(16, max, min, localNode);
      CPPUNIT_ASSERT(!bucket.isInRange(node));
    }
    {
      // nodeID is out of range: larger than this range
      unsigned char nodeID[] = { 0x10, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00 };
      SharedHandle<DHTNode> node = new DHTNode(nodeID);
      DHTBucket bucket(16, max, min, localNode);
      CPPUNIT_ASSERT(!bucket.isInRange(node));
    }
  }
}

void DHTBucketTest::testSplitAllowed()
{
  {
    unsigned char localNodeID[] = { 0x00, 0x00, 0x00, 0x00, 0x00,
				    0x00, 0x00, 0x00, 0x00, 0x00,
				    0x00, 0x00, 0x00, 0x00, 0x00,
				    0x00, 0x00, 0x00, 0x00, 0x00 };
    SharedHandle<DHTNode> localNode = new DHTNode(localNodeID);
    DHTBucket bucket(localNode);
    CPPUNIT_ASSERT(bucket.splitAllowed());
  }
  {
    unsigned char max[] = { 0xff, 0xff, 0xff, 0xff, 0xff,
			    0xff, 0xff, 0xff, 0xff, 0xff,
			    0xff, 0xff, 0xff, 0xff, 0xff,
			    0xff, 0xff, 0xff, 0xff, 0xff };
    unsigned char min[] = { 0xe0, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00 };
    {
      unsigned char localNodeID[] = { 0x00, 0x00, 0x00, 0x00, 0x00,
				      0x00, 0x00, 0x00, 0x00, 0x00,
				      0x00, 0x00, 0x00, 0x00, 0x00,
				      0x00, 0x00, 0x00, 0x00, 0x00 };
      SharedHandle<DHTNode> localNode = new DHTNode(localNodeID);
      DHTBucket bucket(3, max, min, localNode);
      CPPUNIT_ASSERT(!bucket.splitAllowed());
    }
    {
      unsigned char localNodeID[] = { 0xe0, 0x00, 0x00, 0x00, 0x00,
				      0x00, 0x00, 0x00, 0x00, 0x00,
				      0x00, 0x00, 0x00, 0x00, 0x00,
				      0x00, 0x00, 0x00, 0x00, 0x01 };
      SharedHandle<DHTNode> localNode = new DHTNode(localNodeID);
      DHTBucket bucket(3, max, min, localNode);
      CPPUNIT_ASSERT(bucket.splitAllowed());
    }
  }
}

void DHTBucketTest::testSplit()
{
  unsigned char localNodeID[DHT_ID_LENGTH];
  memset(localNodeID, 0, DHT_ID_LENGTH);
  SharedHandle<DHTNode> localNode = new DHTNode(localNodeID);
  {
    DHTBucket bucket(localNode);
    SharedHandle<DHTBucket> r = bucket.split();
    {
      unsigned char expectedRMax[] = { 0x7f, 0xff, 0xff, 0xff, 0xff,
				       0xff, 0xff, 0xff, 0xff, 0xff,
				       0xff, 0xff, 0xff, 0xff, 0xff,
				       0xff, 0xff, 0xff, 0xff, 0xff };
      unsigned char expectedRMin[DHT_ID_LENGTH];
      memset(expectedRMin, 0, DHT_ID_LENGTH);
      CPPUNIT_ASSERT_EQUAL(Util::toHex(expectedRMax, DHT_ID_LENGTH),
			   Util::toHex(r->getMaxID(), DHT_ID_LENGTH));
      CPPUNIT_ASSERT_EQUAL(Util::toHex(expectedRMin, DHT_ID_LENGTH),
			   Util::toHex(r->getMinID(), DHT_ID_LENGTH));
      CPPUNIT_ASSERT_EQUAL((size_t)1, r->getPrefixLength());
    }
    {
      unsigned char expectedLMax[DHT_ID_LENGTH];
      memset(expectedLMax, 0xff, DHT_ID_LENGTH);
      unsigned char expectedLMin[] = { 0x80, 0x00, 0x00, 0x00, 0x00,
				       0x00, 0x00, 0x00, 0x00, 0x00,
				       0x00, 0x00, 0x00, 0x00, 0x00,
				       0x00, 0x00, 0x00, 0x00, 0x00 };
      CPPUNIT_ASSERT_EQUAL(Util::toHex(expectedLMax, DHT_ID_LENGTH),
			   Util::toHex(bucket.getMaxID(), DHT_ID_LENGTH));
      CPPUNIT_ASSERT_EQUAL(Util::toHex(expectedLMin, DHT_ID_LENGTH),
			   Util::toHex(bucket.getMinID(), DHT_ID_LENGTH));
      CPPUNIT_ASSERT_EQUAL((size_t)1, bucket.getPrefixLength());
    }
  }
  {
    SharedHandle<DHTBucket> bucket = new DHTBucket(localNode);
    for(int i = 0; i < 159; ++i) {
      CPPUNIT_ASSERT(bucket->splitAllowed());
      SharedHandle<DHTBucket> t = bucket;
      bucket = bucket->split();
      CPPUNIT_ASSERT(!t->splitAllowed());
    }
    CPPUNIT_ASSERT(!bucket->splitAllowed());
    unsigned char expectedMax[] = { 0x00, 0x00, 0x00, 0x00, 0x00,
				    0x00, 0x00, 0x00, 0x00, 0x00,
				    0x00, 0x00, 0x00, 0x00, 0x00,
				    0x00, 0x00, 0x00, 0x00, 0x01 };
    unsigned char expectedMin[DHT_ID_LENGTH];
    memset(expectedMin, 0, DHT_ID_LENGTH);
    CPPUNIT_ASSERT_EQUAL(Util::toHex(expectedMax, DHT_ID_LENGTH),
			 Util::toHex(bucket->getMaxID(), DHT_ID_LENGTH));
    CPPUNIT_ASSERT_EQUAL(Util::toHex(expectedMin, DHT_ID_LENGTH),
			 Util::toHex(bucket->getMinID(), DHT_ID_LENGTH));
    CPPUNIT_ASSERT_EQUAL((size_t)159, bucket->getPrefixLength());
  }
}

static void createID(unsigned char* id, unsigned char firstChar, unsigned char lastChar)
{
  memset(id, 0, DHT_ID_LENGTH);
  id[0] = firstChar;
  id[DHT_ID_LENGTH-1] = lastChar;
}

void DHTBucketTest::testAddNode()
{
  unsigned char localNodeID[DHT_ID_LENGTH];
  memset(localNodeID, 0, DHT_ID_LENGTH);
  SharedHandle<DHTNode> localNode = new DHTNode(localNodeID);
  DHTBucket bucket(localNode);

  unsigned char id[DHT_ID_LENGTH];
  SharedHandle<DHTNode> nodes[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  for(size_t i = 0; i < DHTBucket::K; ++i) {
    createID(id, 0xf0, i);
    nodes[i] = new DHTNode(id);
    CPPUNIT_ASSERT(bucket.addNode(nodes[i]));
  }
  createID(id, 0xf0, 0xff);
  SharedHandle<DHTNode> newNode = new DHTNode(id);
  CPPUNIT_ASSERT(!bucket.addNode(newNode));

  // nodes[0] is located at the tail of the bucket(least recent seen)
  nodes[0]->markBad();
  CPPUNIT_ASSERT(bucket.addNode(newNode));
  CPPUNIT_ASSERT(bucket.getNodes().back() == newNode);
}

void DHTBucketTest::testMoveToHead()
{
  unsigned char localNodeID[DHT_ID_LENGTH];
  memset(localNodeID, 0, DHT_ID_LENGTH);
  SharedHandle<DHTNode> localNode = new DHTNode(localNodeID);
  DHTBucket bucket(localNode);

  unsigned char id[DHT_ID_LENGTH];
  SharedHandle<DHTNode> nodes[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  for(size_t i = 0; i < DHTBucket::K; ++i) {
    createID(id, 0xf0, i);
    nodes[i] = new DHTNode(id);
    CPPUNIT_ASSERT(bucket.addNode(nodes[i]));
  }
  bucket.moveToHead(nodes[DHTBucket::K-1]);
  CPPUNIT_ASSERT(bucket.getNodes().front() == nodes[DHTBucket::K-1]);
}

void DHTBucketTest::testMoveToTail()
{
  unsigned char localNodeID[DHT_ID_LENGTH];
  memset(localNodeID, 0, DHT_ID_LENGTH);
  SharedHandle<DHTNode> localNode = new DHTNode(localNodeID);
  DHTBucket bucket(localNode);

  unsigned char id[DHT_ID_LENGTH];
  SharedHandle<DHTNode> nodes[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  for(size_t i = 0; i < DHTBucket::K; ++i) {
    createID(id, 0xf0, i);
    nodes[i] = new DHTNode(id);
    CPPUNIT_ASSERT(bucket.addNode(nodes[i]));
  }
  bucket.moveToTail(nodes[0]);
  CPPUNIT_ASSERT(bucket.getNodes().back() == nodes[0]);
}

void DHTBucketTest::testGetGoodNodes()
{
  unsigned char localNodeID[DHT_ID_LENGTH];
  memset(localNodeID, 0, DHT_ID_LENGTH);
  SharedHandle<DHTNode> localNode = new DHTNode(localNodeID);
  DHTBucket bucket(localNode);

  unsigned char id[DHT_ID_LENGTH];
  SharedHandle<DHTNode> nodes[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  for(size_t i = 0; i < DHTBucket::K; ++i) {
    createID(id, 0xf0, i);
    nodes[i] = new DHTNode(id);
    nodes[i]->setPort(6881+i);
    CPPUNIT_ASSERT(bucket.addNode(nodes[i]));
  }
  nodes[3]->markBad();
  nodes[5]->markBad();
  std::deque<SharedHandle<DHTNode> > goodNodes = bucket.getGoodNodes();
  CPPUNIT_ASSERT_EQUAL((size_t)6, goodNodes.size());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, goodNodes[0]->getPort());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6882, goodNodes[1]->getPort());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6883, goodNodes[2]->getPort());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6885, goodNodes[3]->getPort());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6887, goodNodes[4]->getPort());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6888, goodNodes[5]->getPort());
}

} // namespace aria2
