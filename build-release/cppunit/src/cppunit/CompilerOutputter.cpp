#include <cppunit/config/SourcePrefix.h>
#include <cppunit/Exception.h>
#include <cppunit/SourceLine.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/CompilerOutputter.h>
#include <algorithm>
#include <cppunit/tools/StringTools.h>


CPPUNIT_NS_BEGIN


CompilerOutputter::CompilerOutputter( TestResultCollector *result,
                                      OStream &stream,
                                      const std::string &locationFormat )
    : m_result( result )
    , m_stream( stream )
    , m_locationFormat( locationFormat )
    , m_wrapColumn( CPPUNIT_WRAP_COLUMN )
{
}


CompilerOutputter::~CompilerOutputter()
{
}


void 
CompilerOutputter::setLocationFormat( const std::string &locationFormat )
{
  m_locationFormat = locationFormat;
}


CompilerOutputter *
CompilerOutputter::defaultOutputter( TestResultCollector *result,
                                     OStream &stream )
{
  return new CompilerOutputter( result, stream );
}


void 
CompilerOutputter::write()
{
  if ( m_result->wasSuccessful() )
    printSuccess();
  else
    printFailureReport();
}


void 
CompilerOutputter::printSuccess()
{
  m_stream  << "OK (" << m_result->runTests()  << ")\n";
}


void 
CompilerOutputter::printFailureReport()
{
  printFailuresList();
  printStatistics();
}


void 
CompilerOutputter::printFailuresList()
{
  for ( int index =0; index < m_result->testFailuresTotal(); ++index)
  {
    printFailureDetail( m_result->failures()[ index ] );
  }
}


void 
CompilerOutputter::printFailureDetail( TestFailure *failure )
{
  printFailureLocation( failure->sourceLine() );
  printFailureType( failure );
  printFailedTestName( failure );
  printFailureMessage( failure );
}

 
void 
CompilerOutputter::printFailureLocation( SourceLine sourceLine )
{
  if ( !sourceLine.isValid() )
  {
    m_stream  <<  "##Failure Location unknown## : ";
    return;
  }

  std::string location;
  for ( unsigned int index = 0; index < m_locationFormat.length(); ++index )
  {
    char c = m_locationFormat[ index ];
    if ( c == '%'  &&  ( index+1 < m_locationFormat.length() ) )
    {
      char command = m_locationFormat[index+1];
      if ( processLocationFormatCommand( command, sourceLine ) )
      {
        ++index;
        continue;
      }
    }

    m_stream  << c;
  }
}


bool 
CompilerOutputter::processLocationFormatCommand( char command, 
                                                 const SourceLine &sourceLine )
{
  switch ( command )
  {
  case 'p':
    m_stream  <<  sourceLine.fileName();
    return true;
  case 'l':
    m_stream  <<  sourceLine.lineNumber();
    return true;
  case 'f':
    m_stream  <<  extractBaseName( sourceLine.fileName() );
    return true;
  }
  
  return false;
}


std::string 
CompilerOutputter::extractBaseName( const std::string &fileName ) const
{
  int indexLastDirectorySeparator = fileName.find_last_of( '/' );
  
  if ( indexLastDirectorySeparator < 0 )
    indexLastDirectorySeparator = fileName.find_last_of( '\\' );
  
  if ( indexLastDirectorySeparator < 0 )
    return fileName;

  return fileName.substr( indexLastDirectorySeparator +1 );
}


void 
CompilerOutputter::printFailureType( TestFailure *failure )
{
  m_stream  <<  (failure->isError() ? "Error" : "Assertion");
}


void 
CompilerOutputter::printFailedTestName( TestFailure *failure )
{
  m_stream  <<  "\nTest name: "  <<  failure->failedTestName();
}


void 
CompilerOutputter::printFailureMessage( TestFailure *failure )
{
  m_stream  <<  "\n";
  Exception *thrownException = failure->thrownException();
  m_stream  << thrownException->message().shortDescription()  <<  "\n";

  std::string message = thrownException->message().details();
  if ( m_wrapColumn > 0 )
    message = StringTools::wrap( message, m_wrapColumn );

  m_stream  <<  message  <<  "\n";
}


void 
CompilerOutputter::printStatistics()
{
  m_stream  <<  "Failures !!!\n";
  m_stream  <<  "Run: "  <<  m_result->runTests()  << "   "
            <<  "Failure total: "  <<  m_result->testFailuresTotal()  << "   "
            <<  "Failures: "  <<  m_result->testFailures()  << "   "
            <<  "Errors: "  <<  m_result->testErrors()
            <<  "\n";
}


void 
CompilerOutputter::setWrapColumn( int wrapColumn )
{
  m_wrapColumn = wrapColumn;
}


void 
CompilerOutputter::setNoWrap()
{
  m_wrapColumn = 0;
}


int 
CompilerOutputter::wrapColumn() const
{
  return m_wrapColumn;
}


CPPUNIT_NS_END
