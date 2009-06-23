#include "DownloadContext.h"

#include <cppunit/extensions/HelperMacros.h>

#include "FileEntry.h"
#include "MockDownloadContext.h"

namespace aria2 {

class DownloadContextTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DownloadContextTest);
  CPPUNIT_TEST(testFindFileEntryByOffset);
  CPPUNIT_TEST_SUITE_END();
public:
  void testFindFileEntryByOffset();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DownloadContextTest);

void DownloadContextTest::testFindFileEntryByOffset()
{
  MockDownloadContext ctx;

  CPPUNIT_ASSERT(ctx.findFileEntryByOffset(0).isNull());
  
  ctx.addFileEntry(SharedHandle<FileEntry>(new FileEntry("file1",1000,0)));
  ctx.addFileEntry(SharedHandle<FileEntry>(new FileEntry("file2",0,1000)));
  ctx.addFileEntry(SharedHandle<FileEntry>(new FileEntry("file3",0,1000)));
  ctx.addFileEntry(SharedHandle<FileEntry>(new FileEntry("file4",2000,1000)));
  ctx.addFileEntry(SharedHandle<FileEntry>(new FileEntry("file5",3000,3000)));
  ctx.addFileEntry(SharedHandle<FileEntry>(new FileEntry("file6",0,6000)));

  CPPUNIT_ASSERT_EQUAL(std::string("file1"),
		       ctx.findFileEntryByOffset(0)->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("file4"),
		       ctx.findFileEntryByOffset(1500)->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("file5"),
		       ctx.findFileEntryByOffset(5999)->getPath());
  CPPUNIT_ASSERT(ctx.findFileEntryByOffset(6000).isNull());
}

} // namespace aria2
