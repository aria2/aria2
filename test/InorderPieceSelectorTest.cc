#include "InorderPieceSelector.h"

#include <cppunit/extensions/HelperMacros.h>

#include "BitfieldMan.h"
#include "a2functional.h"

namespace aria2 {

class InorderPieceSelectorTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(InorderPieceSelectorTest);
  CPPUNIT_TEST(testSelect);
  CPPUNIT_TEST_SUITE_END();

public:
  void testSelect();
};

CPPUNIT_TEST_SUITE_REGISTRATION(InorderPieceSelectorTest);

void InorderPieceSelectorTest::testSelect()
{
  constexpr size_t pieceLength = 1_k;
  BitfieldMan bf(pieceLength, pieceLength * 6);
  bf.setAllBit();
  bf.unsetBit(1);
  bf.unsetBit(4);
  InorderPieceSelector selector;

  size_t index;
  CPPUNIT_ASSERT(selector.select(index, bf.getBitfield(), bf.countBlock()));
  CPPUNIT_ASSERT_EQUAL((size_t)0, index);
  bf.unsetBit(0);
  CPPUNIT_ASSERT(selector.select(index, bf.getBitfield(), bf.countBlock()));
  CPPUNIT_ASSERT_EQUAL((size_t)2, index);
  bf.unsetBit(2);
  CPPUNIT_ASSERT(selector.select(index, bf.getBitfield(), bf.countBlock()));
  CPPUNIT_ASSERT_EQUAL((size_t)3, index);
  bf.unsetBit(3);
  CPPUNIT_ASSERT(selector.select(index, bf.getBitfield(), bf.countBlock()));
  CPPUNIT_ASSERT_EQUAL((size_t)5, index);
  bf.unsetBit(5);
  CPPUNIT_ASSERT(!selector.select(index, bf.getBitfield(), bf.countBlock()));
}

} // namespace aria2
