#include "RarestPieceSelector.h"

#include <cppunit/extensions/HelperMacros.h>

#include "BitfieldMan.h"
#include "PieceStatMan.h"

namespace aria2 {

class RarestPieceSelectorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RarestPieceSelectorTest);
  CPPUNIT_TEST(testSelect);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testAddPieceStats_index();
  void testAddPieceStats_bitfield();
  void testUpdatePieceStats();
  void testSubtractPieceStats();
  void testSelect();
};


CPPUNIT_TEST_SUITE_REGISTRATION(RarestPieceSelectorTest);

void RarestPieceSelectorTest::testSelect()
{
  SharedHandle<PieceStatMan> pieceStatMan(new PieceStatMan(10, false));
  RarestPieceSelector selector(pieceStatMan);
  BitfieldMan bf(1024, 10*1024);
  bf.setBitRange(0, 2);
  size_t index;

  pieceStatMan->addPieceStats(0);

  CPPUNIT_ASSERT(selector.select(index, bf.getBitfield(),
                                 bf.countBlock()));
  CPPUNIT_ASSERT_EQUAL((size_t)1, index);

  pieceStatMan->addPieceStats(1);

  CPPUNIT_ASSERT(selector.select(index, bf.getBitfield(),
                                 bf.countBlock()));
  CPPUNIT_ASSERT_EQUAL((size_t)2, index);
}

} // namespace aria2
