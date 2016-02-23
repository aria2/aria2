#include <cppunit/extensions/TestDecorator.h>

CPPUNIT_NS_BEGIN


TestDecorator::TestDecorator( Test *test )
    : m_test( test)
{ 
}


TestDecorator::~TestDecorator()
{
  delete m_test;
}


int 
TestDecorator::countTestCases() const
{ 
  return m_test->countTestCases(); 
}


void 
TestDecorator::run( TestResult *result )
{ 
  m_test->run(result); 
}


std::string 
TestDecorator::getName() const
{ 
  return m_test->getName(); 
}


int 
TestDecorator::getChildTestCount() const
{
  return m_test->getChildTestCount();
}


Test *
TestDecorator::doGetChildTestAt( int index ) const
{
  return m_test->getChildTestAt( index );
}


CPPUNIT_NS_END
