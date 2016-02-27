#include "LongestSequencePieceSelector.h"

#include <cppunit/extensions/HelperMacros.h>

#include "array_fun.h"
#include "BitfieldMan.h"
#include "a2functional.h"

namespace aria2 {

class LongestSequencePieceSelectorTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(LongestSequencePieceSelectorTest);
  CPPUNIT_TEST(testSelect);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testSelect();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LongestSequencePieceSelectorTest);

void LongestSequencePieceSelectorTest::testSelect()
{
  size_t A[] = {1, 2, 3, 4, 7, 10, 11, 12, 13, 14, 15, 100, 112, 113, 114};
  BitfieldMan bf(1_k, 256_k);
  for (size_t i = 0; i < arraySize(A); ++i) {
    bf.setBit(A[i]);
  }

  LongestSequencePieceSelector selector;
  size_t index;

  CPPUNIT_ASSERT(selector.select(index, bf.getBitfield(), bf.countBlock()));
  CPPUNIT_ASSERT_EQUAL((size_t)15, index);

  bf.clearAllBit();
  CPPUNIT_ASSERT(!selector.select(index, bf.getBitfield(), bf.countBlock()));

  // See it works in just one range
  bf.setBitRange(1, 4);
  CPPUNIT_ASSERT(selector.select(index, bf.getBitfield(), bf.countBlock()));
  CPPUNIT_ASSERT_EQUAL((size_t)4, index);
}

} // namespace aria2
