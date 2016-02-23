#include "CommandLineParser.h"


CommandLineParser::CommandLineParser( int argc, 
                                      const char *argv[] )
    : m_useCompiler( false )
    , m_useXml( false )
    , m_briefProgress( false )
    , m_noProgress( false )
    , m_useText( false )
    , m_useCout( false )
    , m_waitBeforeExit( false )
    , m_currentArgument( 0 )
{
  for ( int index =1; index < argc; ++index )
  {
    std::string argument( argv[index ] );
    m_arguments.push_back( argument );
  }
}


CommandLineParser::~CommandLineParser()
{
}


void 
CommandLineParser::parse()
{
  while ( hasNextArgument() )
  {
    getNextOption();
    if ( isOption( "c", "compiler" ) )
      m_useCompiler = true;
    else if ( isOption( "x", "xml" ) )
    {
      m_useXml = true;
      m_xmlFileName = getNextOptionalParameter();
    }
    else if ( isOption( "s", "xsl" ) )
      m_xsl = getNextParameter();
    else if ( isOption( "e", "encoding" ) )
      m_encoding = getNextParameter();
    else if ( isOption( "b", "brief-progress" ) )
      m_briefProgress = true;
    else if ( isOption( "n", "no-progress" ) )
      m_noProgress = true;
    else if ( isOption( "t", "text" ) )
      m_useText = true;
    else if ( isOption( "o", "cout" ) )
      m_useCout = true;
    else if ( isOption( "w", "wait" ) )
      m_waitBeforeExit = true;
    else if ( !m_option.empty() )
      fail( "Unknown option" );
    else if ( hasNextArgument() )
      readNonOptionCommands();
  }
}


void 
CommandLineParser::readNonOptionCommands()
{
  if ( argumentStartsWith( ":" ) )
    m_testPath = getNextArgument().substr( 1 );
  else
  {
    CommandLinePlugInInfo plugIn;
    int indexParameter = getCurrentArgument().find( '=' );
    if ( indexParameter < 0 )
      plugIn.m_fileName = getCurrentArgument();
    else
    {
      plugIn.m_fileName = getCurrentArgument().substr( 0, indexParameter );
      std::string parameters = getCurrentArgument().substr( indexParameter +1 );
      plugIn.m_parameters = CPPUNIT_NS::PlugInParameters( parameters );
    }
    
    m_plugIns.push_back( plugIn );

    getNextArgument();
  }
}


bool 
CommandLineParser::hasNextArgument() const
{
  return m_currentArgument < m_arguments.size();
}


std::string 
CommandLineParser::getNextArgument()
{
  if ( hasNextArgument() )
    return m_arguments[ m_currentArgument++ ];
  return "";
}


std::string 
CommandLineParser::getCurrentArgument() const
{
  if ( m_currentArgument < m_arguments.size() )
    return m_arguments[ m_currentArgument ];
  return "";
}


bool 
CommandLineParser::argumentStartsWith( const std::string &expected ) const
{
  return getCurrentArgument().substr( 0, expected.length() ) == expected;
}


void 
CommandLineParser::getNextOption()
{
  if ( argumentStartsWith( "-" )  ||  argumentStartsWith( "--" ) )
    m_option = getNextArgument();
  else
    m_option = "";
}


bool 
CommandLineParser::isOption( const std::string &shortName,
                             const std::string &longName )
{
  return (m_option == "-" + shortName)  ||
         (m_option == "--" + longName);
}


std::string 
CommandLineParser::getNextParameter()
{
  if ( !hasNextArgument() )
    fail( "missing parameter" );
  return getNextArgument();
}


std::string
CommandLineParser::getNextOptionalParameter()
{
  if ( argumentStartsWith( "-" )  ||  argumentStartsWith( ":" ) )
    return "";
  return getNextArgument();
}


void 
CommandLineParser::fail( std::string message )
{
  throw CommandLineParserException( "while parsing option " + m_option+
            ",\n" + message );
}


bool 
CommandLineParser::useCompilerOutputter() const
{
  return m_useCompiler;
}


bool 
CommandLineParser::useXmlOutputter() const
{
  return m_useXml;
}


std::string 
CommandLineParser::getXmlFileName() const
{
  return m_xmlFileName;
}


std::string 
CommandLineParser::getXmlStyleSheet() const
{
  return m_xsl;
}


std::string 
CommandLineParser::getEncoding() const
{
  return m_encoding;
}


bool 
CommandLineParser::useBriefTestProgress() const
{
  return m_briefProgress;
}


bool 
CommandLineParser::noTestProgress() const
{
  return m_noProgress;
}


bool 
CommandLineParser::useTextOutputter() const
{
  return m_useText;
}


bool 
CommandLineParser::useCoutStream() const
{
  return m_useCout;
}


bool 
CommandLineParser::waitBeforeExit() const
{
  return m_waitBeforeExit;
}


int 
CommandLineParser::getPlugInCount() const
{
  return m_plugIns.size(); 
}

CommandLinePlugInInfo 
CommandLineParser::getPlugInAt( int index ) const
{
  return m_plugIns[ index ];
}


std::string 
CommandLineParser::getTestPath() const
{
  return m_testPath;
}

