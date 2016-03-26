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

CPPUNIT_TEST_SUITE_REGISTRATION(SegmentTest);

void SegmentTest::testUpdateWrittenLength()
{
  std::shared_ptr<Piece> p(new Piece(0, 160_k));
  PiecedSegment s(160_k, p);
  CPPUNIT_ASSERT_EQUAL((int64_t)0, s.getWrittenLength());

  s.updateWrittenLength(16_k);
  CPPUNIT_ASSERT(p->hasBlock(0));
  CPPUNIT_ASSERT(!p->hasBlock(1));

  s.updateWrittenLength(16_k * 9);
  CPPUNIT_ASSERT(p->pieceComplete());
}

void SegmentTest::testUpdateWrittenLength_lastPiece()
{
  std::shared_ptr<Piece> p(new Piece(0, 16_k * 9 + 1));
  PiecedSegment s(160_k, p);

  s.updateWrittenLength(p->getLength());
  CPPUNIT_ASSERT(p->pieceComplete());
}

void SegmentTest::testUpdateWrittenLength_incompleteLastPiece()
{
  std::shared_ptr<Piece> p(new Piece(0, 16_k * 9 + 2));
  PiecedSegment s(160_k, p);

  s.updateWrittenLength(16_k * 9 + 1);
  CPPUNIT_ASSERT(!p->pieceComplete());
  s.updateWrittenLength(1);
  CPPUNIT_ASSERT(p->pieceComplete());
}

void SegmentTest::testClear()
{
  std::shared_ptr<Piece> p(new Piece(0, 160_k));
  PiecedSegment s(160_k, p);
  s.updateWrittenLength(160_k);
  CPPUNIT_ASSERT_EQUAL((int64_t)160_k, s.getWrittenLength());
  s.clear(nullptr);
  CPPUNIT_ASSERT_EQUAL((int64_t)0, s.getWrittenLength());
}

} // namespace aria2
