#ifndef CPPUNIT_COMPILERTESTRESULTOUTPUTTER_H
#define CPPUNIT_COMPILERTESTRESULTOUTPUTTER_H

#include <cppunit/Portability.h>
#include <cppunit/Outputter.h>
#include <cppunit/portability/Stream.h>

CPPUNIT_NS_BEGIN


class Exception;
class SourceLine;
class Test;
class TestFailure;
class TestResultCollector;

/*! 
 * \brief Outputs a TestResultCollector in a compiler compatible format.
 * \ingroup WritingTestResult
 *
 * Printing the test results in a compiler compatible format (assertion
 * location has the same format as compiler error), allow you to use your
 * IDE to jump to the assertion failure. Location format can be customized (see
 * setLocationFormat() ).
 *
 * For example, when running the test in a post-build with VC++, if an assertion
 * fails, you can jump to the assertion by pressing F4 (jump to next error).
 *
 * Heres is an example of usage (from examples/cppunittest/CppUnitTestMain.cpp):
 * \code
 * int main( int argc, char* argv[] ) {
 *   // if command line contains "-selftest" then this is the post build check
 *   // => the output must be in the compiler error format.
 *   bool selfTest = (argc > 1)  &&  
 *                   (std::string("-selftest") == argv[1]);
 *
 *   CppUnit::TextUi::TestRunner runner;
 *   runner.addTest( CppUnitTest::suite() );   // Add the top suite to the test runner
 * 
 *  if ( selfTest )
 *   { // Change the default outputter to a compiler error format outputter
 *     // The test runner owns the new outputter.
 *     runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(),
 *                                                          std::cerr ) );
 *   }
 * 
 *  // Run the test and don't wait a key if post build check.
 *   bool wasSuccessful = runner.run( "", !selfTest );
 * 
 *   // Return error code 1 if the one of test failed.
 *   return wasSuccessful ? 0 : 1;
 * }
 * \endcode
 */
class CPPUNIT_API CompilerOutputter : public Outputter
{
public:
  /*! \brief Constructs a CompilerOutputter object.
   * \param result Result of the test run.
   * \param stream Stream used to output test result.
   * \param locationFormat Error location format used by your compiler. Default
   *                       to \c CPPUNIT_COMPILER_LOCATION_FORMAT which is defined
   *                       in the configuration file. See setLocationFormat() for detail.
   * \see setLocationFormat().
   */
  CompilerOutputter( TestResultCollector *result,
                     OStream &stream,
                     const std::string &locationFormat = CPPUNIT_COMPILER_LOCATION_FORMAT );

  /// Destructor.
  virtual ~CompilerOutputter();

  /*! \brief Sets the error location format.
   * 
   * Indicates the format used to report location of failed assertion. This format should
   * match the one used by your compiler.
   *
   * The location format is a string in which the occurence of the following character
   * sequence are replaced:
   *
   * - "%l" => replaced by the line number
   * - "%p" => replaced by the full path name of the file ("G:\prg\vc\cppunit\MyTest.cpp")
   * - "%f" => replaced by the base name of the file ("MyTest.cpp")
   *
   * Some examples:
   *
   * - VC++ error location format: "%p(%l):" => produce "G:\prg\MyTest.cpp(43):"
   * - GCC error location format: "%f:%l:" => produce "MyTest.cpp:43:"
   * 
   * Thoses are the two compilers currently <em>supported</em> (gcc format is used if
   * VC++ is not detected). If you want your compiler to be automatically supported by
   * CppUnit, send a mail to the mailing list (preferred), or submit a feature request
   * that indicates how to detect your compiler with the preprocessor (\#ifdef...) and
   * your compiler location format.
   */
  void setLocationFormat( const std::string &locationFormat );

  /*! \brief Creates an instance of an outputter that matches your current compiler.
   * \deprecated This class is specialized through parameterization instead of subclassing...
   *             Use CompilerOutputter::CompilerOutputter instead.
   */
  static CompilerOutputter *defaultOutputter( TestResultCollector *result,
                                              OStream &stream );

  void write();

  void setNoWrap();

  void setWrapColumn( int wrapColumn );

  int wrapColumn() const;

  virtual void printSuccess();
  virtual void printFailureReport();
  virtual void printFailuresList();
  virtual void printStatistics();
  virtual void printFailureDetail( TestFailure *failure );
  virtual void printFailureLocation( SourceLine sourceLine );
  virtual void printFailureType( TestFailure *failure );
  virtual void printFailedTestName( TestFailure *failure );
  virtual void printFailureMessage( TestFailure *failure );

private:
  /// Prevents the use of the copy constructor.
  CompilerOutputter( const CompilerOutputter &copy );

  /// Prevents the use of the copy operator.
  void operator =( const CompilerOutputter &copy );

  virtual bool processLocationFormatCommand( char command, 
                                             const SourceLine &sourceLine );

  virtual std::string extractBaseName( const std::string &fileName ) const;

private:
  TestResultCollector *m_result;
  OStream &m_stream;
  std::string m_locationFormat;
  int m_wrapColumn;
};


CPPUNIT_NS_END


#endif  // CPPUNIT_COMPILERTESTRESULTOUTPUTTER_H
