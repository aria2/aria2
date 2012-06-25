#include "DirectDiskAdaptor.h"

#include <cppunit/extensions/HelperMacros.h>

#include "FileEntry.h"
#include "DefaultDiskWriter.h"
#include "Exception.h"
#include "util.h"
#include "TestUtil.h"

namespace aria2 {

class DirectDiskAdaptorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DirectDiskAdaptorTest);
  CPPUNIT_TEST(testCutTrailingGarbage);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testCutTrailingGarbage();
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

} // namespace aria2
