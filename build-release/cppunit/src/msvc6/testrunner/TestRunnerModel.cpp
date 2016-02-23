// //////////////////////////////////////////////////////////////////////////
// Implementation file TestRunnerModel.cpp for class TestRunnerModel
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/04/26
// //////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TestRunnerModel.h"
#include <cppunit/tools/Algorithm.h>
#include <algorithm>
#include <stdexcept>
#include <cppunit/testsuite.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



const CString TestRunnerModel::settingKey( _T("CppUnit") );
const CString TestRunnerModel::settingMainDialogKey( _T( "MainDialog" ) );
const CString TestRunnerModel::settingBrowseDialogKey( _T( "BrowseDialog" ) );


TestRunnerModel::TestRunnerModel( CPPUNIT_NS::Test *rootTest ) :
    m_rootTest( rootTest )
{
}


TestRunnerModel::~TestRunnerModel()
{
}

const TestRunnerModel::History &
TestRunnerModel::history() const
{
  return m_history;
}


void 
TestRunnerModel::selectHistoryTest( CPPUNIT_NS::Test *test )
{
   CPPUNIT_NS::removeFromSequence( m_history, test );
  
  if ( test != NULL )
    m_history.push_front( test );
}


CPPUNIT_NS::Test *
TestRunnerModel::selectedTest() const
{
  if ( m_history.size() > 0 )
    return m_history[0];
  return NULL;
}


void 
TestRunnerModel::loadSettings(Settings & s)
{
  CWinApp *app = AfxGetApp();
  ASSERT( app != NULL );

  int autorun = app->GetProfileInt( _T("CppUnit"),
                                    _T("AutorunAtStartup"),
                                    1 );
  s.autorunOnLaunch = (autorun == 1);

  s.col_1 = app->GetProfileInt( _T("CppUnit"), _T("Col_1"), 40 );
  s.col_2 = app->GetProfileInt( _T("CppUnit"), _T("Col_2"), 40 );
  s.col_3 = app->GetProfileInt( _T("CppUnit"), _T("Col_3"), 40 );
  s.col_4 = app->GetProfileInt( _T("CppUnit"), _T("Col_4"), 40 );

  loadHistory();
}


void 
TestRunnerModel::loadHistory()
{
  m_history.clear();
  int idx = 1;
  do
  {
    CString testName = loadHistoryEntry( idx++ );
    if ( testName.IsEmpty() )
      break;

    try
    {
      m_history.push_back( m_rootTest->findTest( toAnsiString(testName ) ) );
    }
    catch ( std::invalid_argument &)
    {
    }
  }
  while ( true );
}


CString
TestRunnerModel::loadHistoryEntry( int idx )
{
  CWinApp *app = AfxGetApp();
  ASSERT( app != NULL );

  return app->GetProfileString( _T("CppUnit"), getHistoryEntryName( idx ) );
}


void 
TestRunnerModel::saveSettings( const Settings & s )
{
  CWinApp *app = AfxGetApp();
  ASSERT( app != NULL );

  int autorun = s.autorunOnLaunch ? 1 : 0;
  app->WriteProfileInt( _T("CppUnit"), _T("AutorunAtStartup"), autorun );

  app->WriteProfileInt( _T("CppUnit"), _T("Col_1"),	 s.col_1 );
  app->WriteProfileInt( _T("CppUnit"), _T("Col_2"),	 s.col_2 );
  app->WriteProfileInt( _T("CppUnit"), _T("Col_3"),	 s.col_3 );
  app->WriteProfileInt( _T("CppUnit"), _T("Col_4"),	 s.col_4 );

  int idx = 1;
  for ( History::const_iterator it = m_history.begin(); 
        it != m_history.end(); 
        ++it , ++idx )
  {
    CPPUNIT_NS::Test *test = *it;
    saveHistoryEntry( idx, test->getName().c_str() );
  }
}


void 
TestRunnerModel::saveHistoryEntry( int idx, 
                                   CString testName )
{
  CWinApp *app = AfxGetApp();
  ASSERT( app != NULL );

  app->WriteProfileString( _T("CppUnit"),
                           getHistoryEntryName( idx ),
                           testName );
}


CString
TestRunnerModel::getHistoryEntryName( int idx ) const
{
  CString entry;
  entry.Format( _T("HistoryTest%d"), idx );
  return entry;
}


CPPUNIT_NS::Test *
TestRunnerModel::rootTest()
{
  return m_rootTest;
}


void 
TestRunnerModel::setRootTest( CPPUNIT_NS::Test *test )
{
  m_rootTest = test;
}


CPPUNIT_NS::Test * 
TestRunnerModel::findTestByName( CString name ) const
{
  return findTestByNameFor( name, m_rootTest );
}


CPPUNIT_NS::Test * 
TestRunnerModel::findTestByNameFor( const CString &name, 
                                    CPPUNIT_NS::Test *test ) const
{
  if ( name == test->getName().c_str() )
    return test;

  CPPUNIT_NS::TestSuite *suite = dynamic_cast<CPPUNIT_NS::TestSuite *>(test);
  if ( suite == NULL )
    return NULL;

  const std::vector<CPPUNIT_NS::Test *> &tests = suite->getTests();
  for ( std::vector<CPPUNIT_NS::Test *>::const_iterator it = tests.begin(); 
        it != tests.end(); 
        ++it )
  {
    CPPUNIT_NS::Test *testFound = findTestByNameFor( name, *it );
    if ( testFound != NULL )
      return testFound;
  }
  return NULL;
}


// Utility method, should be moved somewhere else...
std::string 
TestRunnerModel::toAnsiString( const CString &text )
{
#ifdef _UNICODE
  int bufferLength = ::WideCharToMultiByte( CP_THREAD_ACP, 0, 
                                            text, text.GetLength(),
                                            NULL, 0, NULL, NULL ) +1;
  char *ansiString = new char[bufferLength];
  ::WideCharToMultiByte( CP_THREAD_ACP, 0, 
                         text, text.GetLength(),
                         ansiString, bufferLength, 
                                            NULL,
                                            NULL );

  std::string str( ansiString, bufferLength-1 );
  delete[] ansiString;

  return str;
#else
  return std::string( (LPCTSTR)text );
#endif
}
