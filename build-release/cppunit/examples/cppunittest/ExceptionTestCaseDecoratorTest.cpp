// //////////////////////////////////////////////////////////////////////////
// Implementation file ExceptionTestCaseDecoratorTest.cpp for class ExceptionTestCaseDecoratorTest
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2002/08/03
// //////////////////////////////////////////////////////////////////////////

#include "ExtensionSuite.h"
#include "ExceptionTestCaseDecoratorTest.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ExceptionTestCaseDecoratorTest,
                                       extensionSuiteName() );


ExceptionTestCaseDecoratorTest::ExceptionTestCaseDecoratorTest()
{
}


ExceptionTestCaseDecoratorTest::~ExceptionTestCaseDecoratorTest()
{
}


void 
ExceptionTestCaseDecoratorTest::setUp()
{
  m_testListener = new MockTestListener( "mock-testlistener" );
  m_result = new CPPUNIT_NS::TestResult();
  m_result->addListener( m_testListener );

  m_test = new MockTestCase( "mock-decorated-testcase" );
  m_decorator = new FailureExceptionTestCase( m_test );
}


void 
ExceptionTestCaseDecoratorTest::tearDown()
{
  delete m_decorator;
  delete m_result;
  delete m_testListener;
}


void 
ExceptionTestCaseDecoratorTest::testNoExceptionThrownFailed()
{
  m_testListener->setExpectedAddFailureCall(1);
  m_test->setExpectedSetUpCall();
  m_test->setExpectedRunTestCall();
  m_test->setExpectedTearDownCall();

  m_decorator->run( m_result );

  m_testListener->verify();
}


void 
ExceptionTestCaseDecoratorTest::testExceptionThrownPass()
{
  m_testListener->setExpectNoFailure();
  m_test->setExpectedSetUpCall();
  m_test->setExpectedRunTestCall();
  m_test->setExpectedTearDownCall();
  m_test->makeRunTestThrow();

  m_decorator->run( m_result );

  m_testListener->verify();
}
