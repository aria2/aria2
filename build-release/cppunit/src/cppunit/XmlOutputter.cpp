#include <cppunit/Exception.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/XmlOutputterHook.h>
#include <cppunit/tools/XmlDocument.h>
#include <cppunit/tools/XmlElement.h>
#include <stdlib.h>
#include <algorithm>


CPPUNIT_NS_BEGIN


XmlOutputter::XmlOutputter( TestResultCollector *result,
                            OStream &stream,
                            std::string encoding )
  : m_result( result )
  , m_stream( stream )
  , m_xml( new XmlDocument( encoding ) )
{
}


XmlOutputter::~XmlOutputter()
{
  delete m_xml;
}


void 
XmlOutputter::addHook( XmlOutputterHook *hook )
{
  m_hooks.push_back( hook );
}


void 
XmlOutputter::removeHook( XmlOutputterHook *hook )
{
  m_hooks.erase( std::find( m_hooks.begin(), m_hooks.end(), hook ) );
}


void 
XmlOutputter::write()
{
  setRootNode();
  m_stream  <<  m_xml->toString();
}


void 
XmlOutputter::setStyleSheet( const std::string &styleSheet )
{
  m_xml->setStyleSheet( styleSheet );
}


void
XmlOutputter::setStandalone( bool standalone )
{
  m_xml->setStandalone( standalone );
}
 

void
XmlOutputter::setRootNode()
{
  XmlElement *rootNode = new XmlElement( "TestRun" );
  m_xml->setRootElement( rootNode );

  for ( Hooks::iterator it = m_hooks.begin(); it != m_hooks.end(); ++it )
    (*it)->beginDocument( m_xml );

  FailedTests failedTests;
  fillFailedTestsMap( failedTests );

  addFailedTests( failedTests, rootNode );
  addSuccessfulTests( failedTests, rootNode );
  addStatistics( rootNode );

  for ( Hooks::iterator itEnd = m_hooks.begin(); itEnd != m_hooks.end(); ++itEnd )
    (*itEnd)->endDocument( m_xml );
}


void 
XmlOutputter::fillFailedTestsMap( FailedTests &failedTests )
{
  const TestResultCollector::TestFailures &failures = m_result->failures();
  TestResultCollector::TestFailures::const_iterator itFailure = failures.begin();
  while ( itFailure != failures.end() )
  {
    TestFailure *failure = *itFailure++;
    failedTests.insert( std::pair<Test* const, TestFailure*>(failure->failedTest(), failure ) );
  }
}


void
XmlOutputter::addFailedTests( FailedTests &failedTests,
                              XmlElement *rootNode )
{
  XmlElement *testsNode = new XmlElement( "FailedTests" );
  rootNode->addElement( testsNode );

  const TestResultCollector::Tests &tests = m_result->tests();
  for ( unsigned int testNumber = 0; testNumber < tests.size(); ++testNumber )
  {
    Test *test = tests[testNumber];
    if ( failedTests.find( test ) != failedTests.end() )
      addFailedTest( test, failedTests[test], testNumber+1, testsNode );
  }
}


void
XmlOutputter::addSuccessfulTests( FailedTests &failedTests,
                                           XmlElement *rootNode )
{
  XmlElement *testsNode = new XmlElement( "SuccessfulTests" );
  rootNode->addElement( testsNode );

  const TestResultCollector::Tests &tests = m_result->tests();
  for ( unsigned int testNumber = 0; testNumber < tests.size(); ++testNumber )
  {
    Test *test = tests[testNumber];
    if ( failedTests.find( test ) == failedTests.end() )
      addSuccessfulTest( test, testNumber+1, testsNode );
  }
}


void
XmlOutputter::addStatistics( XmlElement *rootNode )
{
  XmlElement *statisticsElement = new XmlElement( "Statistics" );
  rootNode->addElement( statisticsElement );
  statisticsElement->addElement( new XmlElement( "Tests", m_result->runTests() ) );
  statisticsElement->addElement( new XmlElement( "FailuresTotal", 
                                                 m_result->testFailuresTotal() ) );
  statisticsElement->addElement( new XmlElement( "Errors", m_result->testErrors() ) );
  statisticsElement->addElement( new XmlElement( "Failures", m_result->testFailures() ) );

  for ( Hooks::iterator it = m_hooks.begin(); it != m_hooks.end(); ++it )
    (*it)->statisticsAdded( m_xml, statisticsElement );
}


void
XmlOutputter::addFailedTest( Test *test,
                             TestFailure *failure,
                             int testNumber,
                             XmlElement *testsNode )
{
  Exception *thrownException = failure->thrownException();
  
  XmlElement *testElement = new XmlElement( "FailedTest" );
  testsNode->addElement( testElement );
  testElement->addAttribute( "id", testNumber );
  testElement->addElement( new XmlElement( "Name", test->getName() ) );
  testElement->addElement( new XmlElement( "FailureType", 
                                           failure->isError() ? "Error" : 
                                                                "Assertion" ) );

  if ( failure->sourceLine().isValid() )
    addFailureLocation( failure, testElement );

  testElement->addElement( new XmlElement( "Message", thrownException->what() ) );

  for ( Hooks::iterator it = m_hooks.begin(); it != m_hooks.end(); ++it )
    (*it)->failTestAdded( m_xml, testElement, test, failure );
}


void
XmlOutputter::addFailureLocation( TestFailure *failure,
                                  XmlElement *testElement )
{
  XmlElement *locationNode = new XmlElement( "Location" );
  testElement->addElement( locationNode );
  SourceLine sourceLine = failure->sourceLine();
  locationNode->addElement( new XmlElement( "File", sourceLine.fileName() ) );
  locationNode->addElement( new XmlElement( "Line", sourceLine.lineNumber() ) );
}


void
XmlOutputter::addSuccessfulTest( Test *test, 
                                 int testNumber,
                                 XmlElement *testsNode )
{
  XmlElement *testElement = new XmlElement( "Test" );
  testsNode->addElement( testElement );
  testElement->addAttribute( "id", testNumber );
  testElement->addElement( new XmlElement( "Name", test->getName() ) );

  for ( Hooks::iterator it = m_hooks.begin(); it != m_hooks.end(); ++it )
    (*it)->successfulTestAdded( m_xml, testElement, test );
}


CPPUNIT_NS_END
