#ifndef CPPUNIT_PLUGIN_PARAMETERS
#define CPPUNIT_PLUGIN_PARAMETERS

#include <cppunit/Portability.h>

#if !defined(CPPUNIT_NO_TESTPLUGIN)

#include <cppunit/portability/CppUnitDeque.h>
#include <string>

CPPUNIT_NS_BEGIN

/*! \brief Test plug-ins parameters.
 */
class CPPUNIT_API PlugInParameters
{
public:
  /// Constructs plug-in parameters from the specified command-line.
  PlugInParameters( const std::string &commandLine = "" );

  virtual ~PlugInParameters();

  /// Returns the command line that was passed on construction.
  std::string getCommandLine() const;

private:
  std::string m_commandLine;
};


CPPUNIT_NS_END

#endif // !defined(CPPUNIT_NO_TESTPLUGIN)


#endif // CPPUNIT_PLUGIN_PARAMETERS
