#include "FailureException.h"
#include "MockTestCase.h"
#include <cppunit/TestPath.h>


MockTestCase::MockTestCase( std::string name )
    : CPPUNIT_NS::TestCase( name )
    , m_hasSetUpExpectation( false )
    , m_expectedSetUpCall( 0 )
    , m_actualSetUpCall( 0 )
    , m_hasTearDownExpectation( false )
    , m_expectedTearDownCall( 0 )
    , m_actualTearDownCall( 0 )
    , m_expectRunTestCall( false )
    , m_expectedRunTestCallCount( 0 )
    , m_actualRunTestCallCount( 0 )
    , m_expectCountTestCasesCall( false )
    , m_expectedCountTestCasesCallCount( 0 )
    , m_actualCountTestCasesCallCount( 0 )
    , m_setUpThrow( false )
    , m_tearDownThrow( false )
    , m_runTestThrow( false )
    , m_passingTest( NULL )
{
}


MockTestCase::~MockTestCase()
{
}


int 
MockTestCase::countTestCases() const
{
  MockTestCase *mutableThis = CPPUNIT_CONST_CAST(MockTestCase *, this );
  ++mutableThis->m_actualCountTestCasesCallCount;
  if ( m_expectCountTestCasesCall )
  {
    CPPUNIT_ASSERT_MESSAGE( getName() + ": unexpected MockTestCase::countTestCases() call",
                            m_actualCountTestCasesCallCount <= m_expectedCountTestCasesCallCount );
  }

  return SuperClass::countTestCases();
}


void 
MockTestCase::setUp()
{
  if ( m_hasSetUpExpectation )
  {
    ++m_actualSetUpCall;
    CPPUNIT_ASSERT_MESSAGE( getName() + ": unexpected MockTestCase::setUp() call",
                            m_actualSetUpCall <= m_expectedSetUpCall );
  }

  if ( m_setUpThrow )
    throw FailureException();
}

void 
MockTestCase::tearDown()
{
  if ( m_hasTearDownExpectation )
  {
    ++m_actualTearDownCall;
    CPPUNIT_ASSERT_MESSAGE( getName() + ": unexpected MockTestCase::tearDown() call",
                            m_actualTearDownCall <= m_expectedTearDownCall );
  }

  if ( m_tearDownThrow )
    throw FailureException();
}


void 
MockTestCase::runTest()
{
  ++m_actualRunTestCallCount;
  if ( m_expectRunTestCall )
  {
    CPPUNIT_ASSERT_MESSAGE( getName() + ": unexpected MockTestCase::runTest() call",
                            m_actualRunTestCallCount <= m_expectedRunTestCallCount );
  }

  if ( m_runTestThrow )
    throw FailureException();
}

/*
bool 
MockTestCase::findTestPath( const CPPUNIT_NS::Test *test,
                            CPPUNIT_NS::TestPath &testPath )
{
  if ( m_passingTest == test )
  {
    testPath.add( this );
    return true;
  }

  return false;
}
*/

void 
MockTestCase::setExpectedSetUpCall( int callCount )
{
  m_hasSetUpExpectation = true;
  m_expectedSetUpCall = callCount;
}


void 
MockTestCase::setExpectedTearDownCall( int callCount )
{
}


void 
MockTestCase::setExpectedRunTestCall( int callCount )
{
  m_expectRunTestCall = true;
  m_expectedRunTestCallCount = callCount ;
}


void 
MockTestCase::setExpectedCountTestCasesCall( int callCount )
{
  m_expectCountTestCasesCall = true;
  m_expectedCountTestCasesCallCount = callCount;
}


void 
MockTestCase::makeSetUpThrow()
{
  m_setUpThrow = true;
}


void 
MockTestCase::makeTearDownThrow()
{
  m_tearDownThrow = true;
}


void 
MockTestCase::makeRunTestThrow()
{
  m_runTestThrow = true;
}


void 
MockTestCase::verify()
{
  if ( m_hasSetUpExpectation )
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE( getName() + ": bad MockTestCase::setUp() "
                                  "call count",
                                  m_expectedSetUpCall,
                                  m_actualSetUpCall );
  }

  if ( m_hasTearDownExpectation )
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE( getName() + ": bad MockTestCase::tearDown() "
                                  "call count",
                                  m_expectedTearDownCall,
                                  m_actualTearDownCall );
  }

  if ( m_expectCountTestCasesCall )
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE( getName() + ": bad MockTestCase::countTestCases() "
                                  "call count",
                                  m_expectedCountTestCasesCallCount,
                                  m_actualCountTestCasesCallCount );
  }
  if ( m_expectRunTestCall )
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE( getName() + ": bad MockTestCase::runTest() "
                                  "call count",
                                  m_expectedRunTestCallCount,
                                  m_actualRunTestCallCount );
  }
}
