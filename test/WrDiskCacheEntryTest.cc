#include "WrDiskCacheEntry.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
#include "DirectDiskAdaptor.h"
#include "ByteArrayDiskWriter.h"

namespace aria2 {

class WrDiskCacheEntryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(WrDiskCacheEntryTest);
  CPPUNIT_TEST(testWriteToDisk);
  CPPUNIT_TEST(testClear);
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

  void testWriteToDisk();
  void testClear();
};

CPPUNIT_TEST_SUITE_REGISTRATION( WrDiskCacheEntryTest );

void WrDiskCacheEntryTest::testWriteToDisk()
{
  WrDiskCacheEntry e(adaptor_);
  e.cacheData(createDataCell(0, "??01234567", 2));
  e.cacheData(createDataCell(8, "890"));
  e.writeToDisk();
  CPPUNIT_ASSERT_EQUAL((size_t)0, e.getSize());
  CPPUNIT_ASSERT_EQUAL(std::string("01234567890"), writer_->getString());
}

void WrDiskCacheEntryTest::testClear()
{
  WrDiskCacheEntry e(adaptor_);
  e.cacheData(createDataCell(0, "foo"));
  e.clear();
  CPPUNIT_ASSERT_EQUAL((size_t)0, e.getSize());
  CPPUNIT_ASSERT_EQUAL(std::string(), writer_->getString());
}

} // namespace aria2
