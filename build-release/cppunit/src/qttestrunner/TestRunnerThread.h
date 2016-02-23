// //////////////////////////////////////////////////////////////////////////
// Header file TestRunnerThread.h for class TestRunnerThread
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/22
// //////////////////////////////////////////////////////////////////////////
#ifndef TESTRUNNERTHREAD_H
#define TESTRUNNERTHREAD_H

#include <qthread.h>
#include <cppunit/Test.h>
#include <cppunit/TestResult.h>
class QObject;
class TestRunnerThreadFinishedEvent;


/*! \class TestRunnerThread
 * \brief This class represents the thread used to run TestCase.
 */
class TestRunnerThread : public QThread
{
public:
  /*! Constructs a TestRunnerThread object.
   */
  TestRunnerThread( CPPUNIT_NS::Test *testToRun,
                    CPPUNIT_NS::TestResult *result,
                    QObject *eventTarget,
                    TestRunnerThreadFinishedEvent *finishedEvent );

  /// Destructor.
  virtual ~TestRunnerThread();

private:
  /// Prevents the use of the copy constructor.
  TestRunnerThread( const TestRunnerThread &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestRunnerThread &copy );

  void run();

private:
 CPPUNIT_NS::Test *_testToRun;
 CPPUNIT_NS::TestResult *_result;
 QObject *_eventTarget;
 TestRunnerThreadFinishedEvent *_finishedEvent;
};



// Inlines methods for TestRunnerThread:
// -------------------------------------



#endif  // TESTRUNNERTHREAD_H
