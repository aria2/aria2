// //////////////////////////////////////////////////////////////////////////
// Implementation file TestRunnerFailureEvent.cpp for class TestRunnerFailureEvent
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/22
// //////////////////////////////////////////////////////////////////////////

#include "TestRunnerFailureEvent.h"
#include "TestRunnerModelThreadInterface.h"


TestRunnerFailureEvent::TestRunnerFailureEvent( TestFailureInfo *failure,
                                                int numberOfFailure ) :
    _failure( failure ),
    _numberOfFailure( numberOfFailure )
{
}


TestRunnerFailureEvent::~TestRunnerFailureEvent()
{
}


void 
TestRunnerFailureEvent::process( TestRunnerModelThreadInterface *target )
{
  target->eventNewFailure( _failure, _numberOfFailure );
}
