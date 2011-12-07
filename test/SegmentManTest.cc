#include "SegmentMan.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DownloadContext.h"
#include "UnknownLengthPieceStorage.h"
#include "DefaultPieceStorage.h"
#include "Segment.h"
#include "Option.h"
#include "PieceSelector.h"
#include "FileEntry.h"
#include "PeerStat.h"

namespace aria2 {

class SegmentManTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SegmentManTest);
  CPPUNIT_TEST(testNullBitfield);
  CPPUNIT_TEST(testCompleteSegment);
  CPPUNIT_TEST(testGetSegment_sameFileEntry);
  CPPUNIT_TEST(testRegisterPeerStat);
  CPPUNIT_TEST(testCancelAllSegments);
  CPPUNIT_TEST(testGetPeerStat);
  CPPUNIT_TEST(testGetCleanSegmentIfOwnerIsIdle);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Option> option_;
  SharedHandle<DownloadContext> dctx_;
  SharedHandle<DefaultPieceStorage> pieceStorage_;
  SharedHandle<SegmentMan> segmentMan_;
public:
  void setUp()
  {
    size_t pieceLength = 1024*1024;
    uint64_t totalLength = 64*1024*1024;
    option_.reset(new Option());
    dctx_.reset
      (new DownloadContext(pieceLength, totalLength, "aria2.tar.bz2"));
    pieceStorage_.reset(new DefaultPieceStorage(dctx_, option_.get()));
    segmentMan_.reset(new SegmentMan(option_.get(), dctx_, pieceStorage_));
  }

  void testNullBitfield();
  void testCompleteSegment();
  void testGetSegment_sameFileEntry();
  void testRegisterPeerStat();
  void testCancelAllSegments();
  void testGetPeerStat();
  void testGetCleanSegmentIfOwnerIsIdle();
};


CPPUNIT_TEST_SUITE_REGISTRATION( SegmentManTest );

void SegmentManTest::testNullBitfield()
{
  Option op;
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(0, 0, "aria2.tar.bz2"));
  SharedHandle<UnknownLengthPieceStorage> ps
    (new UnknownLengthPieceStorage(dctx, &op));
  SegmentMan segmentMan(&op, dctx, ps);
  size_t minSplitSize = dctx->getPieceLength();

  SharedHandle<Segment> segment = segmentMan.getSegment(1, minSplitSize);
  CPPUNIT_ASSERT(segment);
  CPPUNIT_ASSERT_EQUAL((size_t)0, segment->getIndex());
  CPPUNIT_ASSERT_EQUAL(0, segment->getLength());
  CPPUNIT_ASSERT_EQUAL(0, segment->getSegmentLength());
  CPPUNIT_ASSERT_EQUAL(0, segment->getWrittenLength());

  SharedHandle<Segment> segment2 = segmentMan.getSegment(2, minSplitSize);
  CPPUNIT_ASSERT(!segment2);

  segmentMan.cancelSegment(1);
  CPPUNIT_ASSERT(segmentMan.getSegment(2, minSplitSize));
}

void SegmentManTest::testCompleteSegment()
{
  Option op;
  size_t pieceLength = 1024*1024;
  uint64_t totalLength = 64*1024*1024;
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(pieceLength, totalLength, "aria2.tar.bz2"));
  SharedHandle<DefaultPieceStorage> ps(new DefaultPieceStorage(dctx, &op));

  SegmentMan segmentMan(&op, dctx, ps);

  CPPUNIT_ASSERT(segmentMan.getSegmentWithIndex(1, 0));
  SharedHandle<Segment> seg = segmentMan.getSegmentWithIndex(1, 1);
  CPPUNIT_ASSERT(seg);
  CPPUNIT_ASSERT(segmentMan.getSegmentWithIndex(1, 2));

  seg->updateWrittenLength(pieceLength);
  segmentMan.completeSegment(1, seg);
  
  std::vector<SharedHandle<Segment> > segments;
  segmentMan.getInFlightSegment(segments, 1);
  CPPUNIT_ASSERT_EQUAL((size_t)2, segments.size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, segments[0]->getIndex());
  CPPUNIT_ASSERT_EQUAL((size_t)2, segments[1]->getIndex());
}

