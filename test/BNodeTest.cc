#include "BNode.h"
#include "DHTNode.h"
#include "DHTBucket.h"
#include "DHTUtil.h"
#include "Exception.h"
#include "Util.h"
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class BNodeTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BNodeTest);
  CPPUNIT_TEST(testIsInRange);
  CPPUNIT_TEST(testFindBucketFor);
  CPPUNIT_TEST(testFindClosestKNodes);
  CPPUNIT_TEST(testEnumerateBucket);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testIsInRange();
  void testFindBucketFor();
  void testFindClosestKNodes();
  void testEnumerateBucket();
};


CPPUNIT_TEST_SUITE_REGISTRATION(BNodeTest);

void BNodeTest::testIsInRange()
{
  unsigned char localNodeID[DHT_ID_LENGTH];
  memset(localNodeID, 0xff, DHT_ID_LENGTH);

  SharedHandle<DHTNode> localNode(new DHTNode(localNodeID));

  SharedHandle<DHTBucket> bucket1(new DHTBucket(localNode));
  SharedHandle<DHTBucket> bucket2 = bucket1->split();
  SharedHandle<DHTBucket> bucket3 = bucket1->split();

  {
    BNode b(bucket1);
    CPPUNIT_ASSERT(b.isInRange(localNode->getID()));
  }
  {
    BNode b(bucket2);
    CPPUNIT_ASSERT(!b.isInRange(localNode->getID()));
  }
}

void BNodeTest::testFindBucketFor()
{
  unsigned char localNodeID[DHT_ID_LENGTH];
  memset(localNodeID, 0xaa, DHT_ID_LENGTH);

  SharedHandle<DHTNode> localNode(new DHTNode(localNodeID));

  SharedHandle<DHTBucket> bucket1(new DHTBucket(localNode));
  SharedHandle<DHTBucket> bucket2 = bucket1->split();
  SharedHandle<DHTBucket> bucket3 = bucket1->split();
  SharedHandle<DHTBucket> bucket4 = bucket3->split();
  SharedHandle<DHTBucket> bucket5 = bucket3->split();

  {
    BNode b(bucket5);
    CPPUNIT_ASSERT(bucket5 == BNode::findBucketFor(&b, localNodeID));
  }
  {
    BNode b(bucket1);
    CPPUNIT_ASSERT(BNode::findBucketFor(&b, localNodeID).isNull());
  }
  {
    BNode* b1 = new BNode(bucket1);
    BNode* b2 = new BNode(bucket2);
    BNode* b3 = new BNode(bucket3);
    BNode* b4 = new BNode(bucket4);
    BNode* b5 = new BNode(bucket5);

    BNode* bp1 = new BNode();
    bp1->setLeft(b3);
    bp1->setRight(b5);

    BNode* bp2 = new BNode();
    bp2->setLeft(bp1);
    bp2->setRight(b4);

    BNode* bp3 = new BNode();
    bp3->setLeft(b1);
    bp3->setRight(bp2);

    BNode* bp4 = new BNode();
    bp4->setLeft(bp3);
    bp4->setRight(b2);

    CPPUNIT_ASSERT(bucket5 == BNode::findBucketFor(bp4, localNode->getID()));

    delete bp4;
  }
}

