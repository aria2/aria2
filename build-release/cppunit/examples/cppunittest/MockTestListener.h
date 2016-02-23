#ifndef MOCKTESTLISTENER_H
#define MOCKTESTLISTENER_H

#include <cppunit/TestListener.h>
#include <string>


class MockTestListener : public CPPUNIT_NS::TestListener
{
public:
  MockTestListener( std::string name );
  virtual ~MockTestListener() {}

  void setExpectFailure( CPPUNIT_NS::Test *failedTest,
                         CPPUNIT_NS::Exception *thrownException,
                         bool isError );
  void setExpectNoFailure();
  void setExpectFailure();
  void setExpectedAddFailureCall( int callCount );
  void setExpectStartTest( CPPUNIT_NS::Test *test );
  void setExpectedStartTestCall( int callCount );
  void setExpectEndTest( CPPUNIT_NS::Test *test );
  void setExpectedEndTestCall( int callCount );
  void setExpectStartSuite( CPPUNIT_NS::Test *suite );
  void setExpectedStartSuiteCall( int callCount );
  void setExpectEndSuite( CPPUNIT_NS::Test *suite );
  void setExpectedEndSuiteCall( int callCount );
  void setExpectStartTestRun( CPPUNIT_NS::Test *test,
                              CPPUNIT_NS::TestResult *eventManager );
  void setExpectedStartTestRunCall( int callCount );
  void setExpectEndTestRun( CPPUNIT_NS::Test *test,
                            CPPUNIT_NS::TestResult *eventManager );
  void setExpectedEndTestRunCall( int callCount );

  void addFailure( const CPPUNIT_NS::TestFailure &failure );
  void startTest( CPPUNIT_NS::Test *test );
  void endTest( CPPUNIT_NS::Test *test );
  void startSuite( CPPUNIT_NS::Test *suite );
  void endSuite( CPPUNIT_NS::Test *suite );
  void startTestRun( CPPUNIT_NS::Test *test, 
                     CPPUNIT_NS::TestResult *eventManager );
  void endTestRun( CPPUNIT_NS::Test *test, 
                   CPPUNIT_NS::TestResult *eventManager );

  void verify();

private:
  std::string m_name;

  bool m_hasExpectationForStartTest;
  bool m_hasParametersExpectationForStartTest;
  int m_expectedStartTestCallCount;
  int m_startTestCall;
  CPPUNIT_NS::Test *m_expectedStartTest;

  bool m_hasExpectationForEndTest;
  bool m_hasParametersExpectationForEndTest;
  int m_expectedEndTestCallCount;
  CPPUNIT_NS::Test *m_expectedEndTest;
  int m_endTestCall;

  bool m_hasExpectationForStartSuite;
  bool m_hasParametersExpectationForStartSuite;
  int m_expectedStartSuiteCallCount;
  CPPUNIT_NS::Test *m_expectedStartSuite;
  int m_startSuiteCall;

  bool m_hasExpectationForEndSuite;
  bool m_hasParametersExpectationForEndSuite;
  int m_expectedEndSuiteCallCount;
  CPPUNIT_NS::Test *m_expectedEndSuite;
  int m_endSuiteCall;

  bool m_hasExpectationForStartTestRun;
  bool m_hasParametersExpectationForStartTestRun;
  int m_expectedStartTestRunCallCount;
  CPPUNIT_NS::Test *m_expectedStartTestRun;
  CPPUNIT_NS::TestResult *m_expectedStartTestRun2;
  int m_startTestRunCall;

  bool m_hasExpectationForEndTestRun;
  bool m_hasParametersExpectationForEndTestRun;
  int m_expectedEndTestRunCallCount;
  CPPUNIT_NS::Test *m_expectedEndTestRun;
  CPPUNIT_NS::TestResult *m_expectedEndTestRun2;
  int m_endTestRunCall;

  bool m_hasExpectationForAddFailure;
  bool m_hasExpectationForSomeFailure;
  bool m_hasParametersExpectationForAddFailure;
  int m_expectedAddFailureCallCount;
  int m_addFailureCall;
  CPPUNIT_NS::Test *m_expectedFailedTest;
  CPPUNIT_NS::Exception *m_expectedException;
  bool m_expectedIsError;
};



// Inlines methods for MockTestListener:
// -------------------------------------



#endif  // MOCKTESTLISTENER_H
