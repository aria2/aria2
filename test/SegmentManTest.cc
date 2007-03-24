#include "SegmentMan.h"
#include "File.h"
#include "prefs.h"
#include "Util.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class SegmentManTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SegmentManTest);
  CPPUNIT_TEST(testSaveAndLoad);
  CPPUNIT_TEST(testNullBitfield);
  CPPUNIT_TEST(testCancelSegmentOnNullBitfield);
  CPPUNIT_TEST(testMarkPieceDone);
  CPPUNIT_TEST(testMarkPieceDone_usedSegment);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testSaveAndLoad();
  void testNullBitfield();
  void testCancelSegmentOnNullBitfield();
  void testMarkPieceDone();
  void testMarkPieceDone_usedSegment();
};


CPPUNIT_TEST_SUITE_REGISTRATION( SegmentManTest );

void SegmentManTest::testSaveAndLoad() {
  string filename = "save-temp";
  string aria2filename = filename+".aria2";
  File saveFile(aria2filename);
  if(saveFile.exists()) {
    saveFile.remove();
    assert(!saveFile.exists());
  }
  try {
    SegmentMan segmentMan;
    segmentMan.totalSize = 10*1024*1024;
    segmentMan.ufilename = filename;
    segmentMan.initBitfield(1024*1024, segmentMan.totalSize);
    
    SegmentHandle seg1 = segmentMan.getSegment(1);
    seg1->writtenLength = seg1->length;
    segmentMan.completeSegment(1, seg1);
    
    SegmentHandle seg2 = segmentMan.getSegment(2);
    seg2->writtenLength = 512*1024;
    //segmentMan.updateSegment(2, seg2);
    
    SegmentHandle seg3 = segmentMan.getSegment(3);
    seg3->writtenLength = 512*1024;
    //segmentMan.updateSegment(2, seg2);
    
    segmentMan.save();
    
    CPPUNIT_ASSERT(saveFile.exists());

    SegmentMan segmentManLoad;
    segmentManLoad.ufilename = filename;
    segmentManLoad.load();

    CPPUNIT_ASSERT_EQUAL(segmentMan.totalSize, segmentManLoad.totalSize);
  
    SegmentHandle seg2Load = segmentManLoad.getSegment(2, seg2->index);
    CPPUNIT_ASSERT_EQUAL(seg2, seg2Load);
    
    SegmentHandle seg3Load = segmentManLoad.getSegment(3, seg3->index);
    CPPUNIT_ASSERT_EQUAL(seg3, seg3Load);

    CPPUNIT_ASSERT_EQUAL(segmentMan.getDownloadLength(), segmentManLoad.getDownloadLength());
  } catch(Exception* e) {
    cerr << e->getMsg() << endl;
    delete e;
  }
}

void SegmentManTest::testNullBitfield() {
  SegmentMan segmentMan;
  Option op;
  op.put(PREF_SEGMENT_SIZE, Util::itos(1024*1024));
  segmentMan.option = &op;

  SegmentHandle segment = segmentMan.getSegment(1);
  CPPUNIT_ASSERT(!segment.isNull());
  CPPUNIT_ASSERT_EQUAL(0, segment->index);
  CPPUNIT_ASSERT_EQUAL(0, segment->length);
  CPPUNIT_ASSERT_EQUAL(0, segment->segmentLength);
  CPPUNIT_ASSERT_EQUAL(0, segment->writtenLength);

  SegmentHandle segment2 = segmentMan.getSegment(2);
  CPPUNIT_ASSERT(segment2.isNull());

  int64_t totalLength = 1024*1024;
  segment->writtenLength = totalLength;
  CPPUNIT_ASSERT_EQUAL(totalLength, segmentMan.getDownloadLength());
  CPPUNIT_ASSERT(segmentMan.completeSegment(1, segment));
  CPPUNIT_ASSERT_EQUAL(totalLength, segmentMan.getDownloadLength());
}

void SegmentManTest::testCancelSegmentOnNullBitfield() {
  SegmentMan segmentMan;
  
  SegmentHandle segment = segmentMan.getSegment(1);
  CPPUNIT_ASSERT(!segment.isNull());
  segmentMan.cancelSegment(1);
  CPPUNIT_ASSERT(!segmentMan.getSegment(1).isNull());
}

void SegmentManTest::testMarkPieceDone()
{
  SegmentMan segmentMan;
  int32_t pieceLength = 1024*1024;
  int64_t totalLength = 10*pieceLength;
  segmentMan.initBitfield(pieceLength, totalLength);
  segmentMan.markPieceDone(5*pieceLength);

  for(int32_t i = 0; i < 5; ++i) {
    CPPUNIT_ASSERT(segmentMan.hasSegment(i));
  }
  for(int32_t i = 5; i < 10; ++i) {
    CPPUNIT_ASSERT(!segmentMan.hasSegment(i));
  }
}

void SegmentManTest::testMarkPieceDone_usedSegment()
{
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
  CPPUNIT_ASSERT_EQUAL(5, segment->index);
  CPPUNIT_ASSERT_EQUAL(pieceLength, segment->length);
  CPPUNIT_ASSERT_EQUAL(pieceLength, segment->segmentLength);
  CPPUNIT_ASSERT_EQUAL(100, segment->writtenLength);
}
