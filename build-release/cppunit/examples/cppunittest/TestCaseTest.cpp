#include "CoreSuite.h"
#include "FailureException.h"
#include "MockTestCase.h"
#include "TestCaseTest.h"
#include <cppunit/TestResult.h>

/*
 - test have been done to check exception management in run(). other 
   tests need to be added to check the other aspect of TestCase.
 */

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TestCaseTest,
                                       coreSuiteName() );


TestCaseTest::TestCaseTest()
{
}


TestCaseTest::~TestCaseTest()
{
}


void 
TestCaseTest::setUp()
{
  m_testListener = new MockTestListener( "mock-testlistener" );
  m_result = new CPPUNIT_NS::TestResult();
  m_result->addListener( m_testListener );
}


void 
TestCaseTest::tearDown()
{
  delete m_result;
  delete m_testListener;
}


void 
TestCaseTest::testSetUpFailure()
{
  checkFailure( true, false, false );
}


void 
TestCaseTest::testRunTestFailure()
{
  checkFailure( false, true, false );
}


void 
TestCaseTest::testTearDownFailure()
{
  checkFailure( false, false, true );
}


void 
TestCaseTest::testFailAll()
{
  checkFailure( true, true, true );
}


void 
TestCaseTest::testNoFailure()
{
  checkFailure( false, false, false );
}


void 
TestCaseTest::checkFailure( bool failSetUp, 
                            bool failRunTest,
                            bool failTearDown )
{
  try
  {
    MockTestCase testCase( "mock-test" );
    if ( failSetUp )
      testCase.makeSetUpThrow();
    if ( failRunTest )
      testCase.makeRunTestThrow();
    if ( failTearDown )
      testCase.makeTearDownThrow();
    testCase.setExpectedSetUpCall( 1 );
    testCase.setExpectedRunTestCall( failSetUp ? 0 : 1 );
    testCase.setExpectedTearDownCall( failSetUp ? 0 : 1 );
    
    testCase.run( m_result );

    testCase.verify();
  }
  catch ( FailureException & )
  {
    CPPUNIT_ASSERT_MESSAGE( "exception should have been caught", false );
  }
}


void 
TestCaseTest::testCountTestCases()
{
  CPPUNIT_NS::TestCase test;
  CPPUNIT_ASSERT_EQUAL( 1, test.countTestCases() );
}


void 
TestCaseTest::testDefaultConstructor()
{
  CPPUNIT_NS::TestCase test;
  CPPUNIT_ASSERT_EQUAL( std::string(""), test.getName() );
}


void 
TestCaseTest::testConstructorWithName()
{
  std::string testName( "TestName" );
  CPPUNIT_NS::TestCase test( testName );
  CPPUNIT_ASSERT_EQUAL( testName, test.getName() );
}


void 
TestCaseTest::testTwoRun()
{
  MockTestCase test1( "mocktest1" );
  test1.makeRunTestThrow();
  m_testListener->setExpectedStartTestCall( 2 );
  m_testListener->setExpectedAddFailureCall( 2 );
  m_testListener->setExpectedEndTestCall( 2 );

  test1.run( m_result );
  test1.run( m_result );

  m_testListener->verify();
}


void 
TestCaseTest::testGetChildTestCount()
{
  CPPUNIT_NS::TestCase test( "test" );
  CPPUNIT_ASSERT_EQUAL( 0, test.getChildTestCount() );
}


void 
TestCaseTest::testGetChildTestAtThrow()
{
  CPPUNIT_NS::TestCase test( "test" );
  test.getChildTestAt( 0 );
}
