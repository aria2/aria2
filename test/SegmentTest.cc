#include "PiecedSegment.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Piece.h"

namespace aria2 {

class SegmentTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SegmentTest);
  CPPUNIT_TEST(testUpdateWrittenLength);
  CPPUNIT_TEST(testUpdateWrittenLength_lastPiece);
  CPPUNIT_TEST(testUpdateWrittenLength_incompleteLastPiece);
  CPPUNIT_TEST(testClear);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testUpdateWrittenLength();
  void testUpdateWrittenLength_lastPiece();
  void testUpdateWrittenLength_incompleteLastPiece();
  void testClear();
};


CPPUNIT_TEST_SUITE_REGISTRATION( SegmentTest );

void SegmentTest::testUpdateWrittenLength()
{
  SharedHandle<Piece> p(new Piece(0, 16*1024*10));
  PiecedSegment s(16*1024*10, p);
  CPPUNIT_ASSERT_EQUAL(0, s.getWrittenLength());

  s.updateWrittenLength(16*1024);
  CPPUNIT_ASSERT(p->hasBlock(0));
  CPPUNIT_ASSERT(!p->hasBlock(1));

  s.updateWrittenLength(16*1024*9);
  CPPUNIT_ASSERT(p->pieceComplete());
}

void SegmentTest::testUpdateWrittenLength_lastPiece()
{
  SharedHandle<Piece> p(new Piece(0, 16*1024*9+1));
  PiecedSegment s(16*1024*10, p);

  s.updateWrittenLength(p->getLength());
  CPPUNIT_ASSERT(p->pieceComplete());
}

void SegmentTest::testUpdateWrittenLength_incompleteLastPiece()
{
  SharedHandle<Piece> p(new Piece(0, 16*1024*9+2));
  PiecedSegment s(16*1024*10, p);

  s.updateWrittenLength(16*1024*9+1);
  CPPUNIT_ASSERT(!p->pieceComplete());
  s.updateWrittenLength(1);
  CPPUNIT_ASSERT(p->pieceComplete());
}

void SegmentTest::testClear()
{
  SharedHandle<Piece> p(new Piece(0, 16*1024*10));
  PiecedSegment s(16*1024*10, p);
  s.updateWrittenLength(16*1024*10);
  CPPUNIT_ASSERT_EQUAL(16*1024*10, s.getWrittenLength());
  s.clear();
  CPPUNIT_ASSERT_EQUAL(0, s.getWrittenLength());
}

} // namespace aria2
