// //////////////////////////////////////////////////////////////////////////
// Header file TestRunnerTestCaseRunEvent.h for class TestRunnerTestCaseRunEvent
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/22
// //////////////////////////////////////////////////////////////////////////
#ifndef TESTRUNNERTESTCASERUNEVENT_H
#define TESTRUNNERTESTCASERUNEVENT_H

#include "TestRunnerThreadEvent.h"


/*! \class TestRunnerTestCaseRunEvent
 * \brief This class represents a new TestCase run event.
 */
class TestRunnerTestCaseRunEvent : public TestRunnerThreadEvent
{
public:
  /*! Constructs a TestRunnerTestCaseRunEvent object.
   */
  TestRunnerTestCaseRunEvent( int numberOfRun );

  /// Destructor.
  virtual ~TestRunnerTestCaseRunEvent();

private:
  /// Prevents the use of the copy constructor.
  TestRunnerTestCaseRunEvent( const TestRunnerTestCaseRunEvent &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestRunnerTestCaseRunEvent &copy );

  void process( TestRunnerModelThreadInterface *target );

private:
  int _numberOfRun;
};



// Inlines methods for TestRunnerTestCaseRunEvent:
// -----------------------------------------------



#endif  // TESTRUNNERTESTCASERUNEVENT_H
