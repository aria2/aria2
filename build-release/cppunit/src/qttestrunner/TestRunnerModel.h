// //////////////////////////////////////////////////////////////////////////
// Header file TestRunnerModel.h for class TestRunnerModel
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/20
// //////////////////////////////////////////////////////////////////////////
#ifndef TESTRUNNERMODEL_H
#define TESTRUNNERMODEL_H

#include <cppunit/Test.h>
#include <cppunit/TestListener.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qthread.h>
#include "TestFailureInfo.h"
#include "TestRunnerModelThreadInterface.h"
class TestRunnerThread;

/*! \class TestRunnerModel
 * \brief This class represents the model for the TestRunner.
 *
 * Warning: methods that override CppUnit::TestListener are called
 * from the TestRunner thread !
 *
 * Warning: _lock is not recursive. Might want to introduce Doug Lea
 * Thread Interface pattern for methods used while locked (isTestRunning()).
 *
 * Refactoring note: a large part of this object actually duplicate
 * TestResult. 
 */
class TestRunnerModel : public QObject,
                        private CPPUNIT_NS::TestListener,
                        private TestRunnerModelThreadInterface
{
  Q_OBJECT
public:
  /*! Constructs a TestRunnerModel object.
   */
  TestRunnerModel( CPPUNIT_NS::Test *rootTest );

  /*! Destructor.
   */
  virtual ~TestRunnerModel();

  CPPUNIT_NS::Test *rootTest();

  int numberOfTestCase();
  int numberOfTestCaseFailure();
  int numberOfTestCaseRun();

  TestFailureInfo *failureAt( int index );


  bool isTestRunning();

signals:
  void numberOfTestCaseChanged( int numberOfTestCase );
  void numberOfTestCaseRunChanged( int numberOfRun );
  void numberOfTestCaseFailureChanged( int numberOfFailure );
  void failureAdded( TestFailureInfo *failure );
  void failuresCleared();
  void testRunStarted( CPPUNIT_NS::Test *runningTest,
                       CPPUNIT_NS::TestResult *result );
  void testRunFinished();

public slots:
  void resetTestReportCounterFor( CPPUNIT_NS::Test *testToRun );

  /*! Request to run the specified test.
   * Returns immedialty. If a test is already running, then
   * the run request is ignored.
   */
  void runTest( CPPUNIT_NS::Test *testToRun );

  /*! Request to stop running test.
   * This methods returns immediately. testRunFinished() signal
   * should be used to now when the test actually stopped running.
   */
  void stopRunningTest();

private:
  /// Prevents the use of the copy constructor.
  TestRunnerModel( const TestRunnerModel &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestRunnerModel &copy );

  /// Called from the TestRunnerThread.
  void startTest( CPPUNIT_NS::Test *test );

  /// Called from the TestRunnerThread.
  void addFailure( const CPPUNIT_NS::TestFailure &failure );
  
  /// Called from the TestRunnerThread.
  void endTest( CPPUNIT_NS::Test *test );

  /// Called from the TestRunnerThread.
  void addFailureInfo( TestFailureInfo *failure );

  bool event( QEvent *event );

  /*! Emits new failure signals.
   * Called by the TestRunnerThreadEvent from the GUI thread to
   * emit the following signals:
   * - numberOfTestCaseFailureChanged()
   * - failureAdded()
   */
  void eventNewFailure( TestFailureInfo *failure,
                        int numberOfFailure );

  /*! Emits numberOfTestCaseRunChanged() signal.
   * Called by the TestRunnerThreadEvent from the GUI thread to
   * emit the numberOfTestCaseRunChanged() signal.
   */
  void eventNumberOfTestRunChanged( int numberOfRun );

  void eventTestRunnerThreadFinished();

private:
  class LockGuard
  {
  public:
    LockGuard( QMutex &mutex ) : _mutex( mutex )
    {
      _mutex.lock();
    }

    ~LockGuard()
    {
      _mutex.unlock();
    }

  private:
    QMutex &_mutex;
  };


  QMutex _lock;
  CPPUNIT_NS::Test *_rootTest;
  int _numberOfTestCase;
  int _numberOfTestCaseRun;
  int _numberOfTestCaseFailure;
  QList<TestFailureInfo> _failures;
  TestRunnerThread *_runnerThread;
  CPPUNIT_NS::TestResult *_result;
};



// Inlines methods for TestRunnerModel:
// ------------------------------------



#endif  // TESTRUNNERMODEL_H
