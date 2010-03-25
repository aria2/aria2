#include "PriorityPieceSelector.h"

#include <cppunit/extensions/HelperMacros.h>

#include "array_fun.h"
#include "BitfieldMan.h"
#include "MockPieceSelector.h"

namespace aria2 {

class PriorityPieceSelectorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PriorityPieceSelectorTest);
  CPPUNIT_TEST(testSelect);
  CPPUNIT_TEST_SUITE_END();
public:
  void testSelect();
};


CPPUNIT_TEST_SUITE_REGISTRATION(PriorityPieceSelectorTest);

void PriorityPieceSelectorTest::testSelect()
{
  size_t pieceLength = 1024;
  size_t A[] = { 1,200};
  BitfieldMan bf(pieceLength, pieceLength*256);
  for(size_t i = 0; i < A2_ARRAY_LEN(A); ++i) {
    bf.setBit(A[i]);
  }
  PriorityPieceSelector selector
    (SharedHandle<PieceSelector>(new MockPieceSelector()));
  selector.setPriorityPiece(vbegin(A), vend(A));

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
