#include <cppunit/Exception.h>
#include <cppunit/SourceLine.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/TestResultCollector.h>


CPPUNIT_NS_BEGIN


TextOutputter::TextOutputter( TestResultCollector *result,
                              OStream &stream )
    : m_result( result )
    , m_stream( stream )
{
}


TextOutputter::~TextOutputter()
{
}


void 
TextOutputter::write() 
{
  printHeader();
  m_stream << "\n";
  printFailures();
  m_stream << "\n";
}


void 
TextOutputter::printFailures()
{
  TestResultCollector::TestFailures::const_iterator itFailure = m_result->failures().begin();
  int failureNumber = 1;
  while ( itFailure != m_result->failures().end() ) 
  {
    m_stream  <<  "\n";
    printFailure( *itFailure++, failureNumber++ );
  }
}


void 
TextOutputter::printFailure( TestFailure *failure,
                             int failureNumber )
{
  printFailureListMark( failureNumber );
  m_stream << ' ';
  printFailureTestName( failure );
  m_stream << ' ';
  printFailureType( failure );
  m_stream << ' ';
  printFailureLocation( failure->sourceLine() );
  m_stream << "\n";
  printFailureDetail( failure->thrownException() );
  m_stream << "\n";
}


void 
TextOutputter::printFailureListMark( int failureNumber )
{
  m_stream << failureNumber << ")";
}


void 
TextOutputter::printFailureTestName( TestFailure *failure )
{
  m_stream << "test: " << failure->failedTestName();
}


void 
TextOutputter::printFailureType( TestFailure *failure )
{
  m_stream << "("
           << (failure->isError() ? "E" : "F")
           << ")";
}


void 
TextOutputter::printFailureLocation( SourceLine sourceLine )
{
  if ( !sourceLine.isValid() )
    return;

  m_stream << "line: " << sourceLine.lineNumber()
           << ' ' << sourceLine.fileName();
}


void 
TextOutputter::printFailureDetail( Exception *thrownException )
{
  m_stream  <<  thrownException->message().shortDescription()  <<  "\n";
  m_stream  <<  thrownException->message().details();
}


void 
TextOutputter::printHeader()
{
  if ( m_result->wasSuccessful() )
    m_stream << "\nOK (" << m_result->runTests () << " tests)\n" ;
  else
  {
    m_stream << "\n";
    printFailureWarning();
    printStatistics();
  }
}


void 
TextOutputter::printFailureWarning()
{
  m_stream  << "!!!FAILURES!!!\n";
}


void 
TextOutputter::printStatistics()
{
  m_stream  << "Test Results:\n";

  m_stream  <<  "Run:  "  <<  m_result->runTests()
            <<  "   Failures: "  <<  m_result->testFailures()
            <<  "   Errors: "  <<  m_result->testErrors()
            <<  "\n";
}


CPPUNIT_NS_END

