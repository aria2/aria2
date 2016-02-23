#include <cppunit/extensions/TestCaseDecorator.h>

CPPUNIT_NS_BEGIN


TestCaseDecorator::TestCaseDecorator( TestCase *test )
    : TestCase( test->getName() ),
      m_test( test )
{ 
}


TestCaseDecorator::~TestCaseDecorator()
{
  delete m_test;
}


std::string 
TestCaseDecorator::getName() const
{ 
  return m_test->getName(); 
}


void 
TestCaseDecorator::setUp()
{
  m_test->setUp();
}


void 
TestCaseDecorator::tearDown()
{
  m_test->tearDown();
}


void 
TestCaseDecorator::runTest()
{
  m_test->runTest();
}


CPPUNIT_NS_END
