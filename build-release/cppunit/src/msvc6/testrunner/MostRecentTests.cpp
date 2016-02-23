// //////////////////////////////////////////////////////////////////////////
// Implementation file MostRecentTests.cpp for class MostRecentTests
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/06/27
// //////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MostRecentTests.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



MostRecentTests::MostRecentTests()
{
}


MostRecentTests::~MostRecentTests()
{
}


void 
MostRecentTests::setLastTestRun( CPPUNIT_NS::Test *test )
{
  for ( TestRuns::iterator it = m_runs.begin(); it != m_runs.end(); ++it )
  {
    if ( it->second == test )
    {
      m_runs.erase( it );
      break;
    }
  }
  
  if ( test != NULL )
    m_runs.push_front( TestRun( test->getName(), test ) );
}


CPPUNIT_NS::Test *
MostRecentTests::lastTestRun() const
{
  return m_runs.front().second;
}


int 
MostRecentTests::getRunCount() const
{
  return m_runs.size();
}


CPPUNIT_NS::Test *
MostRecentTests::getTestAt( int indexTest ) const
{
  return m_runs.at( indexTest ).second;
}

std::string 
MostRecentTests::getTestNameAt( int indexTest ) const
{
  return m_runs.at( indexTest ).first;
}
