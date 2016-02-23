#include "CoreSuite.h"
#include "TestResultCollectorTest.h"



CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TestResultCollectorTest,
                                       coreSuiteName() );


TestResultCollectorTest::TestResultCollectorTest()
{
}


TestResultCollectorTest::~TestResultCollectorTest()
{
}


void 
TestResultCollectorTest::setUp()
{
  m_lockCount = 0;
  m_unlockCount = 0;
  m_result = new CPPUNIT_NS::TestResultCollector();
  m_synchronizedResult = new SynchronizedTestResult( this );  
  m_test = new CPPUNIT_NS::TestCase();
  m_test2 = new CPPUNIT_NS::TestCase();
}


void 
TestResultCollectorTest::tearDown()
{
  delete m_test2;
  delete m_test;
  delete m_synchronizedResult;
  delete m_result;
}


void 
TestResultCollectorTest::testConstructor()
{
  checkResult( 0, 0, 0 );
}


void 
TestResultCollectorTest::testAddTwoErrors()
{
  CPPUNIT_NS::Message errorMessage1( "First Error" );
  CPPUNIT_NS::Message errorMessage2( "Second Error" );
  {
    CPPUNIT_NS::TestFailure failure1( m_test, 
                                   new CPPUNIT_NS::Exception( errorMessage1 ),
                                   true );
    m_result->addFailure( failure1 );

    CPPUNIT_NS::TestFailure failure2( m_test2, 
                                   new CPPUNIT_NS::Exception( errorMessage2 ),
                                   true );
    m_result->addFailure( failure2 );
  } // ensure that the test result duplicate the failures.

  checkResult( 0, 2, 0 );
  checkFailure( m_result->failures()[0],
                errorMessage1,
                m_test,
                true );
  checkFailure( m_result->failures()[1],
                errorMessage2,
                m_test2,
                true );
}


void 
TestResultCollectorTest::testAddTwoFailures()
{
  CPPUNIT_NS::Message errorMessage1( "First Failure" );
  CPPUNIT_NS::Message errorMessage2( "Second Failure" );
  {
    CPPUNIT_NS::TestFailure failure1( m_test, 
                                   new CPPUNIT_NS::Exception( errorMessage1 ),
                                   false );
    m_result->addFailure( failure1 );

    CPPUNIT_NS::TestFailure failure2( m_test2, 
                                   new CPPUNIT_NS::Exception( errorMessage2 ),
                                   false );
    m_result->addFailure( failure2 );
  } // ensure that the test result duplicate the failures.
  checkResult( 2, 0, 0 );
  checkFailure( m_result->failures()[0],
                errorMessage1,
                m_test,
                false );
  checkFailure( m_result->failures()[1],
                errorMessage2,
                m_test2,
                false );
}


void 
TestResultCollectorTest::testStartTest()
{
  m_result->startTest( m_test );
  m_result->startTest( m_test );
  checkResult( 0, 0, 2 );
}


void 
TestResultCollectorTest::testWasSuccessfulWithNoTest()
{
  checkWasSuccessful( true );
}


void 
TestResultCollectorTest::testWasSuccessfulWithErrors()
{
  addError( "Error1" );
  addError( "Error2" );
  checkWasSuccessful( false );
}


void 
TestResultCollectorTest::testWasSuccessfulWithFailures()
{
  addFailure( "Failure1" );
  addFailure( "Failure2" );
  checkWasSuccessful( false );
}


void 
TestResultCollectorTest::testWasSuccessfulWithErrorsAndFailures()
{
  addError( "Error1" );
  addFailure( "Failure2" );
  checkWasSuccessful( false );
}


void 
TestResultCollectorTest::testWasSuccessfulWithSuccessfulTest()
{
  m_result->startTest( m_test );
  m_result->endTest( m_test );
  m_result->startTest( m_test2 );
  m_result->endTest( m_test2 );
  checkWasSuccessful( true );
}


void 
TestResultCollectorTest::testSynchronizationAddFailure()
{
  addFailure( "Failure1", m_test, false, m_synchronizedResult );
  checkSynchronization();
}


void 
TestResultCollectorTest::testSynchronizationStartTest()
{
  m_synchronizedResult->startTest( m_test );
  checkSynchronization();
}


void 
TestResultCollectorTest::testSynchronizationRunTests()
{
  m_synchronizedResult->runTests();
  checkSynchronization();
}


void 
TestResultCollectorTest::testSynchronizationTestErrors()
{
  m_synchronizedResult->testErrors();
  checkSynchronization();
}


void 
TestResultCollectorTest::testSynchronizationTestFailures()
{
  m_synchronizedResult->testFailures();
  checkSynchronization();
}


void 
TestResultCollectorTest::testSynchronizationFailures()
{
  m_synchronizedResult->failures();
  checkSynchronization();
}


void 
TestResultCollectorTest::testSynchronizationWasSuccessful()
{
  m_synchronizedResult->wasSuccessful();
  checkSynchronization();
}


void 
TestResultCollectorTest::checkResult( int failures,
                             int errors,
                             int testsRun )
{
  CPPUNIT_ASSERT_EQUAL( testsRun, m_result->runTests() );
  CPPUNIT_ASSERT_EQUAL( errors, m_result->testErrors() );
  CPPUNIT_ASSERT_EQUAL( failures, m_result->testFailures() );
  CPPUNIT_ASSERT_EQUAL( errors + failures, 
                        m_result->testFailuresTotal() );
}


void
TestResultCollectorTest::checkFailure( CPPUNIT_NS::TestFailure *failure,
                                       CPPUNIT_NS::Message expectedMessage,
                                       CPPUNIT_NS::Test *expectedTest,
                                       bool expectedIsError )
{
  CPPUNIT_NS::Message actualMessage( failure->thrownException()->message() );
  CPPUNIT_ASSERT( expectedMessage == actualMessage );
  CPPUNIT_ASSERT_EQUAL( expectedTest, failure->failedTest() );
  CPPUNIT_ASSERT_EQUAL( expectedIsError, failure->isError() );
}


void 
TestResultCollectorTest::checkWasSuccessful( bool shouldBeSuccessful )
{
  CPPUNIT_ASSERT_EQUAL( shouldBeSuccessful, m_result->wasSuccessful() );
}


void 
TestResultCollectorTest::locked()
{
  CPPUNIT_ASSERT_EQUAL( m_lockCount, m_unlockCount );
  ++m_lockCount;
}


void 
TestResultCollectorTest::unlocked()
{
  ++m_unlockCount;
  CPPUNIT_ASSERT_EQUAL( m_lockCount, m_unlockCount );
}


void 
TestResultCollectorTest::checkSynchronization()
{
  CPPUNIT_ASSERT_EQUAL( m_lockCount, m_unlockCount );
  CPPUNIT_ASSERT( m_lockCount > 0 );
}


void 
TestResultCollectorTest::addFailure( std::string message )
{
  addFailure( message, m_test, false, m_result );
}


void 
TestResultCollectorTest::addError( std::string message )
{
  addFailure( message, m_test, true, m_result );
}


void 
TestResultCollectorTest::addFailure( std::string message, 
                                     CPPUNIT_NS::Test *failedTest, 
                                     bool isError,
                                     CPPUNIT_NS::TestResultCollector *result )
{
  CPPUNIT_NS::TestFailure failure( failedTest, 
                                new CPPUNIT_NS::Exception( CPPUNIT_NS::Message( message ) ), 
                                isError );
  result->addFailure( failure );
}
