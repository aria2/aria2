#include "ExtensionSuite.h"
#include "OrthodoxTest.h"
#include <cppunit/extensions/Orthodox.h>
#include <cppunit/TestResult.h>

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( OrthodoxTest,
                                       extensionSuiteName() );

OrthodoxTest::OrthodoxTest()
{
}


OrthodoxTest::~OrthodoxTest()
{
}


void 
OrthodoxTest::setUp()
{
  m_testListener = new MockTestListener( "mock-listener" );
  m_result = new CPPUNIT_NS::TestResult();
  m_result->addListener( m_testListener );
}


void 
OrthodoxTest::tearDown()
{
  delete m_result;
  delete m_testListener;
}


void 
OrthodoxTest::testValue()
{
  CPPUNIT_NS::Orthodox<Value> test;
  m_testListener->setExpectNoFailure();
  test.run( m_result );
  m_testListener->verify();
}


void 
OrthodoxTest::testValueBadConstructor()
{
  CPPUNIT_NS::Orthodox<ValueBadConstructor> test;
  m_testListener->setExpectFailure();
  test.run( m_result );
  m_testListener->verify();
}


void 
OrthodoxTest::testValueBadInvert()
{
  CPPUNIT_NS::Orthodox<ValueBadInvert> test;
  m_testListener->setExpectFailure();
  test.run( m_result );
  m_testListener->verify();
}


void 
OrthodoxTest::testValueBadEqual()
{
  CPPUNIT_NS::Orthodox<ValueBadEqual> test;
  m_testListener->setExpectFailure();
  test.run( m_result );
  m_testListener->verify();
}


void 
OrthodoxTest::testValueBadNotEqual()
{
  CPPUNIT_NS::Orthodox<ValueBadNotEqual> test;
  m_testListener->setExpectFailure();
  test.run( m_result );
  m_testListener->verify();
}


void 
OrthodoxTest::testValueBadCall()
{
  CPPUNIT_NS::Orthodox<ValueBadCall> test;
  m_testListener->setExpectFailure();
  test.run( m_result );
  m_testListener->verify();
}


void 
OrthodoxTest::testValueBadAssignment()
{
  CPPUNIT_NS::Orthodox<ValueBadAssignment> test;
  m_testListener->setExpectFailure();
  test.run( m_result );
  m_testListener->verify();
}
