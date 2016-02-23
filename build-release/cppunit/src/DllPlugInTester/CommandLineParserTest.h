#ifndef COMMANDLINEPARSERTEST_H
#define COMMANDLINEPARSERTEST_H

#include <cppunit/extensions/HelperMacros.h>


class CommandLineParser;
class CommandLineParserException;


class CommandLineParserTest : public CPPUNIT_NS::TestCase
{
  CPPUNIT_TEST_SUITE( CommandLineParserTest );
  CPPUNIT_TEST( testEmptyCommandLine );
  CPPUNIT_TEST( testFlagCompiler );
  CPPUNIT_TEST( testLongFlagBriefProgress );
  CPPUNIT_TEST( testFileName );
  CPPUNIT_TEST( testTestPath );
  CPPUNIT_TEST( testParameterWithSpace );
  CPPUNIT_TEST_EXCEPTION( testMissingStyleSheetParameterThrow, CommandLineParserException);
  CPPUNIT_TEST_EXCEPTION( testMissingEncodingParameterThrow, CommandLineParserException);
  CPPUNIT_TEST( testXmlFileNameIsOptional );
  CPPUNIT_TEST( testPlugInsWithParameters );
  CPPUNIT_TEST_SUITE_END();

public:
  CommandLineParserTest();
  virtual ~CommandLineParserTest();

  void setUp();
  void tearDown();

  void testEmptyCommandLine();
  void testFlagCompiler();
  void testLongFlagBriefProgress();
  void testFileName();
  void testTestPath();
  void testParameterWithSpace();
  void testMissingStyleSheetParameterThrow();
  void testMissingEncodingParameterThrow();
  void testXmlFileNameIsOptional();
  void testPlugInsWithParameters();

private:
  CommandLineParserTest( const CommandLineParserTest &other );
  void operator =( const CommandLineParserTest &other );

  void parse( const char **lines );

private:
  CommandLineParser *_parser;
};



// Inlines methods for CommandLineParserTest:
// ------------------------------------------



#endif  // COMMANDLINEPARSERTEST_H
