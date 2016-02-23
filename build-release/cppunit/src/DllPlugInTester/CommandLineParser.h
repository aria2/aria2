#ifndef CPPUNIT_HELPER_COMMANDLINEPARSER_H
#define CPPUNIT_HELPER_COMMANDLINEPARSER_H

#include <cppunit/Portability.h>
#include <cppunit/portability/CppUnitDeque.h>
#include <cppunit/plugin/PlugInParameters.h>
#include <string>
#include <stdexcept>


/*! Exception thrown on error while parsing command line.
 */
class CommandLineParserException : public std::runtime_error
{
public:
  CommandLineParserException( std::string message )
    : std::runtime_error( message )
  {
  }
};


struct CommandLinePlugInInfo
{
  std::string m_fileName;
  CPPUNIT_NS::PlugInParameters m_parameters;
};


/*! \brief Parses a command line.

-c --compiler
-x --xml [filename]
-s --xsl stylesheet
-e --encoding encoding
-b --brief-progress
-n --no-progress
-t --text
-o --cout
-w --wait
filename[="options"]
:testpath

 */
class CommandLineParser
{
public:
  /*! Constructs a CommandLineParser object.
   */
  CommandLineParser( int argc, 
                     const char *argv[] );

  /// Destructor.
  virtual ~CommandLineParser();

  /*! Parses the command line.
   * \exception CommandLineParserException if an error occurs.
   */
  void parse();

  bool useCompilerOutputter() const;
  bool useXmlOutputter() const;
  std::string getXmlFileName() const;
  std::string getXmlStyleSheet() const;
  std::string getEncoding() const;
  bool useBriefTestProgress() const;
  bool noTestProgress() const;
  bool useTextOutputter() const;
  bool useCoutStream() const;
  bool waitBeforeExit() const;
  std::string getTestPath() const;
  int getPlugInCount() const;
  CommandLinePlugInInfo getPlugInAt( int index ) const;

protected:
  /// Prevents the use of the copy constructor.
  CommandLineParser( const CommandLineParser &copy );

  /// Prevents the use of the copy operator.
  void operator =( const CommandLineParser &copy );

  void readNonOptionCommands();

  bool hasNextArgument() const;

  std::string getNextArgument();

  std::string getCurrentArgument() const;

  bool argumentStartsWith( const std::string &expected ) const;

  void getNextOption();

  bool isOption( const std::string &shortName,
                 const std::string &longName );

  std::string getNextParameter();

  std::string getNextOptionalParameter();

  void fail( std::string message );

protected:
  bool m_useCompiler;
  bool m_useXml;
  std::string m_xmlFileName;
  std::string m_xsl;
  std::string m_encoding;
  bool m_briefProgress;
  bool m_noProgress;
  bool m_useText;
  bool m_useCout;
  bool m_waitBeforeExit;
  std::string m_testPath;

  typedef CppUnitDeque<CommandLinePlugInInfo> PlugIns;
  PlugIns m_plugIns;

  typedef CppUnitDeque<std::string> Arguments;
  Arguments m_arguments;
  unsigned int m_currentArgument;

  std::string m_option;
};


#endif  // CPPUNIT_HELPER_COMMANDLINEPARSER_H
