#include <cppunit/plugin/PlugInParameters.h>

#if !defined(CPPUNIT_NO_TESTPLUGIN)

CPPUNIT_NS_BEGIN


PlugInParameters::PlugInParameters( const std::string &commandLine )
    : m_commandLine( commandLine )
{
}


PlugInParameters::~PlugInParameters()
{
}


std::string 
PlugInParameters::getCommandLine() const
{
  return m_commandLine;
}


CPPUNIT_NS_END

#endif // !defined(CPPUNIT_NO_TESTPLUGIN)
