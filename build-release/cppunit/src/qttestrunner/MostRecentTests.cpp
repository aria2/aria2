// //////////////////////////////////////////////////////////////////////////
// Implementation file MostRecentTests.cpp for class MostRecentTests
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/20
// //////////////////////////////////////////////////////////////////////////

#include "MostRecentTests.h"


MostRecentTests::MostRecentTests()
{
}


MostRecentTests::~MostRecentTests()
{
}


void 
MostRecentTests::setTestToRun( CPPUNIT_NS::Test *test )
{
  m_tests.removeRef( test );
  m_tests.prepend( test );

  const int maxRecentTest = 20;
  if ( m_tests.count() > maxRecentTest )
    m_tests.remove( maxRecentTest );

  emit listChanged();
  emit testToRunChanged( testToRun() );
}


CPPUNIT_NS::Test *
MostRecentTests::testToRun()
{
  return testAt( 0 );
}


void 
MostRecentTests::selectTestToRun( int index )
{
  if ( index < testCount() )
    setTestToRun( testAt( index ) );
}


int 
MostRecentTests::testCount()
{
  return m_tests.count();
}


QString 
MostRecentTests::testNameAt( int index )
{
  return QString::fromLatin1( testAt( index )->getName().c_str() );
}


CPPUNIT_NS::Test *
MostRecentTests::testAt( int index )
{
  return m_tests.at( index );
}
