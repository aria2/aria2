#include "SegmentMan.h"
#include "File.h"
#include "prefs.h"
#include "Util.h"
#include "SingleFileDownloadContext.h"
#include "UnknownLengthPieceStorage.h"
#include "DefaultPieceStorage.h"
#include "Segment.h"
#include "Option.h"
#include "FileEntry.h"
#include "MockPieceStorage.h"
#include "PeerStat.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class SegmentManTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SegmentManTest);
  CPPUNIT_TEST(testNullBitfield);
  CPPUNIT_TEST(testCompleteSegment);
  CPPUNIT_TEST(testGetPeerStat);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testNullBitfield();
  void testCompleteSegment();
  void testGetPeerStat();
};


CPPUNIT_TEST_SUITE_REGISTRATION( SegmentManTest );

void SegmentManTest::testNullBitfield()
{
  Option op;
  SharedHandle<SingleFileDownloadContext> dctx
    (new SingleFileDownloadContext(0, 0, "aria2.tar.bz2"));
  SharedHandle<UnknownLengthPieceStorage> ps
    (new UnknownLengthPieceStorage(dctx, &op));
  SegmentMan segmentMan(&op, dctx, ps);

  SharedHandle<Segment> segment = segmentMan.getSegment(1);
  CPPUNIT_ASSERT(!segment.isNull());
  CPPUNIT_ASSERT_EQUAL((size_t)0, segment->getIndex());
  CPPUNIT_ASSERT_EQUAL((size_t)0, segment->getLength());
  CPPUNIT_ASSERT_EQUAL((size_t)0, segment->getSegmentLength());
  CPPUNIT_ASSERT_EQUAL((size_t)0, segment->getWrittenLength());

  SharedHandle<Segment> segment2 = segmentMan.getSegment(2);
  CPPUNIT_ASSERT(segment2.isNull());

  segmentMan.cancelSegment(1);
  CPPUNIT_ASSERT(!segmentMan.getSegment(2).isNull());
}

void SegmentManTest::testCompleteSegment()
{
  Option op;
  size_t pieceLength = 1024*1024;
  uint64_t totalLength = 64*1024*1024;
  SharedHandle<SingleFileDownloadContext> dctx
    (new SingleFileDownloadContext(pieceLength, totalLength, "aria2.tar.bz2"));
  SharedHandle<DefaultPieceStorage> ps(new DefaultPieceStorage(dctx, &op));

  SegmentMan segmentMan(&op, dctx, ps);

  CPPUNIT_ASSERT(!segmentMan.getSegment(1, 0).isNull());
  SharedHandle<Segment> seg = segmentMan.getSegment(1, 1);
  CPPUNIT_ASSERT(!seg.isNull());
  CPPUNIT_ASSERT(!segmentMan.getSegment(1, 2).isNull());

  seg->updateWrittenLength(pieceLength);
  segmentMan.completeSegment(1, seg);
  
  std::deque<SharedHandle<Segment> > segments;
  segmentMan.getInFlightSegment(segments, 1);
  CPPUNIT_ASSERT_EQUAL((size_t)2, segments.size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, segments[0]->getIndex());
  CPPUNIT_ASSERT_EQUAL((size_t)2, segments[1]->getIndex());
}

void SegmentManTest::testGetPeerStat()
{
  Option op;
  size_t pieceLength = 1;
  uint64_t totalLength = 1;
  SharedHandle<SingleFileDownloadContext> dctx
    (new SingleFileDownloadContext(pieceLength, totalLength, "aria2.tar.bz2"));
  SharedHandle<PieceStorage> ps(new MockPieceStorage());
  SegmentMan segmentMan(&op, dctx, ps);

  CPPUNIT_ASSERT(segmentMan.getPeerStat(1).isNull());
  SharedHandle<PeerStat> ps1(new PeerStat(1));
  CPPUNIT_ASSERT(segmentMan.registerPeerStat(ps1));
  {
    SharedHandle<PeerStat> ps = segmentMan.getPeerStat(1);
    CPPUNIT_ASSERT(!ps.isNull());
    CPPUNIT_ASSERT_EQUAL(1, ps->getCuid());
  }
  // Duplicate registering is not allowed.
  SharedHandle<PeerStat> ps1d(new PeerStat(1));
  CPPUNIT_ASSERT(!segmentMan.registerPeerStat(ps1d));
  
  // See whether it can work on several entries.
  SharedHandle<PeerStat> ps2(new PeerStat(2));
  SharedHandle<PeerStat> ps3(new PeerStat(3));
  CPPUNIT_ASSERT(segmentMan.registerPeerStat(ps3));
  CPPUNIT_ASSERT(segmentMan.registerPeerStat(ps2));
  {
    SharedHandle<PeerStat> ps = segmentMan.getPeerStat(2);
    CPPUNIT_ASSERT(!ps.isNull());
    CPPUNIT_ASSERT_EQUAL(2, ps->getCuid());
  }
}

} // namespace aria2
