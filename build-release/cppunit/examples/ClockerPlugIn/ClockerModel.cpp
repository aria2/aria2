// //////////////////////////////////////////////////////////////////////////
// Implementation file ClockerModel.cpp for class ClockerModel
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2002/06/14
// //////////////////////////////////////////////////////////////////////////
#include "ClockerModel.h"
#include <cppunit/config/SourcePrefix.h>


ClockerModel::ClockerModel()
    : m_testCaseCount( 0 )
    , m_totalTestCaseTime( 0 )
{
}


ClockerModel::~ClockerModel()
{
}


void 
ClockerModel::setExpectedTestCount( int count )
{
  m_tests.reserve( count );
}


void 
ClockerModel::enterTest( CPPUNIT_NS::Test *test,
                         bool isSuite )
{
  m_currentPath.add( test );

  int testIndex = m_tests.size();
  if ( !m_testIndexes.empty() )
    m_tests[ m_testIndexes.top() ].m_childIndexes.push_back( testIndex );
  m_testIndexes.push( testIndex );
  m_testToIndexes.insert( TestToIndexes::value_type( test, testIndex ) );

  TestInfo info;
  info.m_timer.start();
  info.m_path = m_currentPath;
  info.m_isSuite = isSuite;

  m_tests.push_back( info );

  if ( !isSuite )
    ++m_testCaseCount;
}


void 
ClockerModel::exitTest( CPPUNIT_NS::Test *test,
                        bool isSuite )
{
  m_tests[ m_testIndexes.top() ].m_timer.finish();
  if ( !isSuite )
    m_totalTestCaseTime += m_tests.back().m_timer.elapsedTime();

  m_currentPath.up();
  m_testIndexes.pop();
}


double 
ClockerModel::totalElapsedTime() const
{
  return m_tests[0].m_timer.elapsedTime();
}


double 
ClockerModel::averageTestCaseTime() const
{
  double average = 0;
  if ( m_testCaseCount > 0 )
    average = m_totalTestCaseTime / m_testCaseCount;
  return average;
}


double 
ClockerModel::testTimeFor( int testIndex ) const
{
  return m_tests[ testIndex ].m_timer.elapsedTime();
}


std::string 
ClockerModel::timeStringFor( double time )
{
  char buffer[320];
  const char *format;
  if ( time < 1 )
    format = "%2.3f";
  else if ( time < 10 )
    format = "%3.2f";
  else if (time < 100 )
    format = "%4.1f";
  else
    format = "%6f";

  ::sprintf( buffer, format, time );

  return buffer;
}


bool 
ClockerModel::isSuite( int testIndex ) const
{
  return m_tests[ testIndex ].m_isSuite;
}


const CPPUNIT_NS::TestPath &
ClockerModel::testPathFor( int testIndex ) const
{
  return m_tests[ testIndex ].m_path;
}


int 
ClockerModel::indexOf( CPPUNIT_NS::Test *test ) const
{
  TestToIndexes::const_iterator itFound = m_testToIndexes.find( test );
  if ( itFound != m_testToIndexes.end() )
    return itFound->second;
  return -1;
}


int 
ClockerModel::childCountFor( int testIndex ) const
{
  return m_tests[ testIndex ].m_childIndexes.size();
}


int 
ClockerModel::childAtFor( int testIndex, 
                          int chidIndex ) const
{
  return m_tests[ testIndex ].m_childIndexes[ chidIndex ];
}
