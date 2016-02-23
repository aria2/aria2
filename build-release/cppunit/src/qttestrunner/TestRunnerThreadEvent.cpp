// //////////////////////////////////////////////////////////////////////////
// Implementation file TestRunnerThreadEvent.cpp for class TestRunnerThreadEvent
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/21
// //////////////////////////////////////////////////////////////////////////

#include "TestRunnerThreadEvent.h"


TestRunnerThreadEvent::TestRunnerThreadEvent() : 
    QCustomEvent( User )
{
}


TestRunnerThreadEvent::~TestRunnerThreadEvent()
{
}

