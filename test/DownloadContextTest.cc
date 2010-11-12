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
  CPPUNIT_TEST_SUITE_END();
public:
  void testFindFileEntryByOffset();
  void testGetPieceHash();
  void testGetNumPieces();
  void testGetBasePath();
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
  ctx.setPieceHashes(&pieceHashes[0], &pieceHashes[3]);
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
  ctx.setDir("/tmp");
  // See dir doesn't effect getBasePath().
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), ctx.getBasePath());
}

} // namespace aria2
