#include "SegmentMan.h"
#include "File.h"
#include "prefs.h"
#include "Util.h"
#include "SingleFileDownloadContext.h"
#include "UnknownLengthPieceStorage.h"
#include "DefaultPieceStorage.h"
#include "Segment.h"
#include "Option.h"
#include "MockBtContext.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class SegmentManTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SegmentManTest);
  CPPUNIT_TEST(testNullBitfield);
  CPPUNIT_TEST(testCompleteSegment);
  CPPUNIT_TEST(testMarkPieceDone_usedSegment);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testNullBitfield();
  void testCompleteSegment();
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

void SegmentManTest::testCompleteSegment()
{
  Option op;
  int32_t pieceLength = 1024*1024;
  int64_t totalLength = 64*1024*1024;
  MockBtContextHandle dctx = new MockBtContext();
  dctx->setPieceLength(pieceLength);
  dctx->setTotalLength(totalLength);
  dctx->setNumPieces((totalLength+pieceLength-1)/pieceLength);
  DefaultPieceStorageHandle ps = new DefaultPieceStorage(dctx, &op);

  SegmentMan segmentMan(&op, dctx, ps);

  CPPUNIT_ASSERT(!segmentMan.getSegment(1, 0).isNull());
  SegmentHandle seg = segmentMan.getSegment(1, 1);
  CPPUNIT_ASSERT(!seg.isNull());
  CPPUNIT_ASSERT(!segmentMan.getSegment(1, 2).isNull());

  seg->updateWrittenLength(pieceLength);
  segmentMan.completeSegment(1, seg);
  
  Segments segments = segmentMan.getInFlightSegment(1);
  CPPUNIT_ASSERT_EQUAL((size_t)2, segments.size());
  CPPUNIT_ASSERT_EQUAL(0, segments[0]->getIndex());
  CPPUNIT_ASSERT_EQUAL(2, segments[1]->getIndex());
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
