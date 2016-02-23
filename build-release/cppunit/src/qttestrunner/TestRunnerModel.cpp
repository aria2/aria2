// //////////////////////////////////////////////////////////////////////////
// Implementation file TestRunnerModel.cpp for class TestRunnerModel
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/20
// //////////////////////////////////////////////////////////////////////////

#include <cppunit/TestResult.h>
#include "TestRunnerFailureEvent.h"
#include "TestRunnerModel.h"
#include "TestRunnerTestCaseRunEvent.h"
#include "TestRunnerThread.h"
#include "TestRunnerThreadFinishedEvent.h"


TestRunnerModel::TestRunnerModel( CPPUNIT_NS::Test *rootTest ) :
    _rootTest( rootTest ),
    _runnerThread( NULL ),
    _result( NULL )
{
}


TestRunnerModel::~TestRunnerModel()
{
  delete _runnerThread;
}


CPPUNIT_NS::Test *
TestRunnerModel::rootTest()
{
  return _rootTest;
}


void 
TestRunnerModel::resetTestReportCounterFor( CPPUNIT_NS::Test *testToRun )
{
  if ( isTestRunning() )
    return;

  {
    LockGuard guard( _lock );
    _numberOfTestCase = testToRun->countTestCases();
    _numberOfTestCaseRun =0;
    _numberOfTestCaseFailure =0;
    _failures.clear();
  }
  emit failuresCleared();

  emit numberOfTestCaseChanged( _numberOfTestCase );
  emit numberOfTestCaseRunChanged( _numberOfTestCaseRun );
  emit numberOfTestCaseFailureChanged( _numberOfTestCaseFailure );
}


int 
TestRunnerModel::numberOfTestCase()
{
  LockGuard guard( _lock );
  return _numberOfTestCase;
}


int 
TestRunnerModel::numberOfTestCaseFailure()
{
  LockGuard guard( _lock );
  return _numberOfTestCaseFailure;
}


int 
TestRunnerModel::numberOfTestCaseRun()
{
  LockGuard guard( _lock );
  return _numberOfTestCaseRun;
}


TestFailureInfo *
TestRunnerModel::failureAt( int index )
{
  LockGuard guard( _lock );
  return _failures.at( index );
}


void 
TestRunnerModel::runTest( CPPUNIT_NS::Test *testToRun )
{
  if ( isTestRunning() )
    return;

  resetTestReportCounterFor( testToRun );

  {
    LockGuard guard( _lock );
    delete _result;
    _result = new CPPUNIT_NS::TestResult();
    _result->addListener( this );
  }

  emit testRunStarted( testToRun, _result );

  LockGuard guard( _lock );
  _runnerThread = new TestRunnerThread( testToRun, 
                                        _result, 
                                        this, 
                                        new TestRunnerThreadFinishedEvent() );
}


bool 
TestRunnerModel::isTestRunning()
{
  LockGuard guard( _lock );
  return _runnerThread != NULL  &&  _runnerThread->running();
}


void 
TestRunnerModel::stopRunningTest()
{
  {
    LockGuard guard( _lock );
    if ( _result == NULL )
      return;
  }
  if ( isTestRunning() )
  {
    LockGuard guard( _lock );
    _result->stop();
  }
}


// Called from the TestRunnerThread.
void 
TestRunnerModel::startTest( CPPUNIT_NS::Test * /*test*/ )
{
}


// Called from the TestRunnerThread.
void 
TestRunnerModel::addFailure( const CPPUNIT_NS::TestFailure &failure )
{
  addFailureInfo( new TestFailureInfo( failure.failedTest(), 
                                       failure.thrownException(),
                                       failure.isError() ) );
}


// Called from the TestRunnerThread.
void 
TestRunnerModel::endTest( CPPUNIT_NS::Test * /*test*/ )
{
  int numberOfTestCaseRun;
  {
    LockGuard guard( _lock );
    numberOfTestCaseRun = ++_numberOfTestCaseRun;
  }

  // emit signal asynchronously
  QThread::postEvent( this, 
                      new TestRunnerTestCaseRunEvent( numberOfTestCaseRun ) );
}


// Called from the TestRunnerThread.
void 
TestRunnerModel::addFailureInfo( TestFailureInfo *failure )
{
  int numberOfTestCaseFailure;
  {
    LockGuard guard( _lock );
    _failures.append( failure );
    numberOfTestCaseFailure = ++_numberOfTestCaseFailure;
  }

  // emit signals asynchronously
  QThread::postEvent( this, 
                      new TestRunnerFailureEvent( failure,
                                                  numberOfTestCaseFailure ) );
}


bool 
TestRunnerModel::event( QEvent *event )
{
  if ( event->type() != QEvent::User )
    return false;

  TestRunnerThreadEvent *threadEvent = (TestRunnerThreadEvent *)event;
  threadEvent->process( this );
  return true;
}


void 
TestRunnerModel::eventNewFailure( TestFailureInfo *failure,
                                  int numberOfFailure )
{
  emit numberOfTestCaseFailureChanged( numberOfFailure );
  emit failureAdded( failure );
}


void 
TestRunnerModel::eventNumberOfTestRunChanged( int numberOfRun )
{
  emit numberOfTestCaseRunChanged( numberOfRun );
}


void 
TestRunnerModel::eventTestRunnerThreadFinished()
{
  emit testRunFinished();
}
