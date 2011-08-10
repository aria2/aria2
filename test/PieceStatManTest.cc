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
    int ans[] = { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    const std::vector<size_t>& order(pieceStatMan.getOrder());
    const std::vector<int>& counts(pieceStatMan.getCounts());
    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(i, order[i]);
      CPPUNIT_ASSERT_EQUAL(ans[i], counts[i]);
    }
  }
  pieceStatMan.addPieceStats(1);
  {
    int ans[] = { 0, 2, 0, 0, 0, 0, 0, 0, 0, 0 };
    const std::vector<int>& counts(pieceStatMan.getCounts());
    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(ans[i], counts[i]);
    }
  }
  pieceStatMan.addPieceStats(3);
  pieceStatMan.addPieceStats(9);
  pieceStatMan.addPieceStats(3);
  pieceStatMan.addPieceStats(0);
  {
    int ans[] = {  1, 2, 0, 2, 0, 0, 0, 0, 0, 1 };
    const std::vector<int>& counts(pieceStatMan.getCounts());
    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(ans[i], counts[i]);
    }
  }
}

void PieceStatManTest::testAddPieceStats_bitfield()
{
  PieceStatMan pieceStatMan(10, false);
  const unsigned char bitfield[] = { 0xaa, 0x80 };
  pieceStatMan.addPieceStats(bitfield, sizeof(bitfield));
  {
    int ans[] = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
    const std::vector<int>& counts(pieceStatMan.getCounts());
    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(ans[i], counts[i]);
    }
  }
  pieceStatMan.addPieceStats(bitfield, sizeof(bitfield));
  {
    int ans[] = { 2, 0, 2, 0, 2, 0, 2, 0, 2, 0 };
    const std::vector<int>& counts(pieceStatMan.getCounts());
    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(ans[i], counts[i]);
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
    int ans[] =  { 0, 0, 0, 1, 2, 2, 2, 2, 1, 1 };
    const std::vector<int>& counts(pieceStatMan.getCounts());
    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(ans[i], counts[i]);
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
    int ans[] =  { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    const std::vector<int>& counts(pieceStatMan.getCounts());
    for(size_t i = 0; i < 10; ++i) {
      CPPUNIT_ASSERT_EQUAL(ans[i], counts[i]);
    }
  }
}

} // namespace aria2
