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


CPPUNIT_TEST_SUITE_REGISTRATION( GrowSegmentTest );

void GrowSegmentTest::testUpdateWrittenLength()
{
  GrowSegment segment(SharedHandle<Piece>(new Piece()));
  segment.updateWrittenLength(32*1024);
  
  CPPUNIT_ASSERT_EQUAL((int64_t)32*1024, segment.getPositionToWrite());
  CPPUNIT_ASSERT(!segment.complete());
  CPPUNIT_ASSERT(segment.getPiece()->pieceComplete());
}

void GrowSegmentTest::testClear()
{
  GrowSegment segment(SharedHandle<Piece>(new Piece()));
  segment.updateWrittenLength(32*1024);
  CPPUNIT_ASSERT_EQUAL(32*1024, segment.getWrittenLength());
  segment.clear();
  CPPUNIT_ASSERT_EQUAL(0, segment.getWrittenLength());  
}

} // namespace aria2
