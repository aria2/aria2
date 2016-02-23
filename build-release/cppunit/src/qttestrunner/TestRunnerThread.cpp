// //////////////////////////////////////////////////////////////////////////
// Implementation file TestRunnerThread.cpp for class TestRunnerThread
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/22
// //////////////////////////////////////////////////////////////////////////

#include "TestRunnerThread.h"
#include "TestRunnerThreadFinishedEvent.h"


TestRunnerThread::TestRunnerThread( CPPUNIT_NS::Test *testToRun,
                                    CPPUNIT_NS::TestResult *result,
                                    QObject *eventTarget,
                              TestRunnerThreadFinishedEvent *finishedEvent ) :
    _testToRun( testToRun ),
    _result( result ),
    _eventTarget( eventTarget ),
    _finishedEvent( finishedEvent )
{
  start();
}


TestRunnerThread::~TestRunnerThread()
{
}


void 
TestRunnerThread::run()
{
  _testToRun->run( _result );

  // Signal TestRunnerModel GUI thread
  QThread::postEvent( _eventTarget, _finishedEvent );
  _eventTarget = NULL;
  _finishedEvent = NULL;
}
