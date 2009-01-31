#include "LongestSequencePieceSelector.h"

#include <cppunit/extensions/HelperMacros.h>

#include "array_fun.h"

namespace aria2 {

class LongestSequencePieceSelectorTest:public CppUnit::TestFixture {

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
  size_t A[] = { 1,2,3,4,7,10,11,12,13,14,15,100,112,113,114 };
  std::deque<size_t> indexes(&A[0], &A[arrayLength(A)]);

  LongestSequencePieceSelector selector;
  size_t index;

  CPPUNIT_ASSERT(selector.select(index, indexes));
  CPPUNIT_ASSERT_EQUAL((size_t)15, index);

  std::deque<size_t> zeroindexes;
  CPPUNIT_ASSERT(!selector.select(index, zeroindexes));

  std::deque<size_t> oneseq(&A[0], &A[4]);
  CPPUNIT_ASSERT(selector.select(index, oneseq));
  CPPUNIT_ASSERT_EQUAL((size_t)4, index);
}

} // namespace aria2
