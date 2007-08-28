#include "ParameterizedStringParser.h"
#include "PStringSelect.h"
#include "PStringSegment.h"
#include "PStringNumLoop.h"
#include "FatalException.h"
#include <cppunit/extensions/HelperMacros.h>

class ParameterizedStringParserTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ParameterizedStringParserTest);
  CPPUNIT_TEST(testParse_select);
  CPPUNIT_TEST(testParse_select_empty);
  CPPUNIT_TEST(testParse_select_missingParen);
  CPPUNIT_TEST(testParse_segment);
  CPPUNIT_TEST(testParse_segment_select);
  CPPUNIT_TEST(testParse_loop);
  CPPUNIT_TEST(testParse_loop_empty);
  CPPUNIT_TEST(testParse_loop_missingParen);
  CPPUNIT_TEST(testParse_loop_missingStep);
  CPPUNIT_TEST(testParse_loop_missingRange);
  CPPUNIT_TEST(testParse_alphaLoop);
  CPPUNIT_TEST(testParse_loop_mixedChar);
  CPPUNIT_TEST(testParse_loop_mixedCase);
  CPPUNIT_TEST(testParse_segment_loop);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testParse_select();
  void testParse_select_empty();
  void testParse_select_missingParen();
  void testParse_segment();
  void testParse_segment_select();
  void testParse_loop();
  void testParse_loop_empty();
  void testParse_loop_missingParen();
  void testParse_loop_missingStep();
  void testParse_loop_missingRange();
  void testParse_alphaLoop();
  void testParse_loop_mixedChar();
  void testParse_loop_mixedCase();
  void testParse_segment_loop();
};


CPPUNIT_TEST_SUITE_REGISTRATION( ParameterizedStringParserTest );

void ParameterizedStringParserTest::testParse_select()
{
  PStringDatumHandle ls = ParameterizedStringParser().parse("{alpha, bravo, charlie}");
  PStringSelectHandle select = ls;
  CPPUNIT_ASSERT(!select.isNull());

  Strings values = select->getValues();
  CPPUNIT_ASSERT_EQUAL((size_t)3, values.size());

  CPPUNIT_ASSERT_EQUAL(string("alpha"), values[0]);
  CPPUNIT_ASSERT_EQUAL(string("bravo"), values[1]);
  CPPUNIT_ASSERT_EQUAL(string("charlie"), values[2]);
}

