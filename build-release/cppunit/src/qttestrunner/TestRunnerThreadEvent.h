// //////////////////////////////////////////////////////////////////////////
// Header file TestRunnerThreadEvent.h for class TestRunnerThreadEvent
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/21
// //////////////////////////////////////////////////////////////////////////
#ifndef TESTRUNNERTHREADEVENT_H
#define TESTRUNNERTHREADEVENT_H

#include <qevent.h>
class TestRunnerModelThreadInterface;

/*! \class TestRunnerThreadEvent
 * \brief This class represents an event send by the test runner thread.
 */
class TestRunnerThreadEvent : public QCustomEvent
{
public:
  /*! Constructs a TestRunnerThreadEvent object.
   */
  TestRunnerThreadEvent();

  /// Destructor.
  virtual ~TestRunnerThreadEvent();

  virtual void process( TestRunnerModelThreadInterface *target ) =0;

private:
  /// Prevents the use of the copy constructor.
  TestRunnerThreadEvent( const TestRunnerThreadEvent &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestRunnerThreadEvent &copy );
};



#endif  // TESTRUNNERTHREADEVENT_H
