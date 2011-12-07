#include "Piece.h"

#include <string>

#include <cppunit/extensions/HelperMacros.h>

#include "util.h"

namespace aria2 {

class PieceTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PieceTest);
  CPPUNIT_TEST(testCompleteBlock);
  CPPUNIT_TEST(testGetCompletedLength);

#ifdef ENABLE_MESSAGE_DIGEST

  CPPUNIT_TEST(testUpdateHash);

#endif // ENABLE_MESSAGE_DIGEST

  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testCompleteBlock();
  void testGetCompletedLength();

#ifdef ENABLE_MESSAGE_DIGEST

  void testUpdateHash();

#endif // ENABLE_MESSAGE_DIGEST
};


CPPUNIT_TEST_SUITE_REGISTRATION( PieceTest );

void PieceTest::testCompleteBlock()
{
  size_t blockLength = 32*1024;
  Piece p(0, blockLength*10, blockLength);
  
  p.completeBlock(5);

  CPPUNIT_ASSERT(p.hasBlock(5));
}

void PieceTest::testGetCompletedLength()
{
  int32_t blockLength = 16*1024;
  Piece p(0, blockLength*10+100, blockLength);
  
  p.completeBlock(1);
  p.completeBlock(2);
  p.completeBlock(9);
  p.completeBlock(10); // <-- 100 bytes
  
  CPPUNIT_ASSERT_EQUAL(blockLength*3+100, p.getCompletedLength());
}

#ifdef ENABLE_MESSAGE_DIGEST

void PieceTest::testUpdateHash()
{
  Piece p(0, 16, 2*1024*1024);
  p.setHashType("sha-1");
  
  std::string spam("SPAM!");
  CPPUNIT_ASSERT(p.updateHash
                 (0, reinterpret_cast<const unsigned char*>(spam.c_str()),
                  spam.size()));
  CPPUNIT_ASSERT(!p.isHashCalculated());

  std::string spamspam("SPAM!SPAM!!");
  CPPUNIT_ASSERT(p.updateHash
                 (spam.size(),
                  reinterpret_cast<const unsigned char*>(spamspam.c_str()),
                  spamspam.size()));
  CPPUNIT_ASSERT(p.isHashCalculated());

  CPPUNIT_ASSERT_EQUAL(std::string("d9189aff79e075a2e60271b9556a710dc1bc7de7"),
                       util::toHex(p.getDigest()));
}

#endif // ENABLE_MESSAGE_DIGEST

} // namespace aria2
