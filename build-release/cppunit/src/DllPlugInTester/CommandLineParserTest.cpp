#include "CommandLineParser.h"
#include "CommandLineParserTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CommandLineParserTest );


CommandLineParserTest::CommandLineParserTest()
{
}


CommandLineParserTest::~CommandLineParserTest()
{
}


void 
CommandLineParserTest::setUp()
{
  _parser = NULL;
}


void 
CommandLineParserTest::tearDown()
{
  delete _parser;
}


void 
CommandLineParserTest::parse( const char **lines )
{
  int count =0;
  for ( const char **line = lines; *line != NULL; ++line, ++count );

  delete _parser;
  _parser = new CommandLineParser( count, lines );
  _parser->parse();
}


void 
CommandLineParserTest::testEmptyCommandLine()
{
  static const char *lines[] = { "", NULL };
  parse( lines );

  std::string none;
  CPPUNIT_ASSERT_EQUAL( none, _parser->getEncoding() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getTestPath() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getXmlFileName() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getXmlStyleSheet() );
  CPPUNIT_ASSERT( !_parser->noTestProgress() );
  CPPUNIT_ASSERT( !_parser->useBriefTestProgress() );
  CPPUNIT_ASSERT( !_parser->useCompilerOutputter() );
  CPPUNIT_ASSERT( !_parser->useCoutStream() );
  CPPUNIT_ASSERT( !_parser->useTextOutputter() );
  CPPUNIT_ASSERT( !_parser->useXmlOutputter() );
}


void 
CommandLineParserTest::testFlagCompiler()
{
  static const char *lines[] = { "", "-c", NULL };
  parse( lines );

  std::string none;
  CPPUNIT_ASSERT_EQUAL( none, _parser->getEncoding() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getTestPath() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getXmlFileName() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getXmlStyleSheet() );
  CPPUNIT_ASSERT( !_parser->noTestProgress() );
  CPPUNIT_ASSERT( !_parser->useBriefTestProgress() );
  CPPUNIT_ASSERT( _parser->useCompilerOutputter() );
  CPPUNIT_ASSERT( !_parser->useCoutStream() );
  CPPUNIT_ASSERT( !_parser->useTextOutputter() );
  CPPUNIT_ASSERT( !_parser->useXmlOutputter() );
  CPPUNIT_ASSERT_EQUAL( 0, _parser->getPlugInCount() );
}


void 
CommandLineParserTest::testLongFlagBriefProgress()
{
  static const char *lines[] = { "", "--brief-progress", NULL };
  parse( lines );

  std::string none;
  CPPUNIT_ASSERT_EQUAL( none, _parser->getEncoding() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getTestPath() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getXmlFileName() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getXmlStyleSheet() );
  CPPUNIT_ASSERT( !_parser->noTestProgress() );
  CPPUNIT_ASSERT( _parser->useBriefTestProgress() );
  CPPUNIT_ASSERT( !_parser->useCompilerOutputter() );
  CPPUNIT_ASSERT( !_parser->useCoutStream() );
  CPPUNIT_ASSERT( !_parser->useTextOutputter() );
  CPPUNIT_ASSERT( !_parser->useXmlOutputter() );
  CPPUNIT_ASSERT_EQUAL( 0, _parser->getPlugInCount() );
}


void 
CommandLineParserTest::testFileName()
{
  static const char *lines[] = { "", "TestPlugIn.dll", NULL };
  parse( lines );

  std::string none;
  CPPUNIT_ASSERT_EQUAL( none, _parser->getEncoding() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getTestPath() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getXmlFileName() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getXmlStyleSheet() );
  CPPUNIT_ASSERT( !_parser->noTestProgress() );
  CPPUNIT_ASSERT( !_parser->useBriefTestProgress() );
  CPPUNIT_ASSERT( !_parser->useCompilerOutputter() );
  CPPUNIT_ASSERT( !_parser->useCoutStream() );
  CPPUNIT_ASSERT( !_parser->useTextOutputter() );
  CPPUNIT_ASSERT( !_parser->useXmlOutputter() );

  CPPUNIT_ASSERT_EQUAL( 1, _parser->getPlugInCount() );

  CommandLinePlugInInfo info( _parser->getPlugInAt( 0 ) );
  CPPUNIT_ASSERT_EQUAL( std::string("TestPlugIn.dll"), info.m_fileName );
  CPPUNIT_ASSERT( info.m_parameters.getCommandLine().empty() );
}


