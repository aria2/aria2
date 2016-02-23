#include <cppunit/TestSuite.h>
#include <cppunit/extensions/TestFixtureFactory.h>
#include <cppunit/extensions/TestNamer.h>
#include <cppunit/extensions/TestSuiteBuilderContext.h>


CPPUNIT_NS_BEGIN

TestSuiteBuilderContextBase::TestSuiteBuilderContextBase( 
                                 TestSuite &suite,
                                 const TestNamer &namer,
                                 TestFixtureFactory &factory )
  : m_suite( suite )
  , m_namer( namer )
  , m_factory( factory )
{
}


TestSuiteBuilderContextBase::~TestSuiteBuilderContextBase()
{
}


void 
TestSuiteBuilderContextBase::addTest( Test *test )
{
  m_suite.addTest( test );
}


std::string 
TestSuiteBuilderContextBase::getFixtureName() const
{
  return m_namer.getFixtureName();
}


std::string 
TestSuiteBuilderContextBase::getTestNameFor( 
                                 const std::string &testMethodName ) const
{
  return m_namer.getTestNameFor( testMethodName );
}


TestFixture *
TestSuiteBuilderContextBase::makeTestFixture() const
{
  return m_factory.makeFixture();
}


void 
TestSuiteBuilderContextBase::addProperty( const std::string &key, 
                                          const std::string &value )
{
  Properties::iterator it = m_properties.begin();
  for ( ; it != m_properties.end(); ++it )
  {
    if ( (*it).first == key )
    {
      (*it).second = value;
      return;
    }
  }

  Property property( key, value );
  m_properties.push_back( property );
}

const std::string 
TestSuiteBuilderContextBase::getStringProperty( const std::string &key ) const
{
  Properties::const_iterator it = m_properties.begin();
  for ( ; it != m_properties.end(); ++it )
  {
    if ( (*it).first == key )
      return (*it).second;
  }
  return "";
}


CPPUNIT_NS_END
