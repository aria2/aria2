#include <cppunit/config/SourcePrefix.h>
#include "FailureException.h"
#include "HelperMacrosTest.h"
#include "HelperSuite.h"
#include "MockTestCase.h"
#include "SubclassedTestCase.h"
#include <cppunit/TestResult.h>
#include <memory>

/* Note:
 - no unit test for CPPUNIT_TEST_SUITE_REGISTRATION...
 */

class FailTestFixture : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( FailTestFixture );
  CPPUNIT_TEST_FAIL( testFail );
  CPPUNIT_TEST_SUITE_END();
public:
  void testFail()
  {
    CPPUNIT_ASSERT_MESSAGE( "Failure", false );
  }
};


class FailToFailTestFixture : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( FailToFailTestFixture );
  CPPUNIT_TEST_FAIL( testFailToFail );
  CPPUNIT_TEST_SUITE_END();
public:
  void testFailToFail()
  {
  }
};


class ExceptionTestFixture : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( ExceptionTestFixture );
  CPPUNIT_TEST_EXCEPTION( testException, FailureException );
  CPPUNIT_TEST_SUITE_END();
public:
  void testException()
  {
    throw FailureException();
  }
};


class ExceptionNotCaughtTestFixture : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( ExceptionNotCaughtTestFixture );
  CPPUNIT_TEST_EXCEPTION( testExceptionNotCaught, FailureException );
  CPPUNIT_TEST_SUITE_END();
public:
  void testExceptionNotCaught()
  {
  }
};


class CustomsTestTestFixture : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( CustomsTestTestFixture );
  CPPUNIT_TEST_SUITE_ADD_CUSTOM_TESTS( addCustomTests );
  CPPUNIT_TEST_SUITE_END();
public:
  static void addCustomTests( TestSuiteBuilderContextType &context )
  {
    MockTestCase *test1 = new MockTestCase( context.getTestNameFor( "myCustomTest1" ) );
    test1->makeRunTestThrow();
    MockTestCase *test2 = new MockTestCase( context.getTestNameFor( "myCustomTest2" ) );
    context.addTest( test1 );
    context.addTest( test2 );
  }
};


#undef TEST_ADD_N_MOCK
#define TEST_ADD_N_MOCK( totalCount )                                              \
  {                                                                 \
    for ( int count = (totalCount); count > 0; --count )            \
      CPPUNIT_TEST_SUITE_ADD_TEST(                                  \
         new MockTestCase( context.getTestNameFor( "dummyName" ) ) ); \
  }



class AddTestTestFixture : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( AddTestTestFixture );
  TEST_ADD_N_MOCK( 7 );
  CPPUNIT_TEST_SUITE_END();
public:
};



CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( HelperMacrosTest, 
                                       helperSuiteName() );


HelperMacrosTest::HelperMacrosTest()
{
}


HelperMacrosTest::~HelperMacrosTest()
{
}


void 
HelperMacrosTest::setUp()
{
  m_testListener = new MockTestListener( "mock-testlistener" );
  m_result = new CPPUNIT_NS::TestResult();
  m_result->addListener( m_testListener );
}


void 
HelperMacrosTest::tearDown()
{
  delete m_result;
  delete m_testListener;
}


void 
HelperMacrosTest::testNoSubclassing()
{
  std::auto_ptr<CPPUNIT_NS::TestSuite> suite( BaseTestCase::suite() );
  CPPUNIT_ASSERT_EQUAL( 1, suite->countTestCases() );
  m_testListener->setExpectedStartTestCall( 1 );
  m_testListener->setExpectNoFailure();

  suite->run( m_result );
  m_testListener->verify();
}


void 
HelperMacrosTest::testSubclassing()
{
  std::auto_ptr<CPPUNIT_NS::TestSuite> suite( SubclassedTestCase::suite() );
  CPPUNIT_ASSERT_EQUAL( 2, suite->countTestCases() );
  m_testListener->setExpectedStartTestCall( 2 );
  m_testListener->setExpectedAddFailureCall( 1 );

  suite->run( m_result );
  m_testListener->verify();
}


void 
HelperMacrosTest::testFail()
{
  std::auto_ptr<CPPUNIT_NS::TestSuite> suite( FailTestFixture::suite() );
  m_testListener->setExpectedStartTestCall( 1 );
  m_testListener->setExpectNoFailure();

  suite->run( m_result );
  m_testListener->verify();
}


void 
HelperMacrosTest::testFailToFail()
{
  std::auto_ptr<CPPUNIT_NS::TestSuite> suite( FailToFailTestFixture::suite() );
  m_testListener->setExpectedStartTestCall( 1 );
  m_testListener->setExpectedAddFailureCall( 1 );

  suite->run( m_result );
  m_testListener->verify();
}


void 
HelperMacrosTest::testException()
{
  std::auto_ptr<CPPUNIT_NS::TestSuite> suite( ExceptionTestFixture::suite() );
  m_testListener->setExpectedStartTestCall( 1 );
  m_testListener->setExpectNoFailure();
  
  suite->run( m_result );
  m_testListener->verify();
}


void 
HelperMacrosTest::testExceptionNotCaught()
{
  std::auto_ptr<CPPUNIT_NS::TestSuite> suite( ExceptionNotCaughtTestFixture::suite() );
  m_testListener->setExpectedStartTestCall( 1 );
  m_testListener->setExpectedAddFailureCall( 1 );

  suite->run( m_result );
  m_testListener->verify();
}


void 
HelperMacrosTest::testCustomTests()
{
  std::auto_ptr<CPPUNIT_NS::TestSuite> suite( CustomsTestTestFixture::suite() );
  m_testListener->setExpectedStartTestCall( 2 );
  m_testListener->setExpectedAddFailureCall( 1 );

  suite->run( m_result );
  m_testListener->verify();
}


void 
HelperMacrosTest::testAddTest()
{
  std::auto_ptr<CPPUNIT_NS::TestSuite> suite( AddTestTestFixture::suite() );
  m_testListener->setExpectedStartTestCall( 7 );
  m_testListener->setExpectedAddFailureCall( 0 );

  suite->run( m_result );
  m_testListener->verify();
}
