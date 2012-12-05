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

class DirectDiskAdaptorTest:public CppUnit::TestFixture {

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
  SharedHandle<FileEntry> entry
    (new FileEntry(dir+"/aria2_DirectDiskAdaptorTest_testCutTrailingGarbage",
                   256, 0));
  createFile(entry->getPath(), entry->getLength()+100);

  std::vector<SharedHandle<FileEntry> > fileEntries;
  fileEntries.push_back(entry);

  DirectDiskAdaptor adaptor;
  adaptor.setDiskWriter
    (SharedHandle<DiskWriter>(new DefaultDiskWriter(entry->getPath())));
  adaptor.setTotalLength(entry->getLength());
  adaptor.setFileEntries(fileEntries.begin(), fileEntries.end());
  adaptor.openFile();

  adaptor.cutTrailingGarbage();

  CPPUNIT_ASSERT_EQUAL((int64_t)entry->getLength(),
                       File(entry->getPath()).size());
}

void DirectDiskAdaptorTest::testWriteCache()
{
  SharedHandle<DirectDiskAdaptor> adaptor(new DirectDiskAdaptor());
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
  adaptor->setDiskWriter(dw);
  WrDiskCacheEntry cache(adaptor);
  std::string data1(4096, '1'), data2(4094, '2');
  cache.cacheData(createDataCell(5, data1.c_str()));
  cache.cacheData(createDataCell(5+data1.size(), data2.c_str()));
  adaptor->writeCache(&cache);
  CPPUNIT_ASSERT_EQUAL(data1+data2, dw->getString().substr(5));

  cache.clear();
  dw->setString("");
  cache.cacheData(createDataCell(4096, data1.c_str()));
  adaptor->writeCache(&cache);
  CPPUNIT_ASSERT_EQUAL(data1, dw->getString().substr(4096));

  cache.clear();
  dw->setString("???????");
  cache.cacheData(createDataCell(0, "abc"));
  cache.cacheData(createDataCell(4, "efg"));
  adaptor->writeCache(&cache);
  CPPUNIT_ASSERT_EQUAL(std::string("abc?efg"), dw->getString());
}

} // namespace aria2
