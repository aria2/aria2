// //////////////////////////////////////////////////////////////////////////
// Header file TestCaseTest.h for class TestCaseTest
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2000/06/09
// //////////////////////////////////////////////////////////////////////////
#ifndef TESTCASETEST_H
#define TESTCASETEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestResult.h>
#include "MockTestListener.h"
#include <stdexcept>


class TestCaseTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestCaseTest );
  CPPUNIT_TEST( testSetUpFailure );
  CPPUNIT_TEST( testRunTestFailure );
  CPPUNIT_TEST( testTearDownFailure );
  CPPUNIT_TEST( testFailAll );
  CPPUNIT_TEST( testNoFailure );
  CPPUNIT_TEST( testTwoRun );
  CPPUNIT_TEST( testCountTestCases );
  CPPUNIT_TEST( testDefaultConstructor );
  CPPUNIT_TEST( testConstructorWithName );
  CPPUNIT_TEST( testGetChildTestCount );
  CPPUNIT_TEST_EXCEPTION( testGetChildTestAtThrow, std::out_of_range );
  CPPUNIT_TEST_SUITE_END();

public:
  TestCaseTest();

  virtual ~TestCaseTest();

  void setUp();
  void tearDown();

  void testSetUpFailure();
  void testRunTestFailure();
  void testTearDownFailure();
  void testFailAll();
  void testNoFailure();
  void testTwoRun();

  void testCountTestCases();

  void testDefaultConstructor();
  void testConstructorWithName();

  void testGetChildTestCount();
  void testGetChildTestAtThrow();

private:
  TestCaseTest( const TestCaseTest &copy );
  void operator =( const TestCaseTest &copy );

  void checkFailure( bool failSetUp, 
                     bool failRunTest,
                     bool failTearDown );
/*
  void checkResult( int failures,
                    int errors,
                    int testsRun,
                    CPPUNIT_NS::TestResult *result );
*/
private:
  CPPUNIT_NS::TestResult *m_result;
  MockTestListener *m_testListener;
};


#endif  // TESTCASETEST_H
