// //////////////////////////////////////////////////////////////////////////
// Header file TestRunnerFailureEvent.h for class TestRunnerFailureEvent
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/22
// //////////////////////////////////////////////////////////////////////////
#ifndef TESTRUNNERFAILUREEVENT_H
#define TESTRUNNERFAILUREEVENT_H

#include "TestRunnerThreadEvent.h"
class TestFailureInfo;


/*! \class TestRunnerFailureEvent
 * \brief This class represents a new TestCase failure event.
 */
class TestRunnerFailureEvent : public TestRunnerThreadEvent
{
public:
  /*! Constructs a TestRunnerFailureEvent object.
   */
  TestRunnerFailureEvent( TestFailureInfo *failure,
                          int numberOfFailure );

  /// Destructor.
  virtual ~TestRunnerFailureEvent();

private:
  /// Prevents the use of the copy constructor.
  TestRunnerFailureEvent( const TestRunnerFailureEvent &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestRunnerFailureEvent &copy );

  void process( TestRunnerModelThreadInterface *target );

private:
  TestFailureInfo *_failure;
  int _numberOfFailure;
};



// Inlines methods for TestRunnerFailureEvent:
// -------------------------------------------



#endif  // TESTRUNNERFAILUREEVENT_H
