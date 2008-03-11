#include "Piece.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class PieceTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PieceTest);
  CPPUNIT_TEST(testCompleteBlock);
  CPPUNIT_TEST(testGetCompletedLength);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testCompleteBlock();
  void testGetCompletedLength();
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
  size_t blockLength = 16*1024;
  Piece p(0, blockLength*10+100, blockLength);
  
  p.completeBlock(1);
  p.completeBlock(2);
  p.completeBlock(9);
  p.completeBlock(10); // <-- 100 bytes
  
  CPPUNIT_ASSERT_EQUAL(blockLength*3+100, p.getCompletedLength());
}

} // namespace aria2
