#include "CoreSuite.h"
#include "MockFunctor.h"
#include "MockProtector.h"
#include "MockTestCase.h"
#include "TestResultTest.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TestResultTest,
                                       coreSuiteName() );


TestResultTest::TestResultTest()
{
}


TestResultTest::~TestResultTest()
{
}


void 
TestResultTest::setUp()
{
  m_result = new CPPUNIT_NS::TestResult();
  m_listener1 = new MockTestListener( "listener1" );
  m_listener2 = new MockTestListener( "listener2" );
  m_dummyTest = new MockTestCase( "dummy-test" );
}


void 
TestResultTest::tearDown()
{
  delete m_dummyTest;
  delete m_listener1;
  delete m_listener2;
  delete m_result;
}


void 
TestResultTest::testConstructor()
{
  CPPUNIT_ASSERT( !m_result->shouldStop() );
}


void 
TestResultTest::testStop()
{
  m_result->stop();
  CPPUNIT_ASSERT( m_result->shouldStop() );
}


void 
TestResultTest::testAddError()
{
  CPPUNIT_NS::Exception *dummyException = new CPPUNIT_NS::Exception( 
                                           CPPUNIT_NS::Message( "some_error" ) );
  m_listener1->setExpectFailure( m_dummyTest, dummyException, true );
  m_result->addListener( m_listener1 );

  m_result->addError( m_dummyTest, dummyException );

  m_listener1->verify();
}


void 
TestResultTest::testAddFailure()
{
  CPPUNIT_NS::Exception *dummyException = new CPPUNIT_NS::Exception( 
                                            CPPUNIT_NS::Message("some_error" ) );
  m_listener1->setExpectFailure( m_dummyTest, dummyException, false );
  m_result->addListener( m_listener1 );

  m_result->addFailure( m_dummyTest, dummyException );

  m_listener1->verify();
}


void 
TestResultTest::testStartTest()
{
  m_listener1->setExpectStartTest( m_dummyTest );
  m_result->addListener( m_listener1 );
  
  m_result->startTest( m_dummyTest );

  m_listener1->verify();
}


void 
TestResultTest::testEndTest()
{
  m_listener1->setExpectEndTest( m_dummyTest );
  m_result->addListener( m_listener1 );
  
  m_result->endTest( m_dummyTest );

  m_listener1->verify();
}


void 
TestResultTest::testStartSuite()
{
  m_listener1->setExpectStartSuite( m_dummyTest );
  m_result->addListener( m_listener1 );
  
  m_result->startSuite( m_dummyTest );

  m_listener1->verify();
}


void 
TestResultTest::testEndSuite()
{
  m_listener1->setExpectEndSuite( m_dummyTest );
  m_result->addListener( m_listener1 );
  
  m_result->endSuite( m_dummyTest );

  m_listener1->verify();
}


void 
TestResultTest::testRunTest()
{
  m_listener1->setExpectStartTestRun( m_dummyTest, m_result );
  m_listener1->setExpectEndTestRun( m_dummyTest, m_result );
  m_result->addListener( m_listener1 );
  
  m_result->runTest( m_dummyTest );

  m_listener1->verify();
}


void 
TestResultTest::testTwoListener()
{
  m_listener1->setExpectStartTest( m_dummyTest );
  m_listener2->setExpectStartTest( m_dummyTest );
  CPPUNIT_NS::Exception *dummyException1 = new CPPUNIT_NS::Exception( 
                                             CPPUNIT_NS::Message( "some_error" ) );
  m_listener1->setExpectFailure( m_dummyTest, dummyException1, true );
  m_listener2->setExpectFailure( m_dummyTest, dummyException1, true );
  m_listener1->setExpectEndTest( m_dummyTest );
  m_listener2->setExpectEndTest( m_dummyTest );
  m_result->addListener( m_listener1 );
  m_result->addListener( m_listener2 );

  m_result->startTest( m_dummyTest );
  m_result->addError( m_dummyTest, dummyException1 );
  m_result->endTest( m_dummyTest );

  m_listener1->verify();
  m_listener2->verify();
}


void 
TestResultTest::testDefaultProtectSucceed()
{
  MockFunctor functor;
  functor.setShouldSucceed();
  m_listener1->setExpectNoFailure();

  m_result->addListener( m_listener1 );
  CPPUNIT_ASSERT( m_result->protect( functor, m_dummyTest ) );
  m_listener1->verify();
  functor.verify();
}


void 
TestResultTest::testDefaultProtectFail()
{
  MockFunctor functor;
  functor.setShouldFail();
  m_listener1->setExpectNoFailure();

  m_result->addListener( m_listener1 );
  CPPUNIT_ASSERT( !m_result->protect( functor, m_dummyTest ) );
  m_listener1->verify();
  functor.verify();
}


void 
TestResultTest::testDefaultProtectFailIfThrow()
{
  MockFunctor functor;
  functor.setThrowFailureException();
  m_listener1->setExpectFailure();

  m_result->addListener( m_listener1 );
  CPPUNIT_ASSERT( !m_result->protect( functor, m_dummyTest ) );
  m_listener1->verify();
  functor.verify();
}


void 
TestResultTest::testProtectChainPushOneTrap()
{
  MockFunctor functor;
  MockProtector *protector = new MockProtector();
  functor.setThrowMockProtectorException();
  protector->setExpectException();
  m_listener1->setExpectFailure();

  m_result->pushProtector( protector );
  m_result->addListener( m_listener1 );
  CPPUNIT_ASSERT( !m_result->protect( functor, m_dummyTest ) );
  protector->verify();
  m_listener1->verify();
  functor.verify();
}


void 
TestResultTest::testProtectChainPushOnePassThrough()
{
  MockFunctor functor;
  MockProtector *protector = new MockProtector();
  functor.setThrowFailureException();
  protector->setExpectNoException();
  m_listener1->setExpectFailure();

  m_result->pushProtector( protector );
  m_result->addListener( m_listener1 );
  CPPUNIT_ASSERT( !m_result->protect( functor, m_dummyTest ) );
  protector->verify();
  m_listener1->verify();
  functor.verify();
}


void 
TestResultTest::testProtectChainPushTwoTrap()
{
  MockFunctor functor;
  functor.setThrowMockProtectorException();
  // protector1 catch the exception retrown by protector2
  MockProtector *protector1 = new MockProtector();
  protector1->setExpectException();
  // protector2 catch the exception and rethrow it
  MockProtector *protector2 = new MockProtector();
  protector2->setExpectCatchAndPropagateException();
  m_listener1->setExpectFailure();

  m_result->pushProtector( protector1 );
  m_result->pushProtector( protector2 );
  m_result->addListener( m_listener1 );
  CPPUNIT_ASSERT( !m_result->protect( functor, m_dummyTest ) );
  protector1->verify();
  protector2->verify();
  m_listener1->verify();
  functor.verify();
}
