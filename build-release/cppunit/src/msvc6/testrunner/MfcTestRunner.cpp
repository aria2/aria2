// //////////////////////////////////////////////////////////////////////////
// Implementation file MfcTestRunner.cpp for class MfcTestRunner
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/04/26
// //////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include <cppunit/ui/mfc/MfcTestRunner.h>
#include <cppunit/TestSuite.h>
#include "TestRunnerModel.h"
#include "TestRunnerDlg.h"



CPPUNIT_NS_BEGIN


MfcTestRunner::MfcTestRunner()
  : m_suite( new CPPUNIT_NS::TestSuite() )
{
}


MfcTestRunner::~MfcTestRunner() 
{
  delete m_suite;

  for ( Tests::iterator it = m_tests.begin(); it != m_tests.end(); ++it )
    delete *it;
}


void 
MfcTestRunner::run() 
{ 
  bool comInit = SUCCEEDED( CoInitialize( NULL) );

  TestRunnerModel model( getRootTest() );
  TestRunnerDlg dlg( &model ); 
  dlg.DoModal (); 

  if ( comInit)
    CoUninitialize();
}


void            
MfcTestRunner::addTest( CPPUNIT_NS::Test *test ) 
{ 
  m_tests.push_back( test );
}


void            
MfcTestRunner::addTests( const CppUnitVector<CPPUNIT_NS::Test *> &tests )
{ 
  for ( Tests::const_iterator it=tests.begin();
        it != tests.end();
        ++it )
  {
    addTest( *it );
  }
}


CPPUNIT_NS::Test *   
MfcTestRunner::getRootTest()
{
  if ( m_tests.size() != 1 )
  {
    for ( Tests::iterator it = m_tests.begin(); it != m_tests.end(); ++it )
      m_suite->addTest( *it );
    m_tests.clear();
    return m_suite;
  }
  return m_tests[0];
}

CPPUNIT_NS_END
