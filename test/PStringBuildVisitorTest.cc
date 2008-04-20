#include "PStringBuildVisitor.h"
#include "PStringSegment.h"
#include "PStringNumLoop.h"
#include "PStringSelect.h"
#include "FixedWidthNumberDecorator.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

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
  SharedHandle<PStringSegment> segment1(new PStringSegment("/tango"));

  const char* select1data[] = { "alpha", "bravo", "charlie" };
  
  SharedHandle<PStringSelect> select1
    (new PStringSelect(std::deque<std::string>(&select1data[0], &select1data[3]), segment1));

  PStringBuildVisitor v;

  select1->accept(&v);

  CPPUNIT_ASSERT_EQUAL((size_t)3, v.getURIs().size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha/tango"), v.getURIs()[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("bravo/tango"), v.getURIs()[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("charlie/tango"), v.getURIs()[2]);
}

void PStringBuildVisitorTest::testVisit_numLoop()
{
  SharedHandle<PStringSegment> segment1(new PStringSegment("/tango"));

  SharedHandle<NumberDecorator> decorator(new FixedWidthNumberDecorator(2));
  SharedHandle<PStringNumLoop> loop1
    (new PStringNumLoop(0, 5, 2, decorator, segment1));

  PStringBuildVisitor v;

  loop1->accept(&v);

  CPPUNIT_ASSERT_EQUAL((size_t)3, v.getURIs().size());
  CPPUNIT_ASSERT_EQUAL(std::string("00/tango"), v.getURIs()[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("02/tango"), v.getURIs()[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("04/tango"), v.getURIs()[2]);
}

void PStringBuildVisitorTest::testVisit_select_numLoop()
{
  SharedHandle<PStringSegment> segment1(new PStringSegment("/tango"));

  const char* select1data[] = { "alpha", "bravo", "charlie" };
  
  SharedHandle<PStringSelect> select1
    (new PStringSelect(std::deque<std::string>(&select1data[0], &select1data[3]), segment1));

  SharedHandle<NumberDecorator> decorator(new FixedWidthNumberDecorator(2));
  SharedHandle<PStringNumLoop> loop1
    (new PStringNumLoop(0, 5, 2, decorator, select1));

  PStringBuildVisitor v;

  loop1->accept(&v);

  CPPUNIT_ASSERT_EQUAL((size_t)9, v.getURIs().size());
  CPPUNIT_ASSERT_EQUAL(std::string("00alpha/tango"), v.getURIs()[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("00bravo/tango"), v.getURIs()[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("00charlie/tango"), v.getURIs()[2]);
  CPPUNIT_ASSERT_EQUAL(std::string("02alpha/tango"), v.getURIs()[3]);
  CPPUNIT_ASSERT_EQUAL(std::string("02bravo/tango"), v.getURIs()[4]);
  CPPUNIT_ASSERT_EQUAL(std::string("02charlie/tango"), v.getURIs()[5]);
  CPPUNIT_ASSERT_EQUAL(std::string("04alpha/tango"), v.getURIs()[6]);
  CPPUNIT_ASSERT_EQUAL(std::string("04bravo/tango"), v.getURIs()[7]);
  CPPUNIT_ASSERT_EQUAL(std::string("04charlie/tango"), v.getURIs()[8]);
}

} // namespace aria2
