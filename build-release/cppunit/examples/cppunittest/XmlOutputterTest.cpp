#include <cppunit/config/SourcePrefix.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/TestFailure.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/XmlOutputterHook.h>
#include "OutputSuite.h"
#include "XmlOutputterTest.h"
#include "XmlUniformiser.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( XmlOutputterTest, 
                                       outputSuiteName() );


XmlOutputterTest::XmlOutputterTest()
{
}


XmlOutputterTest::~XmlOutputterTest()
{
}


void 
XmlOutputterTest::setUp()
{
  m_dummyTests.clear();
  m_result = new CPPUNIT_NS::TestResultCollector();
}


void 
XmlOutputterTest::tearDown()
{
  delete m_result;
  for ( unsigned int index =0; index < m_dummyTests.size(); ++index )
    delete m_dummyTests[index];
  m_dummyTests.clear();
}


void 
XmlOutputterTest::testWriteXmlResultWithNoTest()
{
  CPPUNIT_NS::OStringStream stream;
  CPPUNIT_NS::XmlOutputter outputter( m_result, stream );
  outputter.write();

  std::string actualXml = stream.str();
  std::string expectedXml = 
    "<TestRun>"
      "<FailedTests></FailedTests>"
      "<SuccessfulTests></SuccessfulTests>"
      "<Statistics>"
        "<Tests>0</Tests>"
        "<FailuresTotal>0</FailuresTotal>"
        "<Errors>0</Errors>"
        "<Failures>0</Failures>"
      "</Statistics>"
    "</TestRun>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, actualXml );
}


void 
XmlOutputterTest::testWriteXmlResultWithOneFailure()
{
  addTestFailure( "test1", "message failure1", CPPUNIT_NS::SourceLine( "test.cpp", 3 ) );

  CPPUNIT_NS::OStringStream stream;
  CPPUNIT_NS::XmlOutputter outputter( m_result, stream );
  outputter.write();

  std::string actualXml = stream.str();
  std::string expectedXml = 
    "<TestRun>"
      "<FailedTests>"
        "<FailedTest id=\"1\">"
          "<Name>test1</Name>"
          "<FailureType>Assertion</FailureType>"
          "<Location>"
            "<File>test.cpp</File>"
            "<Line>3</Line>"
          "</Location>"
          "<Message>message failure1</Message>"
        "</FailedTest>"
      "</FailedTests>"
      "<SuccessfulTests></SuccessfulTests>"
      "<Statistics>"
        "<Tests>1</Tests>"
        "<FailuresTotal>1</FailuresTotal>"
        "<Errors>0</Errors>"
        "<Failures>1</Failures>"
      "</Statistics>"
    "</TestRun>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, actualXml );
}


void 
XmlOutputterTest::testWriteXmlResultWithOneError()
{
  addTestError( "test1", "message error1" );

  CPPUNIT_NS::OStringStream stream;
  CPPUNIT_NS::XmlOutputter outputter( m_result, stream );
  outputter.write();

  std::string actualXml = stream.str();
  std::string expectedXml = 
    "<TestRun>"
      "<FailedTests>"
        "<FailedTest id=\"1\">"
          "<Name>test1</Name>"
          "<FailureType>Error</FailureType>"
          "<Message>message error1</Message>"
        "</FailedTest>"
      "</FailedTests>"
      "<SuccessfulTests></SuccessfulTests>"
      "<Statistics>"
        "<Tests>1</Tests>"
        "<FailuresTotal>1</FailuresTotal>"
        "<Errors>1</Errors>"
        "<Failures>0</Failures>"
      "</Statistics>"
    "</TestRun>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, actualXml );
}


void 
XmlOutputterTest::testWriteXmlResultWithOneSuccess()
{
  addTest( "test1" );

  CPPUNIT_NS::OStringStream stream;
  CPPUNIT_NS::XmlOutputter outputter( m_result, stream );
  outputter.write();

  std::string actualXml = stream.str();
  std::string expectedXml = 
    "<TestRun>"
      "<FailedTests></FailedTests>"
      "<SuccessfulTests>"
        "<Test id=\"1\">"
          "<Name>test1</Name>"
        "</Test>"
      "</SuccessfulTests>"
      "<Statistics>"
        "<Tests>1</Tests>"
        "<FailuresTotal>0</FailuresTotal>"
        "<Errors>0</Errors>"
        "<Failures>0</Failures>"
      "</Statistics>"
    "</TestRun>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, actualXml );
}


