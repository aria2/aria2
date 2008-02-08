#include "Piece.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class PieceTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PieceTest);
  CPPUNIT_TEST(testCompleteBlock);
  CPPUNIT_TEST(testIsRangeComplete);
  CPPUNIT_TEST(testIsRangeComplete_subPiece);
  CPPUNIT_TEST(testGetCompletedLength);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testCompleteBlock();
  void testIsRangeComplete();
  void testIsRangeComplete_subPiece();
  void testGetCompletedLength();
};


CPPUNIT_TEST_SUITE_REGISTRATION( PieceTest );

void PieceTest::testCompleteBlock()
{
  int32_t blockLength = 32*1024;
  Piece p(0, blockLength*10, blockLength);
  
  // 5 is a block index inside the Piece p.
  SharedHandle<Piece> subPiece = new Piece(5, blockLength, 1);
  p.addSubPiece(subPiece);
  
  CPPUNIT_ASSERT(!p.getSubPiece(5).isNull());

  // When block is complete, then its associated sub piece must be deleted.
  p.completeBlock(5);

  CPPUNIT_ASSERT(p.getSubPiece(5).isNull());
}

void PieceTest::testIsRangeComplete()
{
  int32_t blockLength = 16*1024;
  Piece p(0, blockLength*10, blockLength);
  
  CPPUNIT_ASSERT(!p.isRangeComplete(8*1024, 16*1024));

  p.completeBlock(0);
  CPPUNIT_ASSERT(!p.isRangeComplete(8*1024, 16*1024));
  CPPUNIT_ASSERT(p.isRangeComplete(8*1024, 8*1024));

  p.completeBlock(1);
  CPPUNIT_ASSERT(p.isRangeComplete(8*1024, 16*1024));
}

void PieceTest::testIsRangeComplete_subPiece()
{
  int32_t blockLength = 16*1024;
  Piece p(0, blockLength*10, blockLength);
  
  CPPUNIT_ASSERT(!p.isRangeComplete(8*1024, 32*1024));

  SharedHandle<Piece> startSubPiece = new Piece(0, blockLength, 1);
  p.addSubPiece(startSubPiece);
  
  SharedHandle<Piece> endSubPiece = new Piece(2, blockLength, 1);
  p.addSubPiece(endSubPiece);
  
  p.completeBlock(1);

  CPPUNIT_ASSERT(!p.isRangeComplete(8*1024, 32*1024));

  for(int32_t i = 8*1024; i < blockLength; ++i) {
    startSubPiece->completeBlock(i);
  }

  CPPUNIT_ASSERT(!p.isRangeComplete(8*1024, 32*1024));

  for(int32_t i = 0; i < 8*1024; ++i) {
    endSubPiece->completeBlock(i);
  }
  CPPUNIT_ASSERT(p.isRangeComplete(8*1024, 32*1024));
}

void PieceTest::testGetCompletedLength()
{
  int32_t blockLength = 16*1024;
  Piece p(0, blockLength*10, blockLength);
  
  SharedHandle<Piece> subPiece = new Piece(0, blockLength, 1);
  for(int32_t i = 0; i < blockLength-1; ++i) {
    subPiece->completeBlock(i);
  }
  p.addSubPiece(subPiece);

  p.completeBlock(1);
  p.completeBlock(2);
  p.completeBlock(9);
  
  CPPUNIT_ASSERT_EQUAL(blockLength*3+blockLength-1, p.getCompletedLength());
}

} // namespace aria2
