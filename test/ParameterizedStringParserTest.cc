#include "ParameterizedStringParser.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "PStringSelect.h"
#include "PStringSegment.h"
#include "PStringNumLoop.h"
#include "DlAbortEx.h"

namespace aria2 {

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
  SharedHandle<PStringDatum> ls = ParameterizedStringParser().parse("{alpha, bravo, charlie}");
  SharedHandle<PStringSelect> select(dynamic_pointer_cast<PStringSelect>(ls));
  CPPUNIT_ASSERT(select);

  std::vector<std::string> values = select->getValues();
  CPPUNIT_ASSERT_EQUAL((size_t)3, values.size());

  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), values[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), values[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("charlie"), values[2]);
}

void ParameterizedStringParserTest::testParse_select_empty()
{
  try {
    SharedHandle<PStringDatum> ls = ParameterizedStringParser().parse("{}");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    std::cerr << e.stackTrace() << std::endl;
  } catch(...) {
    CPPUNIT_FAIL("unexpected exception thrown.");
  }
}

void ParameterizedStringParserTest::testParse_select_missingParen()
{
  try {
    SharedHandle<PStringDatum> ls = ParameterizedStringParser().parse("{alpha");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    std::cerr << e.stackTrace() << std::endl;
  } catch(...) {
    CPPUNIT_FAIL("unexpected exception was thrown.");
  }
}

void ParameterizedStringParserTest::testParse_segment()
{
  SharedHandle<PStringDatum> ls = ParameterizedStringParser().parse("hello world");
  SharedHandle<PStringSegment> segment(dynamic_pointer_cast<PStringSegment>(ls));
  CPPUNIT_ASSERT(segment);
  CPPUNIT_ASSERT_EQUAL(std::string("hello world"), segment->getValue());
}

void ParameterizedStringParserTest::testParse_segment_select()
{
  SharedHandle<PStringDatum> ls = ParameterizedStringParser().parse("file:///{alpha, bravo, charlie}/tango");

  SharedHandle<PStringSegment> segment1
    (dynamic_pointer_cast<PStringSegment>(ls));
  CPPUNIT_ASSERT(segment1);
  CPPUNIT_ASSERT_EQUAL(std::string("file:///"), segment1->getValue());

  SharedHandle<PStringSelect> select1
    (dynamic_pointer_cast<PStringSelect>(segment1->getNext()));
  CPPUNIT_ASSERT(select1);
  std::vector<std::string> selectValues = select1->getValues();
  CPPUNIT_ASSERT_EQUAL((size_t)3, selectValues.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), selectValues[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), selectValues[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("charlie"), selectValues[2]);
  
  SharedHandle<PStringSegment> segment2
    (dynamic_pointer_cast<PStringSegment>(select1->getNext()));
  CPPUNIT_ASSERT(segment2);
  CPPUNIT_ASSERT_EQUAL(std::string("/tango"), segment2->getValue());
}

void ParameterizedStringParserTest::testParse_loop()
{
  SharedHandle<PStringDatum> ls = ParameterizedStringParser().parse("[1-10:2]");

  SharedHandle<PStringNumLoop> loop1(dynamic_pointer_cast<PStringNumLoop>(ls));
  CPPUNIT_ASSERT(loop1);
  CPPUNIT_ASSERT_EQUAL(1U, loop1->getStartValue());
  CPPUNIT_ASSERT_EQUAL(10U, loop1->getEndValue());
  CPPUNIT_ASSERT_EQUAL(2U, loop1->getStep());
}

void ParameterizedStringParserTest::testParse_loop_empty()
{
  try {
    SharedHandle<PStringDatum> ls = ParameterizedStringParser().parse("[]");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    std::cerr << e.stackTrace() << std::endl;
  } catch(...) {
    CPPUNIT_FAIL("unexpected exception was thrown.");
  }
}

void ParameterizedStringParserTest::testParse_loop_missingParen()
{
  try {
    SharedHandle<PStringDatum> ls = ParameterizedStringParser().parse("[");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    std::cerr << e.stackTrace() << std::endl;
  } catch(...) {
    CPPUNIT_FAIL("unexpected exception was thrown.");
  }
}

void ParameterizedStringParserTest::testParse_loop_missingStep()
{
  try {
    SharedHandle<PStringDatum> ls = ParameterizedStringParser().parse("[1-10:]");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    std::cerr << e.stackTrace() << std::endl;
  } catch(...) {
    CPPUNIT_FAIL("unexpected exception was thrown.");
  }
}

void ParameterizedStringParserTest::testParse_loop_missingRange()
{
  try {
    SharedHandle<PStringDatum> ls = ParameterizedStringParser().parse("[1-]");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    std::cerr << e.stackTrace() << std::endl;
  } catch(...) {
    CPPUNIT_FAIL("unexpected exception was thrown.");
  }
}

void ParameterizedStringParserTest::testParse_alphaLoop()
{
  SharedHandle<PStringDatum> ls = ParameterizedStringParser().parse("[a-z:2]");

  SharedHandle<PStringNumLoop> loop1(dynamic_pointer_cast<PStringNumLoop>(ls));
  CPPUNIT_ASSERT(loop1);
  CPPUNIT_ASSERT_EQUAL(0U, loop1->getStartValue());
  CPPUNIT_ASSERT_EQUAL(25U, loop1->getEndValue());
  CPPUNIT_ASSERT_EQUAL(2U, loop1->getStep());
}

void ParameterizedStringParserTest::testParse_loop_mixedChar()
{
  try {
    ParameterizedStringParser().parse("[1-z:2]");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    std::cerr << e.stackTrace() << std::endl;
  } catch(...) {
    CPPUNIT_FAIL("DlAbortEx must be thrown.");
  }
}

void ParameterizedStringParserTest::testParse_loop_mixedCase()
{
  try {
    ParameterizedStringParser().parse("[a-Z:2]");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    std::cerr << e.stackTrace() << std::endl;
  } catch(...) {
    CPPUNIT_FAIL("DlAbortEx must be thrown.");
  }
}

void ParameterizedStringParserTest::testParse_segment_loop()
{
  SharedHandle<PStringDatum> ls = ParameterizedStringParser().parse("http://server[1-3]/file");

  SharedHandle<PStringSegment> segment1(dynamic_pointer_cast<PStringSegment>(ls));
  CPPUNIT_ASSERT(segment1);
  CPPUNIT_ASSERT_EQUAL(std::string("http://server"), segment1->getValue());

  SharedHandle<PStringNumLoop> loop1(dynamic_pointer_cast<PStringNumLoop>(segment1->getNext()));
  CPPUNIT_ASSERT(loop1);
  CPPUNIT_ASSERT_EQUAL(1U, loop1->getStartValue());
  CPPUNIT_ASSERT_EQUAL(3U, loop1->getEndValue());
  CPPUNIT_ASSERT_EQUAL(1U, loop1->getStep());

  SharedHandle<PStringSegment> segment2(dynamic_pointer_cast<PStringSegment>(loop1->getNext()));
  CPPUNIT_ASSERT(segment2);
  CPPUNIT_ASSERT_EQUAL(std::string("/file"), segment2->getValue());
}

} // namespace aria2
