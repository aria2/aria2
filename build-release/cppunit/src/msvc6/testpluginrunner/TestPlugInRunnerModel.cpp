// //////////////////////////////////////////////////////////////////////////
// Implementation file TestPlugInRunnerModel.cpp for class TestPlugInRunnerModel
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/06/24
// //////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TestPlugInRunnerModel.h"
#include <cppunit/TestSuite.h>
#include "TestPlugIn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


TestPlugInRunnerModel::TestPlugInRunnerModel() : 
    TestRunnerModel( new CPPUNIT_NS::TestSuite( "Default" ) ),
    m_plugIn( new TestPlugIn( "default plug-in" ) )
{
}


TestPlugInRunnerModel::~TestPlugInRunnerModel()
{
  freeRootTest();
  delete m_plugIn;
}


void 
TestPlugInRunnerModel::setPlugIn( TestPlugIn *plugIn )
{
  freeRootTest();
  delete m_plugIn;
  m_plugIn = plugIn;
  reloadPlugIn();
}


void 
TestPlugInRunnerModel::reloadPlugIn()
{
  try 
  {
    CWaitCursor waitCursor;
    m_history.clear();
    freeRootTest();
    setRootTest( m_plugIn->makeTest() );

    loadHistory();
  }
  catch (...)
  {
    setRootTest( new CPPUNIT_NS::TestSuite( "Default" ) );  
    loadHistory();
    throw;
  }
}


void 
TestPlugInRunnerModel::freeRootTest()
{
  delete m_rootTest;
  m_rootTest = 0;
}


void 
TestPlugInRunnerModel::setRootTest( CPPUNIT_NS::Test *rootTest )
{
  freeRootTest();
  TestRunnerModel::setRootTest( rootTest );
}
