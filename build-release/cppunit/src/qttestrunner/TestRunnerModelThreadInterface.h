// //////////////////////////////////////////////////////////////////////////
// Header file TestRunnerModelThreadInterface.h for class TestRunnerModelThreadInterface
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/21
// //////////////////////////////////////////////////////////////////////////
#ifndef TESTRUNNERMODELTHREADINTERFACE_H
#define TESTRUNNERMODELTHREADINTERFACE_H

class TestFailureInfo;

/*! \class TestRunnerModelThreadInterface
 * \brief This class represents the interface used to process gui thread event.
 */
class TestRunnerModelThreadInterface
{
public:
  /// Destructor.
  virtual ~TestRunnerModelThreadInterface() {}

  virtual void eventNewFailure( TestFailureInfo *failure,
                                int numberOfFailure ) =0;

  virtual void eventNumberOfTestRunChanged( int numberOfRun ) =0;

  virtual void eventTestRunnerThreadFinished() =0;
};



// Inlines methods for TestRunnerModelThreadInterface:
// ---------------------------------------------------



#endif  // TESTRUNNERMODELTHREADINTERFACE_H
