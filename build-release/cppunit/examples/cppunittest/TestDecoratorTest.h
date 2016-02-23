#ifndef TESTDECORATORTEST_H
#define TESTDECORATORTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestDecorator.h>
#include "MockTestCase.h"


class TestDecoratorTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestDecoratorTest );
  CPPUNIT_TEST( testCountTestCases );
  CPPUNIT_TEST( testRun );
  CPPUNIT_TEST( testGetName );
  CPPUNIT_TEST_SUITE_END();

public:
  TestDecoratorTest();
  virtual ~TestDecoratorTest();

  virtual void setUp();
  virtual void tearDown();

  void testCountTestCases();
  void testRun();
  void testGetName();

private:
  TestDecoratorTest( const TestDecoratorTest &copy );
  void operator =( const TestDecoratorTest &copy );

private:
  MockTestCase *m_test;
  CPPUNIT_NS::TestDecorator *m_decorator;
};



#endif  // TESTDECORATORTEST_H
