#include "DHTBucketTree.h"

#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "DHTBucket.h"
#include "Exception.h"
#include "util.h"

namespace aria2 {

class DHTBucketTreeTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTBucketTreeTest);
  CPPUNIT_TEST(testDig);
  CPPUNIT_TEST(testFindBucketFor);
  CPPUNIT_TEST(testFindClosestKNodes);
  CPPUNIT_TEST(testEnumerateBucket);
  CPPUNIT_TEST_SUITE_END();
public:
  void testDig();
  void testFindBucketFor();
  void testFindClosestKNodes();
  void testEnumerateBucket();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTBucketTreeTest);

void DHTBucketTreeTest::testDig()
{
  unsigned char localNodeID[DHT_ID_LENGTH];
  memset(localNodeID, 0xff, DHT_ID_LENGTH);

  SharedHandle<DHTNode> localNode(new DHTNode(localNodeID));

  SharedHandle<DHTBucket> bucket1(new DHTBucket(localNode));
  SharedHandle<DHTBucket> bucket2 = bucket1->split();
  SharedHandle<DHTBucket> bucket3 = bucket1->split();
  // Tree: number is prefix
  //
  //           +
  //    +------+------+
  //   b2             |
  //   0       +------+------+
  //          b3             b1
  //          10             11
  //                         |
  //                     localNode is here
  {
    DHTBucketTreeNode b(bucket1);
    CPPUNIT_ASSERT(!b.dig(localNode->getID()));
  }
  {
    DHTBucketTreeNode* left = new DHTBucketTreeNode(bucket3);
    DHTBucketTreeNode* right = new DHTBucketTreeNode(bucket1);
    DHTBucketTreeNode b(left, right);
    CPPUNIT_ASSERT(b.dig(localNode->getID()) == right);
  }
}

void DHTBucketTreeTest::testFindBucketFor()
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
    DHTBucketTreeNode b(bucket5);
    CPPUNIT_ASSERT(*bucket5 == *dht::findBucketFor(&b, localNodeID));
  }
  {
    // Tree: number is prefix
    //
    //           +
    //    +------+------+
    //   b2             |
    //   0       +------+------+
    //           |             b1
    //     +-----+-----+      11
    //    b4           |
    //   100     +-----+-----+
    //          b5           b3
    //          1010         1011
    //           |
    //    localNode is here
    DHTBucketTreeNode* b1 = new DHTBucketTreeNode(bucket1);
    DHTBucketTreeNode* b2 = new DHTBucketTreeNode(bucket2);
    DHTBucketTreeNode* b3 = new DHTBucketTreeNode(bucket3);
    DHTBucketTreeNode* b4 = new DHTBucketTreeNode(bucket4);
    DHTBucketTreeNode* b5 = new DHTBucketTreeNode(bucket5);

    DHTBucketTreeNode* bp1 = new DHTBucketTreeNode(b5, b3);
    DHTBucketTreeNode* bp2 = new DHTBucketTreeNode(b4, bp1);
    DHTBucketTreeNode* bp3 = new DHTBucketTreeNode(bp2, b1);
    DHTBucketTreeNode bp4(b2, bp3);

    CPPUNIT_ASSERT(*bucket5 == *dht::findBucketFor(&bp4, localNode->getID()));
  }
}

void DHTBucketTreeTest::testFindClosestKNodes()
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
    DHTBucketTreeNode* b1 = new DHTBucketTreeNode(bucket1);
    DHTBucketTreeNode* b2 = new DHTBucketTreeNode(bucket2);
    DHTBucketTreeNode* b3 = new DHTBucketTreeNode(bucket3);
    DHTBucketTreeNode* b4 = new DHTBucketTreeNode(bucket4);
    DHTBucketTreeNode* b5 = new DHTBucketTreeNode(bucket5);

    DHTBucketTreeNode* bp1 = new DHTBucketTreeNode(b5, b3);
    DHTBucketTreeNode* bp2 = new DHTBucketTreeNode(b4, bp1);
    DHTBucketTreeNode* bp3 = new DHTBucketTreeNode(bp2, b1);
    DHTBucketTreeNode bp4(b2, bp3);

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
      std::vector<SharedHandle<DHTNode> > nodes;
      dht::findClosestKNodes(nodes, &bp4, targetID);
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
      std::vector<SharedHandle<DHTNode> > nodes;
      dht::findClosestKNodes(nodes, &bp4, targetID);
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
      std::vector<SharedHandle<DHTNode> > nodes;
      dht::findClosestKNodes(nodes, &bp4, targetID);
      CPPUNIT_ASSERT_EQUAL((size_t)8, nodes.size());
      for(size_t i = 0; i < DHTBucket::K; ++i) {
        CPPUNIT_ASSERT(bucket4->isInRange(nodes[i]));
      }
    }
  }
}


void DHTBucketTreeTest::testEnumerateBucket()
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
    DHTBucketTreeNode b(bucket1);
    std::vector<SharedHandle<DHTBucket> > buckets;
    dht::enumerateBucket(buckets, &b);
    CPPUNIT_ASSERT_EQUAL((size_t)1, buckets.size());
    CPPUNIT_ASSERT(*bucket1 == *buckets[0]);
  }
  {
    DHTBucketTreeNode* b1 = new DHTBucketTreeNode(bucket1);
    DHTBucketTreeNode* b2 = new DHTBucketTreeNode(bucket2);
    DHTBucketTreeNode* b3 = new DHTBucketTreeNode(bucket3);
    DHTBucketTreeNode* b4 = new DHTBucketTreeNode(bucket4);
    DHTBucketTreeNode* b5 = new DHTBucketTreeNode(bucket5);

    DHTBucketTreeNode* bp1 = new DHTBucketTreeNode(b5, b3);
    DHTBucketTreeNode* bp2 = new DHTBucketTreeNode(b4, bp1);
    DHTBucketTreeNode* bp3 = new DHTBucketTreeNode(bp2, b1);
    DHTBucketTreeNode bp4(b2, bp3);

    std::vector<SharedHandle<DHTBucket> > buckets;
    dht::enumerateBucket(buckets, &bp4);
    CPPUNIT_ASSERT_EQUAL((size_t)5, buckets.size());
    CPPUNIT_ASSERT(*bucket2 == *buckets[0]);
    CPPUNIT_ASSERT(*bucket4 == *buckets[1]);
    CPPUNIT_ASSERT(*bucket5 == *buckets[2]);
    CPPUNIT_ASSERT(*bucket3 == *buckets[3]);
    CPPUNIT_ASSERT(*bucket1 == *buckets[4]);
  }
}

} // namespace aria2
