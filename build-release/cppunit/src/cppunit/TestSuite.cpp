#include <cppunit/config/SourcePrefix.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>

CPPUNIT_NS_BEGIN


/// Default constructor
TestSuite::TestSuite( std::string name )
    : TestComposite( name )
{
}


/// Destructor
TestSuite::~TestSuite()
{ 
  deleteContents(); 
}


/// Deletes all tests in the suite.
void 
TestSuite::deleteContents()
{
  int childCount = getChildTestCount();
  for ( int index =0; index < childCount; ++index )
    delete getChildTestAt( index );

  m_tests.clear();
}


/// Adds a test to the suite. 
void 
TestSuite::addTest( Test *test )
{ 
  m_tests.push_back( test ); 
}


const CppUnitVector<Test *> &
TestSuite::getTests() const
{
  return m_tests;
}


int 
TestSuite::getChildTestCount() const
{
  return m_tests.size();
}


Test *
TestSuite::doGetChildTestAt( int index ) const
{
  return m_tests[index];
}


CPPUNIT_NS_END

