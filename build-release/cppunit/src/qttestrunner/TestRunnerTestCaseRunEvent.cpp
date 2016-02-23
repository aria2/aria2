// //////////////////////////////////////////////////////////////////////////
// Implementation file TestRunnerTestCaseRunEvent.cpp for class TestRunnerTestCaseRunEvent
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/22
// //////////////////////////////////////////////////////////////////////////

#include "TestRunnerModelThreadInterface.h"
#include "TestRunnerTestCaseRunEvent.h"


TestRunnerTestCaseRunEvent::TestRunnerTestCaseRunEvent( int numberOfRun ) :
    _numberOfRun( numberOfRun )
{
}


TestRunnerTestCaseRunEvent::~TestRunnerTestCaseRunEvent()
{
}


void 
TestRunnerTestCaseRunEvent::process( TestRunnerModelThreadInterface *target )
{
  target->eventNumberOfTestRunChanged( _numberOfRun );
}
