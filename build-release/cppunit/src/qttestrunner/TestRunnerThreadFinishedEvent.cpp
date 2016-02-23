// //////////////////////////////////////////////////////////////////////////
// Implementation file TestRunnerThreadFinishedEvent.cpp for class TestRunnerThreadFinishedEvent
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/22
// //////////////////////////////////////////////////////////////////////////

#include "TestRunnerThreadFinishedEvent.h"
#include "TestRunnerModelThreadInterface.h"


TestRunnerThreadFinishedEvent::TestRunnerThreadFinishedEvent()
{
}


TestRunnerThreadFinishedEvent::~TestRunnerThreadFinishedEvent()
{
}


void 
TestRunnerThreadFinishedEvent::process( TestRunnerModelThreadInterface *target )
{
  target->eventTestRunnerThreadFinished();
}
