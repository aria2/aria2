#include "WrDiskCache.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
#include "DirectDiskAdaptor.h"
#include "ByteArrayDiskWriter.h"

namespace aria2 {

class WrDiskCacheTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(WrDiskCacheTest);
  CPPUNIT_TEST(testAdd);
  CPPUNIT_TEST_SUITE_END();

  SharedHandle<DirectDiskAdaptor> adaptor_;
  SharedHandle<ByteArrayDiskWriter> writer_;
public:
  void setUp()
  {
    adaptor_.reset(new DirectDiskAdaptor());
    writer_.reset(new ByteArrayDiskWriter());
    adaptor_->setDiskWriter(writer_);
  }

  void testAdd();
};

CPPUNIT_TEST_SUITE_REGISTRATION( WrDiskCacheTest );

void WrDiskCacheTest::testAdd()
{
  WrDiskCache dc(20);
  CPPUNIT_ASSERT_EQUAL((size_t)0, dc.getSize());
  WrDiskCacheEntry e1(adaptor_);
  e1.cacheData(createDataCell(0, "who knows?"));
  CPPUNIT_ASSERT(dc.add(&e1));
  CPPUNIT_ASSERT_EQUAL((size_t)10, dc.getSize());

  WrDiskCacheEntry e2(adaptor_);
  e2.cacheData(createDataCell(21, "seconddata"));
  CPPUNIT_ASSERT(dc.add(&e2));
  CPPUNIT_ASSERT_EQUAL((size_t)20, dc.getSize());

  WrDiskCacheEntry e3(adaptor_);
  e3.cacheData(createDataCell(10, "hello"));
  CPPUNIT_ASSERT(dc.add(&e3));
  CPPUNIT_ASSERT_EQUAL((size_t)15, dc.getSize());
  // e1 is flushed to the disk
  CPPUNIT_ASSERT_EQUAL(std::string("who knows?"), writer_->getString());
  CPPUNIT_ASSERT_EQUAL((size_t)0, e1.getSize());

  e3.cacheData(createDataCell(15, " world"));
  CPPUNIT_ASSERT(dc.update(&e3, 6));

  // e3 is flushed to the disk
  CPPUNIT_ASSERT_EQUAL(std::string("who knows?hello world"),
                       writer_->getString());
  CPPUNIT_ASSERT_EQUAL((size_t)0, e3.getSize());
  CPPUNIT_ASSERT_EQUAL((size_t)10, dc.getSize());

  e2.cacheData(createDataCell(31, "01234567890"));
  CPPUNIT_ASSERT(dc.update(&e2, 11));
  // e2 is flushed to the disk
  CPPUNIT_ASSERT_EQUAL(std::string("who knows?hello worldseconddata01234567890"),
                       writer_->getString());
  CPPUNIT_ASSERT_EQUAL((size_t)0, e2.getSize());
  CPPUNIT_ASSERT_EQUAL((size_t)0, dc.getSize());
}

} // namespace aria2