void 
XmlOutputterTest::testWriteXmlResultWithThreeFailureTwoErrorsAndTwoSuccess()
{
  addTestFailure( "test1", "failure1" );
  addTestError( "test2", "error1" );
  addTestFailure( "test3", "failure2" );
  addTestFailure( "test4", "failure3" );
  addTest( "test5" );
  addTestError( "test6", "error2" );
  addTest( "test7" );

  CPPUNIT_NS::OStringStream stream;
  CPPUNIT_NS::XmlOutputter outputter( m_result, stream );
  outputter.write();

  std::string actualXml = stream.str();
  std::string expectedXml = 
    "<TestRun>"
       "<FailedTests>"
        "<FailedTest id=\"1\">"
          "<Name>test1</Name>"
          "<FailureType>Assertion</FailureType>"
          "<Message>failure1</Message>"
        "</FailedTest>"
        "<FailedTest id=\"2\">"
          "<Name>test2</Name>"
          "<FailureType>Error</FailureType>"
          "<Message>error1</Message>"
        "</FailedTest>"
        "<FailedTest id=\"3\">"
          "<Name>test3</Name>"
          "<FailureType>Assertion</FailureType>"
          "<Message>failure2</Message>"
        "</FailedTest>"
        "<FailedTest id=\"4\">"
          "<Name>test4</Name>"
          "<FailureType>Assertion</FailureType>"
          "<Message>failure3</Message>"
        "</FailedTest>"
        "<FailedTest id=\"6\">"
          "<Name>test6</Name>"
          "<FailureType>Error</FailureType>"
          "<Message>error2</Message>"
        "</FailedTest>"
      "</FailedTests>"
     "<SuccessfulTests>"
        "<Test id=\"5\">"
          "<Name>test5</Name>"
        "</Test>"
        "<Test id=\"7\">"
          "<Name>test7</Name>"
        "</Test>"
      "</SuccessfulTests>"
      "<Statistics>"
        "<Tests>7</Tests>"
        "<FailuresTotal>5</FailuresTotal>"
        "<Errors>2</Errors>"
        "<Failures>3</Failures>"
      "</Statistics>"
    "</TestRun>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, actualXml );
}


class XmlOutputterTest::MockHook : public CPPUNIT_NS::XmlOutputterHook
{
public:
  MockHook( int &beginCalls,
            int &endCalls,
            int &statisticsCalls,
            int &successfulTestCalls,
            int &failedTestCalls )
      : m_beginCalls( beginCalls )
      , m_endCalls( endCalls )
      , m_statisticsCalls( statisticsCalls )
      , m_successfulTestCalls( successfulTestCalls )
      , m_failedTestCalls( failedTestCalls )
  {
  }

  void beginDocument( CPPUNIT_NS::XmlDocument *document )
  {
    ++m_beginCalls;
  }

  void endDocument( CPPUNIT_NS::XmlDocument *document )
  {
    ++m_endCalls;
  }

  void failTestAdded( CPPUNIT_NS::XmlDocument *document,
                      CPPUNIT_NS::XmlElement *testElement,
                      CPPUNIT_NS::Test *test,
                      CPPUNIT_NS::TestFailure *failure )
  {
    ++m_failedTestCalls;
  }

  void successfulTestAdded( CPPUNIT_NS::XmlDocument *document,
                            CPPUNIT_NS::XmlElement *testElement,
                            CPPUNIT_NS::Test *test )
  {
    ++m_successfulTestCalls;
  }

  void statisticsAdded( CPPUNIT_NS::XmlDocument *document,
                        CPPUNIT_NS::XmlElement *statisticsElement )
  {
    ++m_statisticsCalls;
  }

private:
  int &m_beginCalls;
  int &m_endCalls;
  int &m_statisticsCalls;
  int &m_successfulTestCalls;
  int &m_failedTestCalls;
};


void 
XmlOutputterTest::testHook()
{
  int begin =0, end =0, statistics =0, successful =0, failed =0;
  MockHook hook( begin, end, statistics, successful, failed );

  addTest( "test1" );
  addTest( "test2" );
  addTest( "test3" );
  addTestFailure( "testfail1", "assertion failed" );
  addTestError( "testerror1", "exception" );

  CPPUNIT_NS::OStringStream stream;
  CPPUNIT_NS::XmlOutputter outputter( m_result, stream );
  outputter.addHook( &hook );
  outputter.write();

  CPPUNIT_ASSERT_EQUAL( 1, begin );
  CPPUNIT_ASSERT_EQUAL( 1, end );
  CPPUNIT_ASSERT_EQUAL( 1, statistics );
  CPPUNIT_ASSERT_EQUAL( 3, successful );
  CPPUNIT_ASSERT_EQUAL( 2, failed );
}


void 
XmlOutputterTest::addTest( std::string testName )
{
  CPPUNIT_NS::Test *test = makeDummyTest( testName );
  m_result->startTest( test );
  m_result->endTest( test );
}


void 
XmlOutputterTest::addTestFailure( std::string testName,
                                  std::string message,
                                  CPPUNIT_NS::SourceLine sourceLine )
{
  addGenericTestFailure( testName, CPPUNIT_NS::Message(message), sourceLine, false );
}


void 
XmlOutputterTest::addTestError( std::string testName,
                                std::string message,
                                CPPUNIT_NS::SourceLine sourceLine )
{
  addGenericTestFailure( testName, CPPUNIT_NS::Message(message), sourceLine, true );
}


void 
XmlOutputterTest::addGenericTestFailure(  std::string testName,
                                          CPPUNIT_NS::Message message,
                                          CPPUNIT_NS::SourceLine sourceLine,
                                          bool isError )
{
  CPPUNIT_NS::Test *test = makeDummyTest( testName );
  m_result->startTest( test );
  CPPUNIT_NS::TestFailure failure( test, 
                                new CPPUNIT_NS::Exception( message, sourceLine ),
                                isError );
  m_result->addFailure( failure );
  m_result->endTest( test );
}


CPPUNIT_NS::Test *
XmlOutputterTest::makeDummyTest( std::string testName )
{
  CPPUNIT_NS::Test *test = new CPPUNIT_NS::TestCase( testName );
  m_dummyTests.push_back( test );
  return test;
}