void BNodeTest::testFindClosestKNodes()
{
  unsigned char localNodeID[DHT_ID_LENGTH];
  memset(localNodeID, 0xaa, DHT_ID_LENGTH);

  SharedHandle<DHTNode> localNode(new DHTNode(localNodeID));

  SharedHandle<DHTBucket> bucket1(new DHTBucket(localNode));
  SharedHandle<DHTBucket> bucket2 = bucket1->split();
  SharedHandle<DHTBucket> bucket3 = bucket1->split();
  SharedHandle<DHTBucket> bucket4 = bucket3->split();
  SharedHandle<DHTBucket> bucket5 = bucket3->split();

  unsigned char id[DHT_ID_LENGTH];
  {
    BNode* b1 = new BNode(bucket1);
    BNode* b2 = new BNode(bucket2);
    BNode* b3 = new BNode(bucket3);
    BNode* b4 = new BNode(bucket4);
    BNode* b5 = new BNode(bucket5);

    BNode* bp1 = new BNode();
    bp1->setLeft(b3);
    bp1->setRight(b5);

    BNode* bp2 = new BNode();
    bp2->setLeft(bp1);
    bp2->setRight(b4);

    BNode* bp3 = new BNode();
    bp3->setLeft(b1);
    bp3->setRight(bp2);

    BNode* bp4 = new BNode();
    bp4->setLeft(bp3);
    bp4->setRight(b2);


    for(size_t i = 0; i < 2; ++i) {
      bucket1->getRandomNodeID(id);
      bucket1->addNode(SharedHandle<DHTNode>(new DHTNode(id)));
      bucket2->getRandomNodeID(id);
      bucket2->addNode(SharedHandle<DHTNode>(new DHTNode(id)));
      bucket3->getRandomNodeID(id);
      bucket3->addNode(SharedHandle<DHTNode>(new DHTNode(id)));
      bucket4->getRandomNodeID(id);
      bucket4->addNode(SharedHandle<DHTNode>(new DHTNode(id)));
      bucket5->getRandomNodeID(id);
      bucket5->addNode(SharedHandle<DHTNode>(new DHTNode(id)));
    }
    {
      unsigned char targetID[DHT_ID_LENGTH];
      memset(targetID, 0x80, DHT_ID_LENGTH);
      std::deque<SharedHandle<DHTNode> > nodes = BNode::findClosestKNodes(bp4, targetID);
      CPPUNIT_ASSERT_EQUAL((size_t)8, nodes.size());
      CPPUNIT_ASSERT(bucket4->isInRange(nodes[0]));
      CPPUNIT_ASSERT(bucket4->isInRange(nodes[1]));
      CPPUNIT_ASSERT(bucket5->isInRange(nodes[2]));
      CPPUNIT_ASSERT(bucket5->isInRange(nodes[3]));
      CPPUNIT_ASSERT(bucket3->isInRange(nodes[4]));
      CPPUNIT_ASSERT(bucket3->isInRange(nodes[5]));
      CPPUNIT_ASSERT(bucket1->isInRange(nodes[6]));
      CPPUNIT_ASSERT(bucket1->isInRange(nodes[7]));
    }
    {
      unsigned char targetID[DHT_ID_LENGTH];
      memset(targetID, 0xf0, DHT_ID_LENGTH);
      std::deque<SharedHandle<DHTNode> > nodes = BNode::findClosestKNodes(bp4, targetID);
      CPPUNIT_ASSERT_EQUAL((size_t)8, nodes.size());
      CPPUNIT_ASSERT(bucket1->isInRange(nodes[0]));
      CPPUNIT_ASSERT(bucket1->isInRange(nodes[1]));
      CPPUNIT_ASSERT(bucket3->isInRange(nodes[2]));
      CPPUNIT_ASSERT(bucket3->isInRange(nodes[3]));
      CPPUNIT_ASSERT(bucket5->isInRange(nodes[4]));
      CPPUNIT_ASSERT(bucket5->isInRange(nodes[5]));
      CPPUNIT_ASSERT(bucket4->isInRange(nodes[6]));
      CPPUNIT_ASSERT(bucket4->isInRange(nodes[7]));
    }
    {
      for(size_t i = 0; i < 6; ++i) {
	bucket4->getRandomNodeID(id);
	bucket4->addNode(SharedHandle<DHTNode>(new DHTNode(id)));
      }
      unsigned char targetID[DHT_ID_LENGTH];
      memset(targetID, 0x80, DHT_ID_LENGTH);
      std::deque<SharedHandle<DHTNode> > nodes = BNode::findClosestKNodes(bp4, targetID);
      CPPUNIT_ASSERT_EQUAL((size_t)8, nodes.size());
      for(size_t i = 0; i < DHTBucket::K; ++i) {
	CPPUNIT_ASSERT(bucket4->isInRange(nodes[i]));
      }
    }
    delete bp4;
  }
}


void BNodeTest::testEnumerateBucket()
{
  unsigned char localNodeID[DHT_ID_LENGTH];
  memset(localNodeID, 0xaa, DHT_ID_LENGTH);

  SharedHandle<DHTNode> localNode(new DHTNode(localNodeID));

  SharedHandle<DHTBucket> bucket1(new DHTBucket(localNode));
  SharedHandle<DHTBucket> bucket2 = bucket1->split();
  SharedHandle<DHTBucket> bucket3 = bucket1->split();
  SharedHandle<DHTBucket> bucket4 = bucket3->split();
  SharedHandle<DHTBucket> bucket5 = bucket3->split();

  {
    BNode b(bucket1);
    std::deque<SharedHandle<DHTBucket> > buckets = BNode::enumerateBucket(&b);
    CPPUNIT_ASSERT_EQUAL((size_t)1, buckets.size());
    CPPUNIT_ASSERT(bucket1 == buckets[0]);
  }
  {
    BNode* b1 = new BNode(bucket1);
    BNode* b2 = new BNode(bucket2);
    BNode* b3 = new BNode(bucket3);
    BNode* b4 = new BNode(bucket4);
    BNode* b5 = new BNode(bucket5);

    BNode* bp1 = new BNode();
    bp1->setLeft(b3);
    bp1->setRight(b5);

    BNode* bp2 = new BNode();
    bp2->setLeft(bp1);
    bp2->setRight(b4);

    BNode* bp3 = new BNode();
    bp3->setLeft(b1);
    bp3->setRight(bp2);

    BNode* bp4 = new BNode();
    bp4->setLeft(bp3);
    bp4->setRight(b2);

    std::deque<SharedHandle<DHTBucket> > buckets = BNode::enumerateBucket(bp4);
    CPPUNIT_ASSERT_EQUAL((size_t)5, buckets.size());
    CPPUNIT_ASSERT(bucket1 == buckets[0]);
    CPPUNIT_ASSERT(bucket3 == buckets[1]);
    CPPUNIT_ASSERT(bucket5 == buckets[2]);
    CPPUNIT_ASSERT(bucket4 == buckets[3]);
    CPPUNIT_ASSERT(bucket2 == buckets[4]);

    delete bp4;
  }
}

} // namespace aria2
