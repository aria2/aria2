#include "SingleFileDownloadContext.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class SingleFileDownloadContextTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SingleFileDownloadContextTest);
  CPPUNIT_TEST(testGetPieceHash);
  CPPUNIT_TEST(testGetNumPieces);
  CPPUNIT_TEST(testGetActualBasePath);
  CPPUNIT_TEST_SUITE_END();
public:
  SingleFileDownloadContextTest() {}

  void setUp() {}

  void testGetPieceHash();
  void testGetNumPieces();
  void testGetActualBasePath();
};


CPPUNIT_TEST_SUITE_REGISTRATION( SingleFileDownloadContextTest );

void SingleFileDownloadContextTest::testGetPieceHash()
{
  SingleFileDownloadContext ctx(0, 0, "");
  std::deque<std::string> pieceHashes;
  pieceHashes.push_back("0000");
  pieceHashes.push_back("0001");
  pieceHashes.push_back("0002");
  ctx.setPieceHashes(pieceHashes);
  CPPUNIT_ASSERT_EQUAL(std::string("0000"), ctx.getPieceHash(0));
  CPPUNIT_ASSERT_EQUAL(std::string(""), ctx.getPieceHash(3));
}

void SingleFileDownloadContextTest::testGetNumPieces()
{
  SingleFileDownloadContext ctx(345, 9889, "");
  CPPUNIT_ASSERT_EQUAL((size_t)29, ctx.getNumPieces());
}

void SingleFileDownloadContextTest::testGetActualBasePath()
{
  SingleFileDownloadContext ctx(0, 0, "");
  CPPUNIT_ASSERT_EQUAL(std::string("./index.html"), ctx.getActualBasePath());
  ctx.setFilename("aria2.tar.bz2");
  CPPUNIT_ASSERT_EQUAL(std::string("./aria2.tar.bz2"), ctx.getActualBasePath());
  ctx.setUFilename("aria.tar.bz2");
  CPPUNIT_ASSERT_EQUAL(std::string("./aria.tar.bz2"), ctx.getActualBasePath());
  ctx.setDir("/tmp");
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria.tar.bz2"), ctx.getActualBasePath());
}

} // namespace aria2
