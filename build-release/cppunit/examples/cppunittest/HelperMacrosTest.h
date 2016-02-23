#ifndef HELPERMACROSTEST_H
#define HELPERMACROSTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "MockTestListener.h"


class HelperMacrosTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( HelperMacrosTest );
  CPPUNIT_TEST( testNoSubclassing );
  CPPUNIT_TEST( testSubclassing );
  CPPUNIT_TEST( testFail );
  CPPUNIT_TEST( testFailToFail );
  CPPUNIT_TEST( testException );
  CPPUNIT_TEST( testExceptionNotCaught );
  CPPUNIT_TEST( testCustomTests );
  CPPUNIT_TEST( testAddTest );
  CPPUNIT_TEST_SUITE_END();

public:
  HelperMacrosTest();
  virtual ~HelperMacrosTest();

  virtual void setUp();
  virtual void tearDown();

  void testNoSubclassing();

  void testSubclassing();

  void testFail();
  void testFailToFail();

  void testException();
  void testExceptionNotCaught();

  void testCustomTest();
  void testCustomTests();
  void testAddTest();

private:
  HelperMacrosTest( const HelperMacrosTest &copy );
  void operator =( const HelperMacrosTest &copy );

private:
  CPPUNIT_NS::TestResult *m_result;
  MockTestListener *m_testListener;
};



#endif  // HELPERMACROSTEST_H
