#include "Sequence.h"
#include <deque>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class SequenceTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SequenceTest);
  CPPUNIT_TEST(testParseAndNext);
  CPPUNIT_TEST(testParseAndNext2);
  CPPUNIT_TEST(testFlush);
  CPPUNIT_TEST_SUITE_END();
public:
  void testParseAndNext();
  void testParseAndNext2();
  void testFlush();
};


CPPUNIT_TEST_SUITE_REGISTRATION(SequenceTest);

typedef Sequence<int> IntSequence;

void SequenceTest::testParseAndNext()
{
  IntSequence::Value params[] = {
    IntSequence::Value(1, 2),
    IntSequence::Value(3, 9),
    IntSequence::Value(10, 11),
  };
  IntSequence seq = IntSequence(IntSequence::Values(&params[0], &params[3]));
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL(1, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL(3, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL(4, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL(5, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL(6, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL(7, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL(8, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL(10, seq.next());
  CPPUNIT_ASSERT(!seq.hasNext());
  CPPUNIT_ASSERT_EQUAL(0, seq.next());
 
}

void SequenceTest::testParseAndNext2()
{
  IntSequence::Value params[] = {
    IntSequence::Value(1, 2),
  };
  IntSequence seq = IntSequence(IntSequence::Values(&params[0], &params[1]));
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL(1, seq.next());
  CPPUNIT_ASSERT(!seq.hasNext());
  CPPUNIT_ASSERT_EQUAL(0, seq.next());
 
}

void SequenceTest::testFlush()
{
  IntSequence::Value params[] = {
    IntSequence::Value(1, 2),
    IntSequence::Value(3, 9),
    IntSequence::Value(10, 11),
  };
  IntSequence seq = IntSequence(IntSequence::Values(&params[0], &params[3]));
  std::deque<int> r = seq.flush();

  int answers[] = { 1, 3, 4, 5, 6, 7, 8, 10 };

  CPPUNIT_ASSERT(std::equal(r.begin(), r.end(), &answers[0])); 
}

} // namespace aria2
