#include <cppunit/TestComposite.h>
#include <cppunit/TestResult.h>


CPPUNIT_NS_BEGIN


TestComposite::TestComposite( const std::string &name )
    : m_name( name )
{
}


TestComposite::~TestComposite()
{
}


void 
TestComposite::run( TestResult *result )
{
  doStartSuite( result );
  doRunChildTests( result );
  doEndSuite( result );
}


int 
TestComposite::countTestCases() const
{
  int count = 0;
  
  int childCount = getChildTestCount();
  for ( int index =0; index < childCount; ++index )
    count += getChildTestAt( index )->countTestCases();
  
  return count;
}


std::string 
TestComposite::getName() const
{
  return m_name;
}


void 
TestComposite::doStartSuite( TestResult *controller )
{
  controller->startSuite( this );
}


void 
TestComposite::doRunChildTests( TestResult *controller )
{
  int childCount = getChildTestCount();
  for ( int index =0; index < childCount; ++index )
  {
    if ( controller->shouldStop() )
      break;

    getChildTestAt( index )->run( controller );
  }
}


void 
TestComposite::doEndSuite( TestResult *controller )
{
  controller->endSuite( this );
}


CPPUNIT_NS_END

