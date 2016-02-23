#ifndef TESTCOLLECTORRESULTTEST_H
#define TESTCOLLECTORRESULTTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFailure.h>
#include "SynchronizedTestResult.h"


class TestResultCollectorTest : public CPPUNIT_NS::TestFixture,
                       public SynchronizedTestResult::SynchronizationObjectListener
{
  CPPUNIT_TEST_SUITE( TestResultCollectorTest );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testAddTwoErrors );
  CPPUNIT_TEST( testAddTwoFailures );
  CPPUNIT_TEST( testStartTest );
  CPPUNIT_TEST( testWasSuccessfulWithErrors );
  CPPUNIT_TEST( testWasSuccessfulWithFailures );
  CPPUNIT_TEST( testWasSuccessfulWithErrorsAndFailures );
  CPPUNIT_TEST( testWasSuccessfulWithSuccessfulTest );
  CPPUNIT_TEST( testSynchronizationAddFailure );
  CPPUNIT_TEST( testSynchronizationStartTest );
  CPPUNIT_TEST( testSynchronizationRunTests );
  CPPUNIT_TEST( testSynchronizationTestErrors );
  CPPUNIT_TEST( testSynchronizationTestFailures );
  CPPUNIT_TEST( testSynchronizationFailures );
  CPPUNIT_TEST( testSynchronizationWasSuccessful );
  CPPUNIT_TEST_SUITE_END();

public:
  TestResultCollectorTest();
  virtual ~TestResultCollectorTest();

  virtual void setUp();
  virtual void tearDown();

  void testConstructor();

  void testAddTwoErrors();
  void testAddTwoFailures();
  void testStartTest();

  void testWasSuccessfulWithNoTest();
  void testWasSuccessfulWithErrors();
  void testWasSuccessfulWithFailures();
  void testWasSuccessfulWithErrorsAndFailures();
  void testWasSuccessfulWithSuccessfulTest();

  void testSynchronizationAddFailure();
  void testSynchronizationStartTest();
  void testSynchronizationRunTests();
  void testSynchronizationTestErrors();
  void testSynchronizationTestFailures();
  void testSynchronizationErrors();
  void testSynchronizationFailures();
  void testSynchronizationWasSuccessful();

  virtual void locked();
  virtual void unlocked();

private:
  TestResultCollectorTest( const TestResultCollectorTest &copy );
  void operator =( const TestResultCollectorTest &copy );

  void checkResult( int failures,
                    int errors,
                    int testsRun );

  void checkFailure( CPPUNIT_NS::TestFailure *failure,
                     CPPUNIT_NS::Message expectedMessage,
                     CPPUNIT_NS::Test *expectedTest,
                     bool expectedIsError );

  void checkWasSuccessful( bool shouldBeSuccessful );

  void checkSynchronization();

  void addFailure( std::string message );
  void addError( std::string message );
  void addFailure( std::string message, 
                   CPPUNIT_NS::Test *failedTest, 
                   bool isError,
                   CPPUNIT_NS::TestResultCollector *result );

private:
  CPPUNIT_NS::TestResultCollector *m_result;
  SynchronizedTestResult *m_synchronizedResult;  
  CPPUNIT_NS::Test *m_test;
  CPPUNIT_NS::Test *m_test2;
  int m_lockCount;
  int m_unlockCount;
};



#endif  // TESTCOLLECTORRESULTTEST_H
