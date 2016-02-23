#include "ExtensionSuite.h"
#include "RepeatedTestTest.h"
#include <cppunit/extensions/RepeatedTest.h>
#include <cppunit/TestResult.h>

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( RepeatedTestTest,
                                       extensionSuiteName() );


RepeatedTestTest::RepeatedTestTest() :
    m_repeatCount( 17 )
{
}


RepeatedTestTest::~RepeatedTestTest()
{
}


void 
RepeatedTestTest::setUp()
{
  m_test = new RunCountTest();
  m_repeatedTest = new CPPUNIT_NS::RepeatedTest( m_test, m_repeatCount );
}


void 
RepeatedTestTest::tearDown()
{
  delete m_repeatedTest;
}


void 
RepeatedTestTest::testRun()
{
  CPPUNIT_NS::TestResult result;
  m_repeatedTest->run( &result );

  CPPUNIT_ASSERT_EQUAL( 17, m_test->m_runCount );
}
