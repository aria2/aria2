// //////////////////////////////////////////////////////////////////////////
// Header file TestPlugInRunnerModel.h for class TestPlugInRunnerModel
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/06/24
// //////////////////////////////////////////////////////////////////////////
#ifndef TESTPLUGINRUNNERMODEL_H
#define TESTPLUGINRUNNERMODEL_H

#include <TestRunnerModel.h>
class TestPlugIn;


/*! \class TestPlugInRunnerModel
 * \brief This class represents a model for the plug in runner.
 */
class TestPlugInRunnerModel : public TestRunnerModel
{
public:
  /*! Constructs a TestPlugInRunnerModel object.
   */
  TestPlugInRunnerModel();

  /*! Destructor.
   */
  virtual ~TestPlugInRunnerModel();

  void setPlugIn( TestPlugIn *plugIn );

  void reloadPlugIn();

public: // overridden from TestRunnerModel
  void setRootTest( CPPUNIT_NS::Test *rootTest );

private:
  /// Prevents the use of the copy constructor.
  TestPlugInRunnerModel( const TestPlugInRunnerModel &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestPlugInRunnerModel &copy );

  void freeRootTest();

private:
  TestPlugIn *m_plugIn;
};



// Inlines methods for TestPlugInRunnerModel:
// ------------------------------------------



#endif  // TESTPLUGINRUNNERMODEL_H
