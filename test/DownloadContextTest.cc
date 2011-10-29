#include "DownloadContext.h"

#include <cppunit/extensions/HelperMacros.h>

#include "FileEntry.h"
#include "array_fun.h"

namespace aria2 {

class DownloadContextTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DownloadContextTest);
  CPPUNIT_TEST(testFindFileEntryByOffset);
  CPPUNIT_TEST(testGetPieceHash);
  CPPUNIT_TEST(testGetNumPieces);
  CPPUNIT_TEST(testGetBasePath);
  CPPUNIT_TEST(testSetFileFilter);
  CPPUNIT_TEST_SUITE_END();
public:
  void testFindFileEntryByOffset();
  void testGetPieceHash();
  void testGetNumPieces();
  void testGetBasePath();
  void testSetFileFilter();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DownloadContextTest);

void DownloadContextTest::testFindFileEntryByOffset()
{
  DownloadContext ctx;

  CPPUNIT_ASSERT(!ctx.findFileEntryByOffset(0));
  
  const SharedHandle<FileEntry> fileEntries[] = 
    { SharedHandle<FileEntry>(new FileEntry("file1",1000,0)),
      SharedHandle<FileEntry>(new FileEntry("file2",0,1000)),
      SharedHandle<FileEntry>(new FileEntry("file3",0,1000)),
      SharedHandle<FileEntry>(new FileEntry("file4",2000,1000)),
      SharedHandle<FileEntry>(new FileEntry("file5",3000,3000)),
      SharedHandle<FileEntry>(new FileEntry("file6",0,6000))
    };
  ctx.setFileEntries(vbegin(fileEntries), vend(fileEntries));

  CPPUNIT_ASSERT_EQUAL(std::string("file1"),
                       ctx.findFileEntryByOffset(0)->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("file4"),
                       ctx.findFileEntryByOffset(1500)->getPath());
  CPPUNIT_ASSERT_EQUAL(std::string("file5"),
                       ctx.findFileEntryByOffset(5999)->getPath());
  CPPUNIT_ASSERT(!ctx.findFileEntryByOffset(6000));
}

void DownloadContextTest::testGetPieceHash()
{
  DownloadContext ctx;
  const std::string pieceHashes[] = { "hash1","hash2","shash3" };
  ctx.setPieceHashes("sha-1", &pieceHashes[0], &pieceHashes[3]);
  CPPUNIT_ASSERT_EQUAL(std::string("hash1"), ctx.getPieceHash(0));
  CPPUNIT_ASSERT_EQUAL(std::string(""), ctx.getPieceHash(3));
}

void DownloadContextTest::testGetNumPieces()
{
  DownloadContext ctx(345, 9889, "");
  CPPUNIT_ASSERT_EQUAL((size_t)29, ctx.getNumPieces());
}

void DownloadContextTest::testGetBasePath()
{
  DownloadContext ctx(0, 0, "");
  CPPUNIT_ASSERT_EQUAL(std::string(""), ctx.getBasePath());
  ctx.getFirstFileEntry()->setPath("aria2.tar.bz2");
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), ctx.getBasePath());
}

void DownloadContextTest::testSetFileFilter()
{
  DownloadContext ctx;
  std::vector<SharedHandle<FileEntry> > files;
  for(int i = 0; i < 10; ++i) {
    files.push_back(SharedHandle<FileEntry>(new FileEntry("file", 1, i)));
  }
  ctx.setFileEntries(files.begin(), files.end());
  SegList<int> sgl;
  util::parseIntSegments(sgl, "6-8,2-4");
  sgl.normalize();
  ctx.setFileFilter(sgl);
  const std::vector<SharedHandle<FileEntry> >& res = ctx.getFileEntries();
  CPPUNIT_ASSERT(!res[0]->isRequested());
  CPPUNIT_ASSERT(res[1]->isRequested());
  CPPUNIT_ASSERT(res[2]->isRequested());
  CPPUNIT_ASSERT(res[3]->isRequested());
  CPPUNIT_ASSERT(!res[4]->isRequested());
  CPPUNIT_ASSERT(res[5]->isRequested());
  CPPUNIT_ASSERT(res[6]->isRequested());
  CPPUNIT_ASSERT(res[7]->isRequested());
  CPPUNIT_ASSERT(!res[8]->isRequested());
  CPPUNIT_ASSERT(!res[9]->isRequested());
}

} // namespace aria2