void SegmentManTest::testGetSegment_sameFileEntry()
{
  Option op;
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  dctx->setPieceLength(2);
  SharedHandle<FileEntry> fileEntries[] = {
    SharedHandle<FileEntry>(new FileEntry("file1", 3, 0)),
    SharedHandle<FileEntry>(new FileEntry("file2", 6, 3)),
    SharedHandle<FileEntry>(new FileEntry("file3", 1, 9))
  };
  dctx->setFileEntries(&fileEntries[0], &fileEntries[3]);
  SharedHandle<DefaultPieceStorage> ps(new DefaultPieceStorage(dctx, &op));
  SegmentMan segman(&op, dctx, ps);
  size_t minSplitSize =dctx->getPieceLength();
  std::vector<SharedHandle<Segment> > segments;
  segman.getSegment(segments, 1, minSplitSize, fileEntries[1], 4);
  // See 3 segments are returned, not 4 because the part of file1 is
  // not filled in segment#1
  CPPUNIT_ASSERT_EQUAL((size_t)3, segments.size());
  
  SharedHandle<Segment> segmentNo1 = segman.getSegmentWithIndex(2, 1);
  // Fill the part of file1 in segment#1
  segmentNo1->updateWrittenLength(1);
  segman.cancelSegment(2);

  segman.cancelSegment(1);
  segments.clear();
  segman.getSegment(segments, 1, minSplitSize, fileEntries[1], 4);
  CPPUNIT_ASSERT_EQUAL((size_t)4, segments.size());

  segman.cancelSegment(1);
  SharedHandle<Segment> segmentNo4 = segman.getSegmentWithIndex(1, 4);
  // Fill the part of file2 in segment#4
  segmentNo4->updateWrittenLength(1);
  segman.cancelSegment(1);

  segments.clear();
  segman.getSegment(segments, 1, minSplitSize, fileEntries[1], 4);
  // segment#4 is not returned because the part of file2 is filled.
  CPPUNIT_ASSERT_EQUAL((size_t)3, segments.size());
}

void SegmentManTest::testRegisterPeerStat()
{
  Option op;
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  SharedHandle<DefaultPieceStorage> ps(new DefaultPieceStorage(dctx, &op));
  SegmentMan segman(&op, dctx, ps);
  
  SharedHandle<PeerStat> p1(new PeerStat(0, "host1", "http"));
  segman.registerPeerStat(p1);
  CPPUNIT_ASSERT_EQUAL((size_t)1, segman.getPeerStats().size());
  SharedHandle<PeerStat> p2(new PeerStat(0, "host2", "http"));
  segman.registerPeerStat(p2);
  CPPUNIT_ASSERT_EQUAL((size_t)1, segman.getPeerStats().size());
  p2->downloadStart();
  segman.registerPeerStat(p1);
  CPPUNIT_ASSERT_EQUAL((size_t)2, segman.getPeerStats().size());
}

void SegmentManTest::testCancelAllSegments()
{
  segmentMan_->getSegmentWithIndex(1, 0);
  segmentMan_->getSegmentWithIndex(2, 1);
  CPPUNIT_ASSERT(!segmentMan_->getSegmentWithIndex(3, 0));
  CPPUNIT_ASSERT(!segmentMan_->getSegmentWithIndex(4, 1));
  segmentMan_->cancelAllSegments();
  CPPUNIT_ASSERT(segmentMan_->getSegmentWithIndex(3, 0));
  CPPUNIT_ASSERT(segmentMan_->getSegmentWithIndex(4, 1));
}

void SegmentManTest::testGetPeerStat()
{
  SharedHandle<PeerStat> peerStat1(new PeerStat(1));
  segmentMan_->registerPeerStat(peerStat1);
  CPPUNIT_ASSERT_EQUAL((cuid_t)1, segmentMan_->getPeerStat(1)->getCuid());
}

void SegmentManTest::testGetCleanSegmentIfOwnerIsIdle()
{
  SharedHandle<Segment> seg1 = segmentMan_->getSegmentWithIndex(1, 0);
  SharedHandle<Segment> seg2 = segmentMan_->getSegmentWithIndex(2, 1);
  seg2->updateWrittenLength(100);
  CPPUNIT_ASSERT(segmentMan_->getCleanSegmentIfOwnerIsIdle(3, 0));
  SharedHandle<PeerStat> peerStat3(new PeerStat(3));
  segmentMan_->registerPeerStat(peerStat3);
  CPPUNIT_ASSERT(segmentMan_->getCleanSegmentIfOwnerIsIdle(4, 0));
  SharedHandle<PeerStat> peerStat4(new PeerStat(4));
  peerStat4->downloadStart();
  segmentMan_->registerPeerStat(peerStat4);
  // Owner PeerStat is not IDLE
  CPPUNIT_ASSERT(!segmentMan_->getCleanSegmentIfOwnerIsIdle(5, 0));
  // Segment::updateWrittenLength != 0
  CPPUNIT_ASSERT(!segmentMan_->getCleanSegmentIfOwnerIsIdle(5, 1));
}

} // namespace aria2
