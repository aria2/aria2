#include "a2algo.h"

#include <cppunit/extensions/HelperMacros.h>

#include "array_fun.h"

namespace aria2 {

class a2algoTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(a2algoTest);
  CPPUNIT_TEST(testSelect);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testSelect();
};


CPPUNIT_TEST_SUITE_REGISTRATION(a2algoTest);

void a2algoTest::testSelect()
{
  size_t A[] = { 1,2,3,4,7,10,11,12,13,14,15,100,112,113,114 };

  std::pair<size_t*, size_t> p = max_sequence(vbegin(A), vend(A));
  CPPUNIT_ASSERT_EQUAL(&A[5], p.first);
  CPPUNIT_ASSERT_EQUAL((size_t)6, p.second);


  p = max_sequence(&A[0], &A[0]);
  CPPUNIT_ASSERT_EQUAL(&A[0], p.first);
  CPPUNIT_ASSERT_EQUAL((size_t)0, p.second);


  p = max_sequence(&A[0], &A[4]);
  CPPUNIT_ASSERT_EQUAL(&A[0], p.first);
  CPPUNIT_ASSERT_EQUAL((size_t)4, p.second);
}

} // namespace aria2
