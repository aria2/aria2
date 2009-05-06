#include "PieceStatMan.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class PieceStatManTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PieceStatManTest);
  CPPUNIT_TEST(testAddPieceStats_index);
  CPPUNIT_TEST(testAddPieceStats_bitfield);
  CPPUNIT_TEST(testUpdatePieceStats);
  CPPUNIT_TEST(testSubtractPieceStats);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testAddPieceStats_index();
  void testAddPieceStats_bitfield();
  void testUpdatePieceStats();
  void testSubtractPieceStats();
};


CPPUNIT_TEST_SUITE_REGISTRATION(PieceStatManTest);

void PieceStatManTest::testAddPieceStats_index()
{
  PieceStatMan pieceStatMan(10, false);
  pieceStatMan.addPieceStats(1);
  {
    size_t indexes[] = { 0, 2, 3, 4, 5, 6, 7, 8, 9, 1 };
    size_t counts[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
    
    const std::vector<size_t>& statsidx(pieceStatMan.getRarerPieceIndexes());
    const std::vector<SharedHandle<PieceStat> >& stats
      (pieceStatMan.getPieceStats());

    CPPUNIT_ASSERT_EQUAL((size_t)10, stats.size());
    
    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(indexes[i], statsidx[i]);
      CPPUNIT_ASSERT_EQUAL(counts[i], stats[statsidx[i]]->getCount());
    }
  }

  pieceStatMan.addPieceStats(1);

  {
    size_t indexes[] = { 0, 2, 3, 4, 5, 6, 7, 8, 9, 1 };
    size_t counts[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 };

    const std::vector<size_t>& statsidx(pieceStatMan.getRarerPieceIndexes());
    const std::vector<SharedHandle<PieceStat> >& stats
      (pieceStatMan.getPieceStats());

    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(indexes[i], statsidx[i]);
      CPPUNIT_ASSERT_EQUAL(counts[i], stats[statsidx[i]]->getCount());
    }
  }

  pieceStatMan.addPieceStats(3);
  pieceStatMan.addPieceStats(9);
  pieceStatMan.addPieceStats(3);
  pieceStatMan.addPieceStats(0);

  {
    size_t indexes[] = { 2, 4, 5, 6, 7, 8, 0, 9, 1, 3 };
    size_t counts[] = {  0, 0, 0, 0, 0, 0, 1, 1, 2, 2 };

    const std::vector<size_t>& statsidx(pieceStatMan.getRarerPieceIndexes());
    const std::vector<SharedHandle<PieceStat> >& stats
      (pieceStatMan.getPieceStats());

    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(indexes[i], statsidx[i]);
      CPPUNIT_ASSERT_EQUAL(counts[i], stats[statsidx[i]]->getCount());
    }
  }

}

void PieceStatManTest::testAddPieceStats_bitfield()
{
  PieceStatMan pieceStatMan(10, false);
  const unsigned char bitfield[] = { 0xaa, 0x80 };
  pieceStatMan.addPieceStats(bitfield, sizeof(bitfield));
  {
    size_t indexes[] = { 1, 3, 5, 7, 9, 0, 2, 4, 6, 8 };
    size_t counts[] = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 };

    const std::vector<size_t>& statsidx(pieceStatMan.getRarerPieceIndexes());
    const std::vector<SharedHandle<PieceStat> >& stats
      (pieceStatMan.getPieceStats());

    CPPUNIT_ASSERT_EQUAL((size_t)10, stats.size());
    
    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(indexes[i], statsidx[i]);
      CPPUNIT_ASSERT_EQUAL(counts[i], stats[statsidx[i]]->getCount());
    }
  }

  pieceStatMan.addPieceStats(bitfield, sizeof(bitfield));

  {
    size_t indexes[] = { 1, 3, 5, 7, 9, 0, 2, 4, 6, 8 };
    size_t counts[] = { 0, 0, 0, 0, 0, 2, 2, 2, 2, 2 };

    const std::vector<size_t>& statsidx(pieceStatMan.getRarerPieceIndexes());
    const std::vector<SharedHandle<PieceStat> >& stats
      (pieceStatMan.getPieceStats());

    CPPUNIT_ASSERT_EQUAL((size_t)10, stats.size());
    
    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(indexes[i], statsidx[i]);
      CPPUNIT_ASSERT_EQUAL(counts[i], stats[statsidx[i]]->getCount());
    }
  }
}

void PieceStatManTest::testUpdatePieceStats()
{
  PieceStatMan pieceStatMan(10, false);

  const unsigned char bitfield[] = { 0xff, 0xc0 };
  pieceStatMan.addPieceStats(bitfield, sizeof(bitfield));

  const unsigned char oldBitfield[] = { 0xf0, 0x00 };
  const unsigned char newBitfield[] = { 0x1f, 0x00 };

  pieceStatMan.updatePieceStats(newBitfield, sizeof(newBitfield), oldBitfield);
  {
    // idx: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    // bf : 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    // old: 1, 1, 1, 1, 0, 0, 0, 0, 0, 0
    // new: 0, 0, 0, 1, 1, 1, 1, 1, 0, 0
    // ---------------------------------
    // res: 0, 0, 0, 1, 2, 2, 2, 2, 1, 1

    size_t indexes[] = { 0, 1, 2, 3, 8, 9, 4, 5, 6, 7 };
    size_t counts[] =  { 0, 0, 0, 1, 1, 1, 2, 2, 2, 2 };

    const std::vector<size_t>& statsidx(pieceStatMan.getRarerPieceIndexes());
    const std::vector<SharedHandle<PieceStat> >& stats
      (pieceStatMan.getPieceStats());

    CPPUNIT_ASSERT_EQUAL((size_t)10, stats.size());
    
    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(indexes[i], statsidx[i]);
      CPPUNIT_ASSERT_EQUAL(counts[i], stats[statsidx[i]]->getCount());
    }
  }
}

void PieceStatManTest::testSubtractPieceStats()
{
  PieceStatMan pieceStatMan(10, false);

  const unsigned char bitfield[] = { 0xf0, 0x00 };
  pieceStatMan.addPieceStats(bitfield, sizeof(bitfield));

  const unsigned char newBitfield[] = { 0x3f, 0x00 };

  pieceStatMan.subtractPieceStats(newBitfield, sizeof(newBitfield));
  {
    // idx: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    // bf : 1, 1, 1, 1, 0, 0, 0, 0, 0, 0
    // new: 0, 0, 1, 1, 1, 1, 1, 1, 0, 0
    // ---------------------------------
    // res: 1, 1, 0, 0, 0, 0, 0, 0, 0, 0

    size_t indexes[] = { 2, 3, 4, 5, 6, 7, 8, 9, 0, 1 };
    size_t counts[] =  { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 };

    const std::vector<size_t>& statsidx(pieceStatMan.getRarerPieceIndexes());
    const std::vector<SharedHandle<PieceStat> >& stats
      (pieceStatMan.getPieceStats());

    CPPUNIT_ASSERT_EQUAL((size_t)10, stats.size());
    
    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(indexes[i], statsidx[i]);
      CPPUNIT_ASSERT_EQUAL(counts[i], stats[statsidx[i]]->getCount());
    }
  }
}

} // namespace aria2
