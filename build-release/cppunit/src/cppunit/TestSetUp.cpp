#include <cppunit/extensions/TestSetUp.h>

CPPUNIT_NS_BEGIN


TestSetUp::TestSetUp( Test *test ) : TestDecorator( test ) 
{
}


void 
TestSetUp::setUp()
{
}


void 
TestSetUp::tearDown() 
{
}


void
TestSetUp::run( TestResult *result )
{ 
  setUp();
  TestDecorator::run(result);
  tearDown();
}


CPPUNIT_NS_END
