#include "SegmentMan.h"
#include "File.h"
#include "prefs.h"
#include "Util.h"
#include "SingleFileDownloadContext.h"
#include "UnknownLengthPieceStorage.h"
#include "Segment.h"
#include "Option.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class SegmentManTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SegmentManTest);
  CPPUNIT_TEST(testNullBitfield);
  CPPUNIT_TEST(testMarkPieceDone_usedSegment);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testNullBitfield();
  void testMarkPieceDone_usedSegment();
};


CPPUNIT_TEST_SUITE_REGISTRATION( SegmentManTest );

void SegmentManTest::testNullBitfield()
{
  Option op;
  SingleFileDownloadContextHandle dctx = new SingleFileDownloadContext(0, 0, "aria2.tar.bz2");
  UnknownLengthPieceStorageHandle ps = new UnknownLengthPieceStorage(dctx, &op);
  SegmentMan segmentMan(&op, dctx, ps);

  SegmentHandle segment = segmentMan.getSegment(1);
  CPPUNIT_ASSERT(!segment.isNull());
  CPPUNIT_ASSERT_EQUAL((int32_t)0, segment->getIndex());
  CPPUNIT_ASSERT_EQUAL((int32_t)0, segment->getLength());
  CPPUNIT_ASSERT_EQUAL((int32_t)0, segment->getSegmentLength());
  CPPUNIT_ASSERT_EQUAL((int32_t)0, segment->getWrittenLength());

  SegmentHandle segment2 = segmentMan.getSegment(2);
  CPPUNIT_ASSERT(segment2.isNull());

  segmentMan.cancelSegment(1);
  CPPUNIT_ASSERT(!segmentMan.getSegment(2).isNull());
}

void SegmentManTest::testMarkPieceDone_usedSegment()
{
  // TODO implement this later
  /*
  SegmentMan segmentMan;
  int32_t pieceLength = 1024*1024;
  int64_t totalLength = 10*pieceLength;
  segmentMan.initBitfield(pieceLength, totalLength);
  segmentMan.markPieceDone(5*pieceLength+100);

  for(int32_t i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(segmentMan.hasSegment(i));
  }
  for(int32_t i = 5; i < 10; ++i) {
    CPPUNIT_ASSERT(!segmentMan.hasSegment(i));
  }

  SegmentHandle segment = segmentMan.getSegment(0, 5);
  CPPUNIT_ASSERT(!segment.isNull());
  CPPUNIT_ASSERT_EQUAL((int32_t)5, segment->index);
  CPPUNIT_ASSERT_EQUAL(pieceLength, (int32_t) segment->length);
  CPPUNIT_ASSERT_EQUAL(pieceLength, (int32_t) segment->segmentLength);
  CPPUNIT_ASSERT_EQUAL((int32_t)100, segment->writtenLength);
  */
}
