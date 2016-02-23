// //////////////////////////////////////////////////////////////////////////
// Header file TestRunnerThreadFinishedEvent.h for class TestRunnerThreadFinishedEvent
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/22
// //////////////////////////////////////////////////////////////////////////
#ifndef TESTRUNNERTHREADFINISHEDEVENT_H
#define TESTRUNNERTHREADFINISHEDEVENT_H

#include "TestRunnerThreadEvent.h"


/*! \class TestRunnerThreadFinishedEvent
 * \brief This class represents an event indicating that the TestRunnerThread finished.
 */
class TestRunnerThreadFinishedEvent : public TestRunnerThreadEvent
{
public:
  /*! Constructs a TestRunnerThreadFinishedEvent object.
   */
  TestRunnerThreadFinishedEvent();

  /// Destructor.
  virtual ~TestRunnerThreadFinishedEvent();

  void process( TestRunnerModelThreadInterface *target );

private:
  /// Prevents the use of the copy constructor.
  TestRunnerThreadFinishedEvent( const TestRunnerThreadFinishedEvent &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestRunnerThreadFinishedEvent &copy );
};



// Inlines methods for TestRunnerThreadFinishedEvent:
// --------------------------------------------------



#endif  // TESTRUNNERTHREADFINISHEDEVENT_H
