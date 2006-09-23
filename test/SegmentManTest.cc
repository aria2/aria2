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
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testSaveAndLoad();
  void testNullBitfield();
  void testCancelSegmentOnNullBitfield();
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
    
    Segment2 seg1;
    segmentMan.getSegment2(seg1, 1);
    seg1.writtenLength = seg1.length;
    segmentMan.completeSegment2(1, seg1);
    
    Segment2 seg2;
    segmentMan.getSegment2(seg2, 2);
    seg2.writtenLength = 512*1024;
    segmentMan.updateSegment2(2, seg2);
    
    Segment2 seg3;
    segmentMan.getSegment2(seg3, 3);
    seg2.writtenLength = 512*1024;
    segmentMan.updateSegment2(2, seg2);
    
    segmentMan.save();
    
    CPPUNIT_ASSERT(saveFile.exists());

    SegmentMan segmentManLoad;
    segmentManLoad.ufilename = filename;
    segmentManLoad.load();

    CPPUNIT_ASSERT_EQUAL(segmentMan.totalSize, segmentManLoad.totalSize);
  
    Segment2 seg2Load;
    segmentManLoad.getSegment2(seg2Load, 2, seg2.index);
    CPPUNIT_ASSERT_EQUAL(seg2, seg2Load);
    
    Segment2 seg3Load;
    segmentManLoad.getSegment2(seg3Load, 3, seg3.index);
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

  Segment2 segment;
  CPPUNIT_ASSERT(segmentMan.getSegment2(segment, 1));
  CPPUNIT_ASSERT_EQUAL(Segment2(0, 0, 0), segment);

  Segment2 segment2;
  CPPUNIT_ASSERT(!segmentMan.getSegment2(segment2, 2));

  long long int totalLength = 1024*1024;
  segment.writtenLength = totalLength;
  CPPUNIT_ASSERT(segmentMan.updateSegment2(1, segment));
  CPPUNIT_ASSERT_EQUAL(totalLength, segmentMan.getDownloadLength());
  CPPUNIT_ASSERT(segmentMan.completeSegment2(1, segment));
  CPPUNIT_ASSERT_EQUAL(totalLength, segmentMan.getDownloadLength());
}

void SegmentManTest::testCancelSegmentOnNullBitfield() {
  SegmentMan segmentMan;
  
  Segment2 segment;
  CPPUNIT_ASSERT(segmentMan.getSegment2(segment, 1));
  CPPUNIT_ASSERT(segmentMan.cancelSegment2(1, segment));
  CPPUNIT_ASSERT(segmentMan.getSegment2(segment, 1));
}
