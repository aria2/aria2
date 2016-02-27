#include "DirectDiskAdaptor.h"

#include <cppunit/extensions/HelperMacros.h>

#include "FileEntry.h"
#include "DefaultDiskWriter.h"
#include "Exception.h"
#include "util.h"
#include "TestUtil.h"
#include "ByteArrayDiskWriter.h"
#include "WrDiskCacheEntry.h"

namespace aria2 {

class DirectDiskAdaptorTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DirectDiskAdaptorTest);
  CPPUNIT_TEST(testCutTrailingGarbage);
  CPPUNIT_TEST(testWriteCache);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testCutTrailingGarbage();
  void testWriteCache();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DirectDiskAdaptorTest);

void DirectDiskAdaptorTest::testCutTrailingGarbage()
{
  std::string dir = A2_TEST_OUT_DIR;
  auto entry = std::make_shared<FileEntry>(
      dir + "/aria2_DirectDiskAdaptorTest_testCutTrailingGarbage", 256, 0);
  createFile(entry->getPath(), entry->getLength() + 100);
  auto fileEntries = std::vector<std::shared_ptr<FileEntry>>{entry};
  DirectDiskAdaptor adaptor;
  adaptor.setDiskWriter(make_unique<DefaultDiskWriter>(entry->getPath()));
  adaptor.setTotalLength(entry->getLength());
  adaptor.setFileEntries(fileEntries.begin(), fileEntries.end());
  adaptor.openFile();

  adaptor.cutTrailingGarbage();

  CPPUNIT_ASSERT_EQUAL((int64_t)entry->getLength(),
                       File(entry->getPath()).size());
}

void DirectDiskAdaptorTest::testWriteCache()
{
  auto adaptor = std::make_shared<DirectDiskAdaptor>();
  ByteArrayDiskWriter* dw;
  {
    auto sdw = make_unique<ByteArrayDiskWriter>();
    dw = sdw.get();
    adaptor->setDiskWriter(std::move(sdw));
  }
  WrDiskCacheEntry cache{adaptor};
  std::string data1(4_k, '1'), data2(4094, '2');
  cache.cacheData(createDataCell(5, data1.c_str()));
  cache.cacheData(createDataCell(5 + data1.size(), data2.c_str()));
  adaptor->writeCache(&cache);
  CPPUNIT_ASSERT_EQUAL(data1 + data2, dw->getString().substr(5));

  cache.clear();
  dw->setString("");
  cache.cacheData(createDataCell(4_k, data1.c_str()));
  adaptor->writeCache(&cache);
  CPPUNIT_ASSERT_EQUAL(data1, dw->getString().substr(4_k));

  cache.clear();
  dw->setString("???????");
  cache.cacheData(createDataCell(0, "abc"));
  cache.cacheData(createDataCell(4, "efg"));
  adaptor->writeCache(&cache);
  CPPUNIT_ASSERT_EQUAL(std::string("abc?efg"), dw->getString());
}

} // namespace aria2
