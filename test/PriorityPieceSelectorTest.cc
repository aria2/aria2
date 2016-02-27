#include "PriorityPieceSelector.h"

#include <cppunit/extensions/HelperMacros.h>

#include "array_fun.h"
#include "BitfieldMan.h"
#include "MockPieceSelector.h"
#include "a2functional.h"

namespace aria2 {

class PriorityPieceSelectorTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PriorityPieceSelectorTest);
  CPPUNIT_TEST(testSelect);
  CPPUNIT_TEST_SUITE_END();

public:
  void testSelect();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PriorityPieceSelectorTest);

void PriorityPieceSelectorTest::testSelect()
{
  constexpr size_t pieceLength = 1_k;
  size_t A[] = {1, 200};
  BitfieldMan bf(pieceLength, pieceLength * 256);
  for (auto i : A) {
    bf.setBit(i);
  }
  PriorityPieceSelector selector(
      std::shared_ptr<PieceSelector>(new MockPieceSelector()));
  selector.setPriorityPiece(std::begin(A), std::end(A));

  size_t index;
  CPPUNIT_ASSERT(selector.select(index, bf.getBitfield(), bf.countBlock()));
  CPPUNIT_ASSERT_EQUAL((size_t)1, index);
  bf.unsetBit(1);
  CPPUNIT_ASSERT(selector.select(index, bf.getBitfield(), bf.countBlock()));
  CPPUNIT_ASSERT_EQUAL((size_t)200, index);
  bf.unsetBit(200);
  CPPUNIT_ASSERT(!selector.select(index, bf.getBitfield(), bf.countBlock()));
}

} // namespace aria2
