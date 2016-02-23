#include <cppunit/config/SourcePrefix.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TestPath.h>
#include <cppunit/TestResult.h>


CPPUNIT_NS_BEGIN


TestRunner::WrappingSuite::WrappingSuite( const std::string &name ) 
    : TestSuite( name )
{
}


int 
TestRunner::WrappingSuite::getChildTestCount() const
{
  if ( hasOnlyOneTest() )
    return getUniqueChildTest()->getChildTestCount();
  return TestSuite::getChildTestCount();
}


std::string 
TestRunner::WrappingSuite::getName() const
{
  if ( hasOnlyOneTest() )
    return getUniqueChildTest()->getName();
  return TestSuite::getName();
}


Test *
TestRunner::WrappingSuite::doGetChildTestAt( int index ) const
{
  if ( hasOnlyOneTest() )
    return getUniqueChildTest()->getChildTestAt( index );
  return TestSuite::doGetChildTestAt( index );
}


void 
TestRunner::WrappingSuite::run( TestResult *result )
{
  if ( hasOnlyOneTest() )
    getUniqueChildTest()->run( result );
  else
    TestSuite::run( result );
}


bool 
TestRunner::WrappingSuite::hasOnlyOneTest() const
{
  return TestSuite::getChildTestCount() == 1;
}


Test *
TestRunner::WrappingSuite::getUniqueChildTest() const
{
  return TestSuite::doGetChildTestAt( 0 );
}





TestRunner::TestRunner()
    : m_suite( new WrappingSuite() )
{
}


TestRunner::~TestRunner()
{
  delete m_suite;
}


void 
TestRunner::addTest( Test *test )
{
  m_suite->addTest( test ); 
}


void 
TestRunner::run( TestResult &controller,
                 const std::string &testPath )
{
  TestPath path = m_suite->resolveTestPath( testPath );
  Test *testToRun = path.getChildTest();

  controller.runTest( testToRun );
}


CPPUNIT_NS_END