void ParameterizedStringParserTest::testParse_select_empty()
{
  try {
    PStringDatumHandle ls = ParameterizedStringParser().parse("{}");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(FatalException* e) {
    cerr << e->getMsg() << endl;
    delete e;
  } catch(...) {
    CPPUNIT_FAIL("unexpected exception thrown.");
  }
}

void ParameterizedStringParserTest::testParse_select_missingParen()
{
  try {
    PStringDatumHandle ls = ParameterizedStringParser().parse("{alpha");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(FatalException* e) {
    cerr << e->getMsg() << endl;
    delete e;
  } catch(...) {
    CPPUNIT_FAIL("unexpected exception was thrown.");
  }
}

void ParameterizedStringParserTest::testParse_segment()
{
  PStringDatumHandle ls = ParameterizedStringParser().parse("hello world");
  PStringSegmentHandle segment = ls;
  CPPUNIT_ASSERT(!segment.isNull());
  CPPUNIT_ASSERT_EQUAL(string("hello world"), segment->getValue());
}

void ParameterizedStringParserTest::testParse_segment_select()
{
  PStringDatumHandle ls = ParameterizedStringParser().parse("file:///{alpha, bravo, charlie}/tango");

  PStringSegmentHandle segment1 = ls;
  CPPUNIT_ASSERT(!segment1.isNull());
  CPPUNIT_ASSERT_EQUAL(string("file:///"), segment1->getValue());

  PStringSelectHandle select1 = segment1->getNext();
  CPPUNIT_ASSERT(!select1.isNull());
  Strings selectValues = select1->getValues();
  CPPUNIT_ASSERT_EQUAL((size_t)3, selectValues.size());
  CPPUNIT_ASSERT_EQUAL(string("alpha"), selectValues[0]);
  CPPUNIT_ASSERT_EQUAL(string("bravo"), selectValues[1]);
  CPPUNIT_ASSERT_EQUAL(string("charlie"), selectValues[2]);

  PStringSegmentHandle segment2 = select1->getNext();
  CPPUNIT_ASSERT(!segment2.isNull());
  CPPUNIT_ASSERT_EQUAL(string("/tango"), segment2->getValue());
}

void ParameterizedStringParserTest::testParse_loop()
{
  PStringDatumHandle ls = ParameterizedStringParser().parse("[1-10:2]");

  PStringNumLoopHandle loop1 = ls;
  CPPUNIT_ASSERT(!loop1.isNull());
  CPPUNIT_ASSERT_EQUAL((int32_t)1, loop1->getStartValue());
  CPPUNIT_ASSERT_EQUAL((int32_t)10, loop1->getEndValue());
  CPPUNIT_ASSERT_EQUAL((int32_t)2, loop1->getStep());
}

void ParameterizedStringParserTest::testParse_loop_empty()
{
  try {
    PStringDatumHandle ls = ParameterizedStringParser().parse("[]");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(FatalException* e) {
    cerr << e->getMsg() << endl;
    delete e;
  } catch(...) {
    CPPUNIT_FAIL("unexpected exception was thrown.");
  }
}

void ParameterizedStringParserTest::testParse_loop_missingParen()
{
  try {
    PStringDatumHandle ls = ParameterizedStringParser().parse("[");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(FatalException* e) {
    cerr << e->getMsg() << endl;
    delete e;
  } catch(...) {
    CPPUNIT_FAIL("unexpected exception was thrown.");
  }
}

void ParameterizedStringParserTest::testParse_loop_missingStep()
{
  try {
    PStringDatumHandle ls = ParameterizedStringParser().parse("[1-10:]");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(FatalException* e) {
    cerr << e->getMsg() << endl;
    delete e;
  } catch(...) {
    CPPUNIT_FAIL("unexpected exception was thrown.");
  }
}

void ParameterizedStringParserTest::testParse_loop_missingRange()
{
  try {
    PStringDatumHandle ls = ParameterizedStringParser().parse("[1-]");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(FatalException* e) {
    cerr << e->getMsg() << endl;
    delete e;
  } catch(...) {
    CPPUNIT_FAIL("unexpected exception was thrown.");
  }
}

void ParameterizedStringParserTest::testParse_alphaLoop()
{
  PStringDatumHandle ls = ParameterizedStringParser().parse("[a-z:2]");

  PStringNumLoopHandle loop1 = ls;
  CPPUNIT_ASSERT(!loop1.isNull());
  CPPUNIT_ASSERT_EQUAL((int32_t)0, loop1->getStartValue());
  CPPUNIT_ASSERT_EQUAL((int32_t)25, loop1->getEndValue());
  CPPUNIT_ASSERT_EQUAL((int32_t)2, loop1->getStep());
}

void ParameterizedStringParserTest::testParse_loop_mixedChar()
{
  try {
    ParameterizedStringParser().parse("[1-z:2]");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(FatalException* e) {
    cerr << e->getMsg() << endl;
    delete e;
  } catch(...) {
    CPPUNIT_FAIL("FatalException must be thrown.");
  }
}

void ParameterizedStringParserTest::testParse_loop_mixedCase()
{
  try {
    ParameterizedStringParser().parse("[a-Z:2]");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(FatalException* e) {
    cerr << e->getMsg() << endl;
    delete e;
  } catch(...) {
    CPPUNIT_FAIL("FatalException must be thrown.");
  }
}

void ParameterizedStringParserTest::testParse_segment_loop()
{
  PStringDatumHandle ls = ParameterizedStringParser().parse("http://server[1-3]/file");

  PStringSegmentHandle segment1 = ls;
  CPPUNIT_ASSERT(!segment1.isNull());
  CPPUNIT_ASSERT_EQUAL(string("http://server"), segment1->getValue());

  PStringNumLoopHandle loop1 = segment1->getNext();
  CPPUNIT_ASSERT(!loop1.isNull());
  CPPUNIT_ASSERT_EQUAL((int32_t)1, loop1->getStartValue());
  CPPUNIT_ASSERT_EQUAL((int32_t)3, loop1->getEndValue());
  CPPUNIT_ASSERT_EQUAL((int32_t)1, loop1->getStep());

  PStringSegmentHandle segment2 = loop1->getNext();
  CPPUNIT_ASSERT(!segment2.isNull());
  CPPUNIT_ASSERT_EQUAL(string("/file"), segment2->getValue());
}
