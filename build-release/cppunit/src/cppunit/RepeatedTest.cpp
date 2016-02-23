#include <cppunit/extensions/RepeatedTest.h>
#include <cppunit/TestResult.h>

CPPUNIT_NS_BEGIN


// Counts the number of test cases that will be run by this test.
int
RepeatedTest::countTestCases() const
{ 
  return TestDecorator::countTestCases() * m_timesRepeat; 
}


// Runs a repeated test
void 
RepeatedTest::run( TestResult *result )
{
  for ( int n = 0; n < m_timesRepeat; n++ ) 
  {
    if ( result->shouldStop() )
        break;

    TestDecorator::run( result );
  }
}


CPPUNIT_NS_END
