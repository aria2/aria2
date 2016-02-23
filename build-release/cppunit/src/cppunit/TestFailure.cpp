#include <cppunit/Exception.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>

CPPUNIT_NS_BEGIN


/// Constructs a TestFailure with the given test and exception.
TestFailure::TestFailure( Test *failedTest, 
                          Exception *thrownException,
                          bool isError ) :
    m_failedTest( failedTest ), 
    m_thrownException( thrownException ),
    m_isError( isError )
{
}

/// Deletes the owned exception.
TestFailure::~TestFailure()
{ 
  delete m_thrownException; 
}

/// Gets the failed test.
Test *
TestFailure::failedTest() const
{ 
  return m_failedTest; 
}


/// Gets the thrown exception. Never \c NULL.
Exception *
TestFailure::thrownException() const
{ 
  return m_thrownException; 
}


/// Gets the failure location.
SourceLine 
TestFailure::sourceLine() const
{
  return m_thrownException->sourceLine();
}


/// Indicates if the failure is a failed assertion or an error.
bool 
TestFailure::isError() const
{
  return m_isError;
}


/// Gets the name of the failed test.
std::string 
TestFailure::failedTestName() const
{
  return m_failedTest->getName();
}


TestFailure *
TestFailure::clone() const
{
  return new TestFailure( m_failedTest, m_thrownException->clone(), m_isError );
}


CPPUNIT_NS_END
