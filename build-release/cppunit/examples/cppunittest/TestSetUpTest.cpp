#include "ExtensionSuite.h"
#include "TestSetUpTest.h"
#include <cppunit/TestResult.h>
#include "MockTestCase.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TestSetUpTest,
                                       extensionSuiteName() );


TestSetUpTest::TestSetUpTest()
{
}


TestSetUpTest::~TestSetUpTest()
{
}


void 
TestSetUpTest::setUp()
{
}


void 
TestSetUpTest::tearDown()
{
}


void 
TestSetUpTest::testRun()
{
  CPPUNIT_NS::TestResult result;
  MockTestCase *test = new MockTestCase( "TestSetUpTest" );
  test->setExpectedSetUpCall();
  test->setExpectedRunTestCall();
  test->setExpectedTearDownCall();
  MockSetUp setUpTest( test );
  
  setUpTest.run( &result );

  setUpTest.verify();
  test->verify();
}