void 
CommandLineParserTest::testTestPath()
{
  static const char *lines[] = { "", ":Core", NULL };
  parse( lines );

  std::string none;
  CPPUNIT_ASSERT_EQUAL( none, _parser->getEncoding() );
  CPPUNIT_ASSERT_EQUAL( std::string("Core"), _parser->getTestPath() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getXmlFileName() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getXmlStyleSheet() );
  CPPUNIT_ASSERT( !_parser->noTestProgress() );
  CPPUNIT_ASSERT( !_parser->useBriefTestProgress() );
  CPPUNIT_ASSERT( !_parser->useCompilerOutputter() );
  CPPUNIT_ASSERT( !_parser->useCoutStream() );
  CPPUNIT_ASSERT( !_parser->useTextOutputter() );
  CPPUNIT_ASSERT( !_parser->useXmlOutputter() );
  CPPUNIT_ASSERT_EQUAL( 0, _parser->getPlugInCount() );
}


void 
CommandLineParserTest::testParameterWithSpace()
{
  static const char *lines[] = { "", "--xml", "Test Results.xml", NULL };
  parse( lines );

  std::string none;
  CPPUNIT_ASSERT_EQUAL( none, _parser->getEncoding() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getTestPath() );
  CPPUNIT_ASSERT_EQUAL( std::string("Test Results.xml"), 
                        _parser->getXmlFileName() );
  CPPUNIT_ASSERT_EQUAL( none, _parser->getXmlStyleSheet() );
  CPPUNIT_ASSERT( !_parser->noTestProgress() );
  CPPUNIT_ASSERT( !_parser->useBriefTestProgress() );
  CPPUNIT_ASSERT( !_parser->useCompilerOutputter() );
  CPPUNIT_ASSERT( !_parser->useCoutStream() );
  CPPUNIT_ASSERT( !_parser->useTextOutputter() );
  CPPUNIT_ASSERT( _parser->useXmlOutputter() );
  CPPUNIT_ASSERT_EQUAL( 0, _parser->getPlugInCount() );
}


void 
CommandLineParserTest::testMissingStyleSheetParameterThrow()
{
  static const char *lines[] = { "", "--xsl", NULL };
  parse( lines );
}


void 
CommandLineParserTest::testMissingEncodingParameterThrow()
{
  static const char *lines[] = { "", "--encoding", NULL };
  parse( lines );
}


void 
CommandLineParserTest::testXmlFileNameIsOptional()
{
  static const char *lines[] = { "", "--xml", NULL };
  parse( lines );

  std::string none;
  CPPUNIT_ASSERT_EQUAL( none, _parser->getXmlFileName() );
}


void 
CommandLineParserTest::testPlugInsWithParameters()
{
  static const char *lines[] = { "", "TestPlugIn1.dll=login = lain",
                           "Clocker.dll", NULL };
  parse( lines );

  CPPUNIT_ASSERT_EQUAL( 2, _parser->getPlugInCount() );

  CommandLinePlugInInfo info1( _parser->getPlugInAt( 0 ) );

  CPPUNIT_ASSERT_EQUAL( std::string("TestPlugIn1.dll"), info1.m_fileName );
  CPPUNIT_ASSERT_EQUAL( std::string("login = lain"), 
                        info1.m_parameters.getCommandLine() );

  CommandLinePlugInInfo info2( _parser->getPlugInAt( 1 ) );
  CPPUNIT_ASSERT_EQUAL( std::string("Clocker.dll"), info2.m_fileName );
  CPPUNIT_ASSERT( info2.m_parameters.getCommandLine().empty() );
}
