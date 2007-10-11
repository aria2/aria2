#include "PiecedSegment.h"
#include "Piece.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class SegmentTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SegmentTest);
  CPPUNIT_TEST(testUpdateWrittenLength);
  CPPUNIT_TEST(testUpdateWrittenLength_overflow);
  CPPUNIT_TEST(testUpdateWrittenLength_lastPiece);
  CPPUNIT_TEST(testUpdateWrittenLength_incompleteLastPiece);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testUpdateWrittenLength();
  void testUpdateWrittenLength_overflow();
  void testUpdateWrittenLength_lastPiece();
  void testUpdateWrittenLength_incompleteLastPiece();
};


CPPUNIT_TEST_SUITE_REGISTRATION( SegmentTest );

void SegmentTest::testUpdateWrittenLength()
{
  PieceHandle p = new Piece(0, 16*1024*10);
  PiecedSegment s(16*1024*10, p);
  CPPUNIT_ASSERT_EQUAL((int32_t)0, s.getWrittenLength());

  s.updateWrittenLength(16*1024);
  CPPUNIT_ASSERT(p->hasBlock(0));
  CPPUNIT_ASSERT(!p->hasBlock(1));

  s.updateWrittenLength(16*1024*9);
  CPPUNIT_ASSERT(p->pieceComplete());
}

void SegmentTest::testUpdateWrittenLength_overflow()
{
  PieceHandle p = new Piece(0, 16*1024*10);
  PiecedSegment s(16*1024*10, p);

  s.updateWrittenLength(16*1024*11);
  CPPUNIT_ASSERT(p->pieceComplete());
  CPPUNIT_ASSERT_EQUAL((int32_t)16*1024, s.getOverflowLength());
}

void SegmentTest::testUpdateWrittenLength_lastPiece()
{
  PieceHandle p = new Piece(0, 16*1024*9+1);
  PiecedSegment s(16*1024*10, p);

  s.updateWrittenLength(16*1024*10);
  CPPUNIT_ASSERT(p->pieceComplete());
}

void SegmentTest::testUpdateWrittenLength_incompleteLastPiece()
{
  PieceHandle p = new Piece(0, 16*1024*9+2);
  PiecedSegment s(16*1024*10, p);

  s.updateWrittenLength(16*1024*9+1);
  CPPUNIT_ASSERT(!p->pieceComplete());
  s.updateWrittenLength(1);
  CPPUNIT_ASSERT(p->pieceComplete());
}
