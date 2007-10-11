#include "GrowSegment.h"
#include "Piece.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class GrowSegmentTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(GrowSegmentTest);
  CPPUNIT_TEST(testUpdateWrittenLength);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testUpdateWrittenLength();
};


CPPUNIT_TEST_SUITE_REGISTRATION( GrowSegmentTest );

void GrowSegmentTest::testUpdateWrittenLength()
{
  GrowSegment segment(new Piece());
  segment.updateWrittenLength(32*1024);
  
  CPPUNIT_ASSERT_EQUAL((int64_t)32*1024, segment.getPositionToWrite());
  CPPUNIT_ASSERT(!segment.complete());
  CPPUNIT_ASSERT(segment.getPiece()->pieceComplete());
}
