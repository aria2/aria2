#include "GrowSegment.h"
#include "Piece.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class GrowSegmentTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(GrowSegmentTest);
  CPPUNIT_TEST(testUpdateWrittenLength);
  CPPUNIT_TEST(testClear);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testUpdateWrittenLength();
  void testClear();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GrowSegmentTest);

void GrowSegmentTest::testUpdateWrittenLength()
{
  GrowSegment segment(std::shared_ptr<Piece>(new Piece()));
  segment.updateWrittenLength(32_k);

  CPPUNIT_ASSERT_EQUAL((int64_t)32_k, segment.getPositionToWrite());
  CPPUNIT_ASSERT(!segment.complete());
  CPPUNIT_ASSERT(segment.getPiece()->pieceComplete());
}

void GrowSegmentTest::testClear()
{
  GrowSegment segment(std::shared_ptr<Piece>(new Piece()));
  segment.updateWrittenLength(32_k);
  CPPUNIT_ASSERT_EQUAL((int64_t)32_k, segment.getWrittenLength());
  segment.clear(nullptr);
  CPPUNIT_ASSERT_EQUAL((int64_t)0, segment.getWrittenLength());
}

} // namespace aria2
