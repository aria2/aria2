#include "DirectDiskAdaptor.h"
#include "FileEntry.h"
#include "DefaultDiskWriter.h"
#include "Exception.h"
#include "Util.h"
#include "TestUtil.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

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
  std::string dir = "/tmp";
  SharedHandle<FileEntry> entry
    (new FileEntry(dir+"/aria2_DirectDiskAdaptorTest_testCutTrailingGarbage",
		   256, 0));
  createFile(entry->getPath(), entry->getLength()+100);

  std::deque<SharedHandle<FileEntry> > fileEntries;
  fileEntries.push_back(entry);

  DirectDiskAdaptor adaptor;
  adaptor.setDiskWriter
    (SharedHandle<DiskWriter>(new DefaultDiskWriter(entry->getPath())));
  adaptor.setTotalLength(entry->getLength());
  adaptor.setFileEntries(fileEntries);
  adaptor.openFile();

  adaptor.cutTrailingGarbage();

  CPPUNIT_ASSERT_EQUAL((uint64_t)entry->getLength(),
		       File(entry->getPath()).size());
}

} // namespace aria2
