#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestListener.h>
#include <cppunit/TestResult.h>
#include <cppunit/tools/Algorithm.h>
#include <algorithm>
#include "DefaultProtector.h"
#include "ProtectorChain.h"
#include "ProtectorContext.h"

CPPUNIT_NS_BEGIN


TestResult::TestResult( SynchronizationObject *syncObject )
    : SynchronizedObject( syncObject )
    , m_protectorChain( new ProtectorChain() )
    , m_stop( false )
{ 
  m_protectorChain->push( new DefaultProtector() );
}


TestResult::~TestResult()
{
  delete m_protectorChain;
}


void 
TestResult::reset()
{
  ExclusiveZone zone( m_syncObject ); 
  m_stop = false;
}


void 
TestResult::addError( Test *test, 
                      Exception *e )
{ 
  TestFailure failure( test, e, true );
  addFailure( failure );
}


void 
TestResult::addFailure( Test *test, Exception *e )
{ 
  TestFailure failure( test, e, false );
  addFailure( failure );
}


void 
TestResult::addFailure( const TestFailure &failure )
{
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->addFailure( failure );
}


void 
TestResult::startTest( Test *test )
{ 
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->startTest( test );
}

  
void 
TestResult::endTest( Test *test )
{ 
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->endTest( test );
}


void 
TestResult::startSuite( Test *test )
{
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->startSuite( test );
}


void 
TestResult::endSuite( Test *test )
{
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->endSuite( test );
}


bool 
TestResult::shouldStop() const
{ 
  ExclusiveZone zone( m_syncObject );
  return m_stop; 
}


void 
TestResult::stop()
{ 
  ExclusiveZone zone( m_syncObject );
  m_stop = true; 
}


void 
TestResult::addListener( TestListener *listener )
{
  ExclusiveZone zone( m_syncObject ); 
  m_listeners.push_back( listener );
}


void 
TestResult::removeListener ( TestListener *listener )
{
  ExclusiveZone zone( m_syncObject ); 
  removeFromSequence( m_listeners, listener );
}


void 
TestResult::runTest( Test *test )
{
  startTestRun( test );
  test->run( this );
  endTestRun( test );
}


void 
TestResult::startTestRun( Test *test )
{
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->startTestRun( test, this );
}


void 
TestResult::endTestRun( Test *test )
{
  ExclusiveZone zone( m_syncObject ); 
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end(); 
        ++it )
    (*it)->endTestRun( test, this );
}


bool 
TestResult::protect( const Functor &functor,
                     Test *test,
                     const std::string &shortDescription )
{
  ProtectorContext context( test, this, shortDescription );
  return m_protectorChain->protect( functor, context );
}


void 
TestResult::pushProtector( Protector *protector )
{
  m_protectorChain->push( protector );
}


void 
TestResult::popProtector()
{
  m_protectorChain->pop();
}


CPPUNIT_NS_END
