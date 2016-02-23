#ifndef TESTCALLERTEST_H
#define TESTCALLERTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestSuite.h>
#include "MockTestListener.h"
#include "TrackedTestCase.h"

class TestCallerTest : public CPPUNIT_NS::TestFixture, 
                              Tracker
{
  CPPUNIT_TEST_SUITE( TestCallerTest );
  CPPUNIT_TEST( testBasicConstructor );
  CPPUNIT_TEST( testReferenceConstructor );
  CPPUNIT_TEST( testPointerConstructor );
//  CPPUNIT_TEST( testExpectFailureException );
//  CPPUNIT_TEST( testExpectException );
//  CPPUNIT_TEST( testExpectedExceptionNotCaught );
  CPPUNIT_TEST_SUITE_END();
public:
  TestCallerTest();
  virtual ~TestCallerTest();

  void setUp();
  void tearDown();

  void testBasicConstructor();
  void testReferenceConstructor();
  void testPointerConstructor();

//  void testExpectFailureException();
//  void testExpectException();
//  void testExpectedExceptionNotCaught();

private:
  class ExceptionThrower : public CPPUNIT_NS::TestCase
  {
  public:
    void testThrowFailureException();
    void testThrowException();
    void testThrowNothing();
  };

  virtual void onConstructor();
  virtual void onDestructor();
  virtual void onSetUp();
  virtual void onTearDown();
  virtual void onTest();

  void checkNothingButConstructorCalled();
  void checkRunningSequenceCalled();
  void checkTestName( std::string testName );

  TestCallerTest( const TestCallerTest &copy );
  void operator =( const TestCallerTest &copy );

private:
  int m_constructorCount;
  int m_destructorCount;
  int m_setUpCount;
  int m_tearDownCount;
  int m_testCount;
  const std::string m_testName;
  MockTestListener *m_testListener;
  CPPUNIT_NS::TestResult *m_result;
};



// Inlines methods for TestCallerTest:
// -----------------------------------



#endif  // TESTCALLERTEST_H
