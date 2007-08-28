#include "PStringBuildVisitor.h"
#include "PStringSegment.h"
#include "PStringNumLoop.h"
#include "PStringSelect.h"
#include "FixedWidthNumberDecorator.h"
#include <cppunit/extensions/HelperMacros.h>

class PStringBuildVisitorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PStringBuildVisitorTest);
  CPPUNIT_TEST(testVisit_select);
  CPPUNIT_TEST(testVisit_numLoop);
  CPPUNIT_TEST(testVisit_select_numLoop);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testVisit_select();
  void testVisit_numLoop();
  void testVisit_select_numLoop();
};


CPPUNIT_TEST_SUITE_REGISTRATION( PStringBuildVisitorTest );

void PStringBuildVisitorTest::testVisit_select()
{
  PStringSegmentHandle segment1 = new PStringSegment("/tango");

  char* select1data[] = { "alpha", "bravo", "charlie" };
  
  PStringSelectHandle select1 =
    new PStringSelect(Strings(&select1data[0], &select1data[3]), segment1);

  PStringBuildVisitorHandle v = new PStringBuildVisitor();

  select1->accept(v);

  CPPUNIT_ASSERT_EQUAL((size_t)3, v->getURIs().size());
  CPPUNIT_ASSERT_EQUAL(string("alpha/tango"), v->getURIs()[0]);
  CPPUNIT_ASSERT_EQUAL(string("bravo/tango"), v->getURIs()[1]);
  CPPUNIT_ASSERT_EQUAL(string("charlie/tango"), v->getURIs()[2]);
}

void PStringBuildVisitorTest::testVisit_numLoop()
{
  PStringSegmentHandle segment1 = new PStringSegment("/tango");

  PStringNumLoopHandle loop1 =
    new PStringNumLoop(0, 5, 2, new FixedWidthNumberDecorator(2), segment1);

  PStringBuildVisitorHandle v = new PStringBuildVisitor();

  loop1->accept(v);

  CPPUNIT_ASSERT_EQUAL((size_t)3, v->getURIs().size());
  CPPUNIT_ASSERT_EQUAL(string("00/tango"), v->getURIs()[0]);
  CPPUNIT_ASSERT_EQUAL(string("02/tango"), v->getURIs()[1]);
  CPPUNIT_ASSERT_EQUAL(string("04/tango"), v->getURIs()[2]);
}

void PStringBuildVisitorTest::testVisit_select_numLoop()
{
  PStringSegmentHandle segment1 = new PStringSegment("/tango");

  char* select1data[] = { "alpha", "bravo", "charlie" };
  
  PStringSelectHandle select1 =
    new PStringSelect(Strings(&select1data[0], &select1data[3]), segment1);

  PStringNumLoopHandle loop1 =
    new PStringNumLoop(0, 5, 2, new FixedWidthNumberDecorator(2), select1);

  PStringBuildVisitorHandle v = new PStringBuildVisitor();

  loop1->accept(v);

  CPPUNIT_ASSERT_EQUAL((size_t)9, v->getURIs().size());
  CPPUNIT_ASSERT_EQUAL(string("00alpha/tango"), v->getURIs()[0]);
  CPPUNIT_ASSERT_EQUAL(string("00bravo/tango"), v->getURIs()[1]);
  CPPUNIT_ASSERT_EQUAL(string("00charlie/tango"), v->getURIs()[2]);
  CPPUNIT_ASSERT_EQUAL(string("02alpha/tango"), v->getURIs()[3]);
  CPPUNIT_ASSERT_EQUAL(string("02bravo/tango"), v->getURIs()[4]);
  CPPUNIT_ASSERT_EQUAL(string("02charlie/tango"), v->getURIs()[5]);
  CPPUNIT_ASSERT_EQUAL(string("04alpha/tango"), v->getURIs()[6]);
  CPPUNIT_ASSERT_EQUAL(string("04bravo/tango"), v->getURIs()[7]);
  CPPUNIT_ASSERT_EQUAL(string("04charlie/tango"), v->getURIs()[8]);
}
