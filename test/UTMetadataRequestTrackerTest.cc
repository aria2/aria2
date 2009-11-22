#include "UTMetadataRequestTracker.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class UTMetadataRequestTrackerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UTMetadataRequestTrackerTest);
  CPPUNIT_TEST(testAdd);
  CPPUNIT_TEST(testRemove);
  CPPUNIT_TEST(testGetAllTrackedIndex);
  CPPUNIT_TEST(testCount);
  CPPUNIT_TEST(testAvail);
  CPPUNIT_TEST_SUITE_END();
public:
  void testAdd();
  void testRemove();
  void testGetAllTrackedIndex();
  void testCount();
  void testAvail();
};


CPPUNIT_TEST_SUITE_REGISTRATION(UTMetadataRequestTrackerTest);

void UTMetadataRequestTrackerTest::testAdd()
{
  UTMetadataRequestTracker tr;
  tr.add(1);
  CPPUNIT_ASSERT(tr.tracks(1));
}

void UTMetadataRequestTrackerTest::testRemove()
{
  UTMetadataRequestTracker tr;
  tr.add(1);
  tr.remove(1);
  CPPUNIT_ASSERT(!tr.tracks(1));
}

void UTMetadataRequestTrackerTest::testGetAllTrackedIndex()
{
  UTMetadataRequestTracker tr;
  tr.add(1);
  tr.add(2);

  std::vector<size_t> indexes = tr.getAllTrackedIndex();
  CPPUNIT_ASSERT_EQUAL((size_t)2, indexes.size());
  CPPUNIT_ASSERT_EQUAL((size_t)1, indexes[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)2, indexes[1]);
}

void UTMetadataRequestTrackerTest::testCount()
{
  UTMetadataRequestTracker tr;
  tr.add(1);
  tr.add(2);
  CPPUNIT_ASSERT_EQUAL((size_t)2, tr.count());
}

void UTMetadataRequestTrackerTest::testAvail()
{
  UTMetadataRequestTracker tr;
  CPPUNIT_ASSERT_EQUAL((size_t)1, tr.avail());
  tr.add(1);
  CPPUNIT_ASSERT_EQUAL((size_t)0, tr.avail());
  tr.add(2);
  CPPUNIT_ASSERT_EQUAL((size_t)0, tr.avail());
}

} // namespace aria2
