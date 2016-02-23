// //////////////////////////////////////////////////////////////////////////
// Implementation file QtTestRunner.cpp for class QtTestRunner
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/19
// //////////////////////////////////////////////////////////////////////////

#include <qapplication.h>
#include <cppunit/TestSuite.h>
#include <cppunit/ui/qt/QtTestRunner.h>
#include "TestRunnerDlgImpl.h"
#include "TestRunnerModel.h"


CPPUNIT_NS_BEGIN

QtTestRunner::QtTestRunner() :
  _suite( new CPPUNIT_NS::TestSuite( "All Tests" ) ),
  _tests( new Tests() )
{
}


QtTestRunner::~QtTestRunner()
{
  delete _suite;

  Tests::iterator it = _tests->begin();
  while ( it != _tests->end() )
    delete *it++;

  delete _tests;
}


Test *
QtTestRunner::getRootTest()
{
  if ( _tests->size() != 1 )
  {
    Tests::iterator it = _tests->begin();
    while ( it != _tests->end() )
      _suite->addTest( *it++ );
    _tests->clear();
    return _suite;
  }
  return (*_tests)[0];
}


void 
QtTestRunner::run( bool autoRun )
{
  TestRunnerDlg *dlg = new TestRunnerDlg( qApp->mainWidget(), 
                                          "QtTestRunner", 
                                          TRUE );
  dlg->setModel( new TestRunnerModel( getRootTest() ),
                 autoRun );
  dlg->exec();
  delete dlg;
}


void 
QtTestRunner::addTest( CPPUNIT_NS::Test *test )
{
  _tests->push_back( test );
}


CPPUNIT_NS_END
